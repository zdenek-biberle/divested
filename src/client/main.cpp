#include <dlfcn.h>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
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
#include "common/path/build_for_handler.hpp"
#include "common/shm/shm.hpp"
#include "common/util/random_string.hpp"

namespace log {
	void log(const std::stringstream &msg) {
		std::stringstream ss;
		ss << "CLIENT " << gettid() << ": " << msg.str() << std::endl;
		std::cerr << ss.str();
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

using array_buffer = std::array<char, 2048>;

struct shared_t {
	AEffect effect;
	audioMasterCallback cb;
	client_state_t state = client_state_t::creating;
	std::vector<std::unique_ptr<char[]>> allocated_chunks; // TODO: this should be either not shared or protected by a mutex

	shared_t(audioMasterCallback cb):
		cb{cb}
	{}
};

struct client_t : public handler::with_shm {
	int send_fd;
	int recv_fd;
	shared_t &shared;
	ERect rect;

	struct message_configuration {
		using dispatcher_received = msg::host_dispatcher;
		using dispatcher_sent = msg::effect_dispatcher;
		static constexpr bool is_plugin = false;
		static constexpr std::array<msg::type_t, 1> supported_messages = {msg::type_t::dispatcher};
	};

	client_t(int send_fd, int recv_fd, shm::shm_t &&shm, shared_t &shared):
		handler::with_shm{std::move(shm)},
		send_fd{send_fd},
		recv_fd{recv_fd},
		shared{shared}
	{
		LOG_TRACE("Instantiated client with send_fd " << send_fd << ", recv_fd " << recv_fd << " & shm " << _shm.name());
	}

	client_t(client_t &&other):
		handler::with_shm{std::move(other)},
		send_fd(other.send_fd),
		recv_fd(other.recv_fd),
		shared(other.shared),
		rect(other.rect)
	{
		other.send_fd = -1;
		other.recv_fd = -1;
	}

	static void close_pipe(int fd) {
		if (fd != -1)
			::close(fd);
	}

	~client_t() {
		close_pipe(send_fd);
		close_pipe(recv_fd);
	}

	VstIntPtr dispatcher(VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {

		auto effect_ptr = shared.state == client_state_t::created ? &shared.effect : nullptr;
		LOG_TRACE("Dispatching to master, effect " << reinterpret_cast<void*>(effect_ptr)
			<< ", opcode " << opcode
			<< ", index " << index
			<< ", value " << value
			<< ", ptr " << ptr
			<< ", opt " << opt);
		auto result = shared.cb(effect_ptr, opcode, index, value, ptr, opt);
		LOG_TRACE("Got result " << result);
		return result;
	}

	template<size_t BufLen>
	ssize_t message_read(std::array<char, BufLen> &buf) {
		LOG_TRACE("Reading " << buf.size() << " B...");
		ssize_t readlen = ::read(recv_fd, buf.data(), buf.size());
		LOG_TRACE("Read " << readlen << " B");

		if (readlen == 0)
			throw std::runtime_error("Unexpected end of pipe");
		
		return readlen;
	}

	ssize_t message_write(const char *buf, size_t len) {
		LOG_TRACE("Writing " << len << " B...");
		ssize_t writelen = ::write(send_fd, buf, len);
		LOG_TRACE("Wrote " << len << " B");

		if (static_cast<ssize_t>(len) != writelen)
			throw std::runtime_error("Didn't write expected number of bytes");

		return writelen;
	}

	char *allocate_chunk(size_t size) {
		auto chunk = std::make_unique<char[]>(size);
		auto ptr = chunk.get();
		shared.allocated_chunks.push_back(std::move(chunk));
		return ptr;
	}

	ERect &get_rect() {
		return rect;
	}

	void instantiate_handler(int index) {
		array_buffer buf;
		msg::send_instantiate_handler(*this, buf, {index}); 
	}
};

struct paths_t {
	std::string send;
	std::string recv;
	std::string shm;

	paths_t(const std::string &send, const std::string &recv, const std::string &shm):
		send(send),
		recv(recv),
		shm(shm)
	{}

	paths_t(const paths_t &base, int index):
		send{path::build_for_handler(base.send, index)},
		recv{path::build_for_handler(base.recv, index)},
		shm{path::build_for_handler(base.shm, index)}
	{}
};

struct created_resources_t {
	paths_t paths;
	shm::shm_t shm;
};

struct plugin_t {
	paths_t base_paths;
	shared_t shared;
	client_t primary_client;
	std::mutex primary_client_mutex;
	std::map<pid_t, std::unique_ptr<client_t>> tid_to_client; // This should somehow be protected from concurrent writes

	static created_resources_t create_resources(const paths_t &base_paths, int index) {
		paths_t paths{base_paths, index};

		LOG_TRACE("mkfifo send: " << ::mkfifo(paths.send.c_str(), S_IRUSR | S_IWUSR));
		LOG_TRACE("mkfifo recv: " << ::mkfifo(paths.recv.c_str(), S_IRUSR | S_IWUSR));

		shm::shm_t shm{shm::shm_t::create(paths.shm)};

		return {paths, std::move(shm)};
	}

	static client_t create_client(shared_t &shared, created_resources_t &&res) {
		// open the pipes
		LOG_TRACE("Opening recv pipe " << res.paths.recv);
		int recv_fd = ::open(res.paths.recv.c_str(), O_RDONLY);
		LOG_TRACE("Opening send pipe " << res.paths.send);
		int send_fd = ::open(res.paths.send.c_str(), O_WRONLY);

		// opening the pipes should block until the server has opened them as
		// well, so now we can delete them so that they do not stick around
		// when/if this process dies
		::unlink(res.paths.send.c_str());
		::unlink(res.paths.recv.c_str());

		// now we instantiate the client
		return client_t{send_fd, recv_fd, std::move(res.shm), shared};
	}

	static client_t create_primary_client(const paths_t &base_paths, const std::string &server_path, const std::string &dll_path, shared_t &shared) {
		// First, create the shared resources
		auto res = create_resources(base_paths, 0);

		// Next, spawn the server so that it can open the shared resources
		const char *const server_argv[] = {
			server_path.c_str(),
			dll_path.c_str(),
			base_paths.send.c_str(),
			base_paths.recv.c_str(),
			base_paths.shm.c_str(),
			nullptr
		};
		pid_t server_pid;
		posix_spawn_file_actions_t actions;
		posix_spawn_file_actions_init(&actions);
		posix_spawnattr_t attrs;
		posix_spawnattr_init(&attrs);
		LOG_TRACE(server_path << " " << dll_path << " " << res.paths.send << " " << res.paths.recv << " " << res.paths.shm);
		LOG_TRACE(const_cast<char *const*>(server_argv));
		int spawn_result = ::posix_spawn(
				&server_pid,
				server_path.c_str(),
				&actions,
				&attrs,
				const_cast<char *const*>(server_argv), // according to man exec, the cast should be fine
				environ
			);

		LOG_TRACE("posix_spawn: " << spawn_result);

		if (spawn_result != 0) {
			std::stringstream ss;
			ss << "posix_spawn failed: " << ::strerror(errno);
			throw std::runtime_error(ss.str());
		}
	
		// Next we instantiate our client
		return create_client(shared, std::move(res));
	}

	plugin_t(const paths_t &paths, const std::string &server_path, const std::string &dll_path, audioMasterCallback cb):
		base_paths{paths},
		shared{cb},
		primary_client{create_primary_client(base_paths, server_path, dll_path, shared)}
	{}

	client_t &get_client() {
		auto tid = gettid();
		std::lock_guard lock(primary_client_mutex); // TODO: don't lock the mutex when accessing a client that already exists

		if (auto it = tid_to_client.find(tid) ; it != tid_to_client.end()) {
			return *(it->second.get());
		} else {
			LOG_TRACE("Creating new server handler");
			auto index = tid_to_client.size() + 1;
			auto res{create_resources(base_paths, index)};
			primary_client.instantiate_handler(index);
			auto client = create_client(shared, std::move(res));
			auto client_ptr = std::make_unique<client_t>(std::move(client));
			auto [client_it, inserted] = tid_to_client.insert({tid, std::move(client_ptr)});

			if (!inserted) {
				throw std::runtime_error("Client was already in tid_to_client map");
			}

			return *(client_it->second.get());
		}
	}

	void wait_for_effect() {
		LOG_TRACE("Waiting for AEffect (sizeof " << sizeof(AEffect) << ")");

		array_buffer buf;
		size_t offset = msg::receive_message(primary_client, buf);
		msg::io::read_data(buf.data(), offset, shared.effect);

		LOG_TRACE("Received AEffect, patching...");

		log_proc_addresses(&shared.effect);
		shared.effect.dispatcher = &aeffect_dispatcher_proc;
		shared.effect.process = &aeffect_process_proc;
		shared.effect.setParameter = &aeffect_set_parameter_proc;
		shared.effect.getParameter = &aeffect_get_parameter_proc;
		shared.effect.processReplacing = &aeffect_process_proc;
		shared.effect.processDoubleReplacing = &aeffect_process_double_proc;
		shared.effect.user = reinterpret_cast<void*>(this);
		shared.effect.object = nullptr; // TODO: try setting this to a not null value when everything else works to see if the DAW breaks
		shared.effect.flags &= ~effFlagsHasEditor;
		log_proc_addresses(&shared.effect);

		shared.state = client_state_t::created;
	}

	void log_proc_addresses(AEffect *eff) {
		LOG_TRACE("Procs: "
			<< "    object: " << eff->object << std::endl
			<< "    user: " << eff->user << std::endl
			<< "    dispatcher: " << reinterpret_cast<void*>(eff->dispatcher) << std::endl
			<< "    process: " << reinterpret_cast<void*>(eff->process) << std::endl
			<< "    setParameter: " << reinterpret_cast<void*>(eff->setParameter) << std::endl
			<< "    getParameter: " << reinterpret_cast<void*>(eff->getParameter) << std::endl
			<< "    processReplacing: " << reinterpret_cast<void*>(eff->processReplacing) << std::endl
			<< "    processDoubleReplacing: " << reinterpret_cast<void*>(eff->processDoubleReplacing));
	}
};

VstIntPtr VSTCALLBACK aeffect_dispatcher_proc(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {

	msg::dispatcher_request request{index, value, ptr, opt};
	array_buffer buf;
	LOG_TRACE("aeffect_dispatcher_proc called, effect=" << reinterpret_cast<void*>(effect)
		<< " opcode=" << opcode
		<< " request=" << request);

	// ignore vendor specific calls (for now)
	if (opcode == effVendorSpecific) {
		LOG_TRACE("Request is vendor specific, ignoring");
		return 0;
	}

	// get a ckuebt 
	auto &plugin = *reinterpret_cast<plugin_t*>(effect->user);
	auto &client = plugin.get_client();
	auto result = msg::send_dispatcher(client, buf, opcode, request);
	LOG_TRACE("aeffect_dispatcher_proc finished with result " << result);

	// If we got an effMainsChanged message, we can clean up allocated chunks.
	if (opcode == effMainsChanged) {
		LOG_TRACE("Cleaning up " << plugin.shared.allocated_chunks.size() << " allocated chunks");
		plugin.shared.allocated_chunks.clear();
	}

	// If we got an effClose message, we should first close all server handlers
	// by sending them a return and then close the primary handler by sending it
	// a return as well.
	if (opcode == effClose) {
		LOG_TRACE("effClose received, shutting down " << plugin.tid_to_client.size() << " client");

		std::lock_guard lock{plugin.primary_client_mutex};

		for (auto &i : plugin.tid_to_client) {
			LOG_TRACE("Stopping server handler for TID " << i.first);

			msg::send_return(*i.second, buf);
			i.second.reset();
		}

		LOG_TRACE("Stopping server handler for primary client");
		msg::send_return(plugin.primary_client, buf);

		LOG_TRACE("Deleting plugin");
		delete &plugin;
		LOG_TRACE("Cleaned up");
	}

	return result;
}

void VSTCALLBACK aeffect_process_proc(AEffect* effect, float** inputs, float** outputs, VstInt32 sampleFrames) {
	//throw std::runtime_error("CLIENT: Don't yet know how to process.");
}

void VSTCALLBACK aeffect_process_double_proc(AEffect* effect, double** inputs, double** outputs, VstInt32 sampleFrames) {
	//throw std::runtime_error("CLIENT: Don't yet know how to process doubles.");
}

void VSTCALLBACK aeffect_set_parameter_proc(AEffect* effect, VstInt32 index, float parameter) {
	auto &client = reinterpret_cast<plugin_t*>(effect->user)->get_client();
	msg::set_parameter_request request{index, parameter};
	array_buffer buf;
	LOG_TRACE("aeffect_set_parameter_proc called, effect: " << reinterpret_cast<void *>(effect)
		<< "request: " << request);
	msg::send_set_parameter(client, buf, request);
}

float VSTCALLBACK aeffect_get_parameter_proc(AEffect* effect, VstInt32 index) {
	auto &client = reinterpret_cast<plugin_t*>(effect->user)->get_client();
	msg::get_parameter_request request{index};
	array_buffer buf;
	LOG_TRACE("aeffect_get_parameter_proc called, effect: " << reinterpret_cast<void *>(effect)
		<< "request: " << request);
	return msg::send_get_parameter(client, buf, request);
}

extern "C" AEffect VSTCALLBACK *VSTPluginMain(audioMasterCallback audio_master) {
	LOG_TRACE("Starting Divested client");
	auto shared_name = "divested-" + util::random_string(32);
	LOG_TRACE("Using shared name " << shared_name);

	std::string temp_path = (fs::temp_directory_path() / shared_name).native();
	std::string shm_path = "/" + shared_name;
	std::string c2s_path = temp_path + "-c2s";
	std::string s2c_path = temp_path + "-s2c";

	// get path to the library itself
	Dl_info dl_info;
	LOG_TRACE("dladdr: " << ::dladdr(reinterpret_cast<void*>(&VSTPluginMain), &dl_info));
	auto dll_path = fs::path{dl_info.dli_fname}.replace_extension("dll");
	LOG_TRACE("dll path: " << dll_path);
	auto server_path = fs::path{dll_path}.replace_filename("divested-server.exe");

	// now we instantiate the plugin
	plugin_t *plugin = new plugin_t{paths_t{c2s_path, s2c_path, shm_path}, server_path, dll_path, audio_master};
	plugin->wait_for_effect();
	auto effect = &(plugin->shared.effect);
	LOG_TRACE("main done with effect " << reinterpret_cast<void*>(effect) << " returned to host");

	return effect;
}
