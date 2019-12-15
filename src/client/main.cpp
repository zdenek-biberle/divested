#include <cstdio>
#include <dlfcn.h>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <memory>
#include <spawn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <type_traits>
#include <unistd.h>
#include <vector>
#include "pluginterfaces/vst2.x/aeffect.h"
#include "common/handler/shm.hpp"
#include "common/msg/io/base.hpp"
#include "common/msg/master/messages.hpp"
#include "common/msg/dispatchers.hpp"
#include "common/msg/receiver.hpp"
#include "common/msg/type.hpp"
#include "common/shm/shm.hpp"

namespace log {
	std::ostream &log() {
		return std::cerr << "CLIENT: ";
	}
}

namespace fs = std::filesystem;

VstIntPtr VSTCALLBACK aeffect_dispatcher_proc(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
void VSTCALLBACK aeffect_process_proc(AEffect* effect, float** inputs, float** outputs, VstInt32 sampleFrames);
void VSTCALLBACK aeffect_process_double_proc(AEffect* effect, double** inputs, double** outputs, VstInt32 sampleFrames);
void VSTCALLBACK aeffect_set_parameter_proc(AEffect* effect, VstInt32 index, float parameter);
float VSTCALLBACK aeffect_get_parameter_proc(AEffect* effect, VstInt32 index);

enum class client_state_t {
	creating,
	created
};

struct client_t : public handler::with_shm {
	int send_fd;
	int recv_fd;
	AEffect effect;
	audioMasterCallback cb;
	client_state_t state;
	std::vector<std::unique_ptr<char[]>> allocated_chunks;

	struct message_configuration {
		using dispatcher_received = msg::host_dispatcher;
		using dispatcher_sent = msg::effect_dispatcher;
	};

	client_t(int send_fd, int recv_fd, audioMasterCallback cb, shm::shm_t &&shm):
		handler::with_shm(std::move(shm)),
		send_fd(send_fd),
		recv_fd(recv_fd),
		cb(cb),
		state(client_state_t::creating)
	{
		log::log() << "Instantiated client with send_fd " << send_fd << ", recv_fd " << recv_fd << " & shm " << _shm.name() << std::endl;
	}

	~client_t() {
		::close(send_fd);
		::close(recv_fd);
	}

	VstIntPtr dispatcher(VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {

		auto effect_ptr = state == client_state_t::created ? &effect : nullptr;
		log::log() << "Dispatching to master, effect " << reinterpret_cast<void*>(effect_ptr)
			<< ", opcode " << opcode
			<< ", index " << index
			<< ", value " << value
			<< ", ptr " << ptr
			<< ", opt " << opt
			<< std::endl;
		auto result = cb(effect_ptr, opcode, index, value, ptr, opt);
		log::log() << "Got result " << result << std::endl;
		return result;
	}

	template<size_t BufLen>
	ssize_t message_read(std::array<char, BufLen> &buf) {
		log::log() << "Reading " << buf.size() << " B...";
		ssize_t readlen = ::read(recv_fd, buf.data(), buf.size());
		log::log() << "Read " << readlen << " B";
		
		return readlen;
	}

	ssize_t message_write(const char *buf, size_t len) {
		log::log() << "Writing " << len << " B...";
		ssize_t writelen = ::write(send_fd, buf, len);
		log::log() << "Wrote " << len << " B";

		if (static_cast<ssize_t>(len) != writelen)
			throw std::runtime_error("Didn't write expected number of bytes");

		return writelen;
	}

	void wait_for_effect() {
		log::log() << "Waiting for AEffect (sizeof " << sizeof(AEffect) << ")" << std::endl;

		std::array<char, 2048> buf;
		size_t offset = msg::receive_message(*this, buf);
		msg::io::read_data(buf.data(), offset, effect);

		log::log() << "Received AEffect, patching..." << std::endl;

		log_proc_addresses(&effect);
		effect.dispatcher = &aeffect_dispatcher_proc;
		effect.process = &aeffect_process_proc;
		effect.setParameter = &aeffect_set_parameter_proc;
		effect.getParameter = &aeffect_get_parameter_proc;
		effect.processReplacing = &aeffect_process_proc;
		effect.processDoubleReplacing = &aeffect_process_double_proc;
		effect.user = reinterpret_cast<void*>(this);
		effect.object = nullptr; // TODO: try setting this to a not null value when everything else works to see if the DAW breaks
		log_proc_addresses(&effect);

		state = client_state_t::created;
	}

	void log_proc_addresses(AEffect *eff) {
		log::log() << "Procs: "
			<< "    object: " << eff->object << std::endl
			<< "    user: " << eff->user << std::endl
			<< "    dispatcher: " << reinterpret_cast<void*>(eff->dispatcher) << std::endl
			<< "    process: " << reinterpret_cast<void*>(eff->process) << std::endl
			<< "    setParameter: " << reinterpret_cast<void*>(eff->setParameter) << std::endl
			<< "    getParameter: " << reinterpret_cast<void*>(eff->getParameter) << std::endl
			<< "    processReplacing: " << reinterpret_cast<void*>(eff->processReplacing) << std::endl
			<< "    processDoubleReplacing: " << reinterpret_cast<void*>(eff->processDoubleReplacing) << std::endl
		;
		
	}

	char *allocate_chunk(size_t size) {
		auto chunk = std::make_unique<char[]>(size);
		auto ptr = chunk.get();
		allocated_chunks.push_back(std::move(chunk));
		return ptr;
	}
};

VstIntPtr VSTCALLBACK aeffect_dispatcher_proc(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {
	auto &client = *reinterpret_cast<client_t*>(effect->user);
	msg::dispatcher_request request{index, value, ptr, opt};
	std::array<char, 2048> buf;
	log::log() << "aeffect_dispatcher_proc called, effect " << reinterpret_cast<void*>(effect)
		<< ", opcode: " << opcode
		<< ", request: " << request
		<< std::endl;
	auto result = msg::send_dispatcher(client, buf, opcode, request);
	log::log() << "aeffect_dispatcher_proc finished with result " << result << std::endl;

	// If we got an effMainsChanged message, we can clean up allocated chunks.
	if (opcode == effMainsChanged) {
		log::log() << "Cleaning up " << client.allocated_chunks.size() << " allocated chunks" << std::endl;
		client.allocated_chunks.clear();
	}

	// If we got an effClose message, we should send a return to the server so that
	// it finishes. We should also cleanup after ourselves.
	if (opcode == effClose) {
		log::log() << "Stopping the server and cleaning up" << std::endl;
		msg::send_return(client, buf);
		delete &client;
		log::log() << "Cleaned up" << std::endl;
	}

	return result;
}

void VSTCALLBACK aeffect_process_proc(AEffect* effect, float** inputs, float** outputs, VstInt32 sampleFrames) {
	throw std::runtime_error("CLIENT: Don't yet know how to process.");
}

void VSTCALLBACK aeffect_process_double_proc(AEffect* effect, double** inputs, double** outputs, VstInt32 sampleFrames) {
	throw std::runtime_error("CLIENT: Don't yet know how to process doubles.");
}

void VSTCALLBACK aeffect_set_parameter_proc(AEffect* effect, VstInt32 index, float parameter) {
	throw std::runtime_error("CLIENT: Don't yet know how to set parameters.");
}

float VSTCALLBACK aeffect_get_parameter_proc(AEffect* effect, VstInt32 index) {
	throw std::runtime_error("CLIENT: Don't yet know how to get parameters.");
}

extern "C" AEffect VSTCALLBACK *VSTPluginMain(audioMasterCallback audio_master) {
	std::cout.rdbuf()->pubsetbuf(0, 0);
	std::cerr.rdbuf()->pubsetbuf(0, 0);

	log::log() << "Starting Divested client" << std::endl;
	std::string mkdtemp_template = (fs::temp_directory_path() / "divested.XXXXXX").native();
	log::log() << "mkdtemp: " << ::mkdtemp(mkdtemp_template.data()) << std::endl;
	fs::path temp_path{mkdtemp_template};
	fs::path c2s_path = temp_path / "c2s";
	fs::path s2c_path = temp_path / "s2c";

	// get path to the library itself
	Dl_info dl_info;
	log::log() << "dladdr: " << ::dladdr(reinterpret_cast<void*>(&VSTPluginMain), &dl_info) << std::endl;
	auto dll_path = fs::path{dl_info.dli_fname}.replace_extension("dll");
	log::log() << "dll path: " << dll_path << std::endl;

	// make the pipes
	log::log() << "mkfifo c2s: " << ::mkfifo(c2s_path.c_str(), S_IRUSR | S_IWUSR) << std::endl;
	log::log() << "mkfifo s2c: " << ::mkfifo(s2c_path.c_str(), S_IRUSR | S_IWUSR) << std::endl;

	// make the shared memory
	shm::shm_t shm{shm::shm_t::create()};

	// spawn the server
	auto server_path = fs::path{dll_path}.replace_filename("divested-server.exe");
	const char *const server_argv[] = {
		server_path.c_str(),
		dll_path.c_str(),
		c2s_path.c_str(),
		s2c_path.c_str(),
		shm.name().c_str(),
		nullptr
	};
	pid_t server_pid;
	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);
	posix_spawnattr_t attrs;
	posix_spawnattr_init(&attrs);
	std::cout << server_path << " " << dll_path << " " << c2s_path << " " << s2c_path << std::endl;
	std::cout << const_cast<char *const*>(server_argv);
	int res = ::posix_spawn(
			&server_pid,
			server_path.c_str(),
			&actions,
			&attrs,
			const_cast<char *const*>(server_argv), // according to man exec, the cast should be fine
			environ
		);

	log::log() << "posix_spawn: " << res << std::endl;
	
	// open the pipes
	int send_fd = ::open(c2s_path.c_str(), O_WRONLY);
	int recv_fd = ::open(s2c_path.c_str(), O_RDONLY);

	// opening the pipes should block until the server has opened them as
	// well, so now we can delete them so that they do not stick around
	// when/if this process dies
	::unlink(c2s_path.c_str());
	::unlink(s2c_path.c_str());
	::rmdir(temp_path.c_str());

	// now we instantiate the client
	client_t *client = new client_t{send_fd, recv_fd, audio_master, std::move(shm)};

	client->wait_for_effect();

	auto effect = &(client->effect);

	log::log() << "main done with effect " << reinterpret_cast<void*>(effect) << " returned to host" << std::endl;

	return effect;
}

