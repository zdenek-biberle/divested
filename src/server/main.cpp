#include <cassert>
#include <codecvt>
#include <cstring>
#include <locale.h>
#include <locale>
#include <map>
#include <mutex>
#include <iostream>
#include <cstdio>
#include <sstream>
#include <thread>
#include <vector>
#include <windows.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common/handler/shm.hpp"
#include "common/msg/master/messages.hpp"
#include "common/msg/dispatchers.hpp"
#include "common/msg/receiver.hpp"
#include "common/msg/type.hpp"
#include "common/path/build_for_handler.hpp"
#include "common/shm/shm.hpp"
#include "pluginterfaces/vst2.x/aeffect.h"

namespace log {
	void log(const std::stringstream &msg) {
		std::stringstream ss;
		ss << "SERVER " << GetCurrentThreadId() << ": " << msg.str() << std::endl;
		std::cerr << ss.str();
	}
}

typedef AEffect *(VSTCALLBACK *vst_plugin_main_t)(audioMasterCallback);

shm::shm_t open_shm(const std::string &base, int index) {
	auto path = path::build_for_handler(base, index);
	LOG_TRACE("Opening shm " << path);

	// open the shared memory
	shm::shm_t shm{shm::shm_t::open(path)};

	// now unlink the shared memory, because we do not need the file anymore
	shm.unlink();
	
	// and return it
	return shm;
}

int open_pipe(const std::string &base, int index, int omode) {
	// Get the pipe's path
	auto path = path::build_for_handler(base, index);

	LOG_TRACE("Opening pipe " << path);

	// Try to open the pipe
	int fd = ::open(path.c_str(), omode);

	// Throw if it failed
	if (fd == -1) {
		std::stringstream ss;
		ss << "Opening pipe " << path << " failed: " << ::strerror(errno);
		throw std::runtime_error(ss.str());
	}
	
	// and return it otherwise
	return fd;
}

struct server_t;

struct shared_t {
	std::mutex tid_to_server_mutex;
	std::map<DWORD, server_t*> tid_to_server; // TODO: make the map writes atomic

	server_t &get_server() {
		auto tid = GetCurrentThreadId();
		auto server = tid_to_server[tid];
		LOG_TRACE("Server for TID " << tid << " requested, got " << reinterpret_cast<void *>(server)); 
		return *server;
	}

	void add_server(server_t *server) {
		std::lock_guard lock{tid_to_server_mutex};
		auto tid = GetCurrentThreadId();
		LOG_TRACE("Adding server " << reinterpret_cast<void *>(server) << " for TID " << tid);
		tid_to_server.insert({tid, server});
	}

	void remove_server() {
		std::lock_guard lock{tid_to_server_mutex};
		tid_to_server.erase(GetCurrentThreadId());
	}
};

struct server_t : public handler::with_shm {
	shared_t &shared;
	int send_fd;
	int recv_fd;
	AEffect* effect;
	const std::string &send_base_path;
	const std::string &recv_base_path;
	const std::string &shm_base_path;
	int index;

	struct message_configuration {
		using dispatcher_received = msg::effect_dispatcher;
		using dispatcher_sent = msg::host_dispatcher;
		static constexpr bool is_plugin = true;
	};

	server_t(shared_t &shared, const std::string &send_base_path, const std::string &recv_base_path, const std::string &shm_base_path, int index):
		handler::with_shm{open_shm(shm_base_path, index)},
		shared{shared},
		send_fd{open_pipe(send_base_path, index, O_WRONLY)},
		recv_fd{open_pipe(recv_base_path, index, O_RDONLY)},
		effect{nullptr},
		send_base_path{send_base_path},
		recv_base_path{recv_base_path},
		shm_base_path{shm_base_path},
		index{index}
	{
		LOG_TRACE("Instantiated server with index=" << index << " send_base_path=" << send_base_path << " recv_base_path=" << recv_base_path << " shm_base_path=" << shm_base_path);
	}

	~server_t() {
		::close(send_fd);
		::close(recv_fd);
	}

	struct start_thread_t {
		const server_t *base_server;
		int index;
	};

	static DWORD WINAPI start_thread(void *ptr) {
		auto data = static_cast<start_thread_t *>(ptr);
		auto [base_server, index] = *data;
		delete data;

		LOG_TRACE("Starting new server with index " << index);
		try {
			server_t server{base_server->shared, base_server->send_base_path, base_server->recv_base_path, base_server->shm_base_path, index};
			server.effect = base_server->effect;
			base_server->shared.add_server(&server);
			server.run();
			base_server->shared.remove_server();
		} catch (std::exception &ex) {
			LOG_TRACE("Server with index " << index << " failed: " << ex.what());
			return 1;
		}

		return 0;
	}

	VstIntPtr dispatcher(VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {
		return effect->dispatcher(effect, opcode, index, value, ptr, opt);
	}

	float get_parameter(VstInt32 index) {
		return effect->getParameter(effect, index);
	}

	void set_parameter(VstInt32 index, float opt) {
		effect->setParameter(effect, index, opt);
	}

	void instantiate_handler(int index) {
		LOG_TRACE("Starting thread for server with index " << index);
		auto data = new start_thread_t{this, index};
		CreateThread(nullptr, 0, &start_thread, data, 0, nullptr);
		LOG_TRACE("Thread started");
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

	/// Return the AEffect to the client
	void return_effect() {
		LOG_TRACE("Server " << index << " returning AEffect to client");
		std::array<char, 2048> buf;
		msg::send_return_payload(*this, buf, *effect);
	}

	/// Keep processing messages until a return is received
	void run() {
		LOG_TRACE("Server " << index << " will now start handling messages");
		std::array<char, 2048> buf;
		msg::receive_message(*this, buf);
		LOG_TRACE("Server " << index << " finished");
	}

};

struct globals_t {
	shared_t *shared;
};

globals_t globals;

VstIntPtr VSTCALLBACK audio_master(AEffect *effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt) {
	auto &server = globals.shared->get_server();
	msg::dispatcher_request request{index, value, ptr, opt};
	LOG_TRACE("master: effect: " << reinterpret_cast<void *>(effect)
		<< "opcode: " << opcode
		<< "request: " << request);
	std::array<char, 2048> buf;
	return msg::send_dispatcher(server, buf, opcode, request);
}

std::string wstr_conv(const WCHAR *str) {
	std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> cvt;
	return cvt.to_bytes(reinterpret_cast<const char16_t *>(str));
}

int actual_win_main() {
	LPWSTR cmd_line = GetCommandLineW();
	int argc;
	LPWSTR *argv = CommandLineToArgvW(cmd_line, &argc);
	for (int i = 0; i < argc; i++)
		std::cout << i << ": " << wstr_conv(argv[i]) << std::endl;

	std::cout << "sizeof wchar_t: " << sizeof(wchar_t) << std::endl;
	std::cout << "argc: " << argc << std::endl;
	std::cout << "cmd line: " << wstr_conv(cmd_line) << std::endl;

	// open the pipes
	auto recv_path = wstr_conv(argv[2]);
	auto send_path = wstr_conv(argv[3]);
	auto shm_path = wstr_conv(argv[4]);
	
	// load the library and find the VSTPluginMain procedure
	HMODULE lib = LoadLibraryW(argv[1]);
	std::cout << "SERVER: lib: " << lib << std::endl;
	vst_plugin_main_t vst_plugin_main = (vst_plugin_main_t) GetProcAddress(lib, "VSTPluginMain");
	std::cout << "SERVER: vst plugin main: " << reinterpret_cast<void *>(vst_plugin_main) << std::endl;

	{
		// create shared data
		shared_t shared;

		// instantiate the first server
		LOG_TRACE("Instantiating first server");
		server_t server{shared, send_path, recv_path, shm_path, 0};
		shared.add_server(&server);

		// initialize the globals for audio master
		globals.shared = &shared;
		AEffect *effect = vst_plugin_main(&audio_master);

		LOG_TRACE("Got effect " << reinterpret_cast<void *>(effect));

		// if we got a null effect, fail
		// TODO: fail gracefully
		if (!effect)
			throw std::runtime_error("SERVER: Got null effect");

		// store the AEffect pointer inside server
		server.effect = effect;

		// return the effect
		server.return_effect();

		// and then process messages
		server.run();
	}

	LOG_TRACE("Destroyed");

	return 0;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR ansiCmdLine, INT cmdShow) {
	try {
		return actual_win_main();
	} catch (std::exception &e) {
		std::cerr << "SERVER: Caught " << typeid(e).name() << ": " << e.what() << std::endl;
		return 1;
	}
}
