#include <cassert>
#include <codecvt>
#include <cstring>
#include <locale.h>
#include <locale>
#include <iostream>
#include <cstdio>
#include <sstream>
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
#include "common/shm/shm.hpp"
#include "pluginterfaces/vst2.x/aeffect.h"

namespace log {
	std::ostream &log() {
		return std::cerr << "SERVER: ";
	}
}

typedef AEffect *(VSTCALLBACK *vst_plugin_main_t)(audioMasterCallback);

struct server_t : public handler::with_shm {
	int send_fd;
	int recv_fd;
	AEffect* effect;

	struct message_configuration {
		using dispatcher_received = msg::effect_dispatcher;
		using dispatcher_sent = msg::host_dispatcher;
	};

	server_t(int send_fd, int recv_fd, shm::shm_t &&shm):
		handler::with_shm(std::move(shm)),
		send_fd(send_fd),
		recv_fd(recv_fd),
		effect(nullptr)
	{
		log() << "Instantiated server with send_fd " << send_fd << ", recv_fd " << recv_fd << " and shm " << _shm.name() << std::endl;
	}

	~server_t() {
		::close(send_fd);
		::close(recv_fd);
	}

	std::ostream &log() {
		return std::cerr << "SERVER: ";
	}

	VstIntPtr dispatcher(VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {
		return effect->dispatcher(effect, opcode, index, value, ptr, opt);
	}

	template<size_t BufLen>
	ssize_t message_read(std::array<char, BufLen> &buf) {
		log() << "Reading " << buf.size() << " B..." << std::endl;
		ssize_t readlen = ::read(recv_fd, buf.data(), buf.size());
		log() << "Read " << readlen << " B" << std::endl;
		
		return readlen;
	}

	ssize_t message_write(const char *buf, size_t len) {
		log() << "Writing " << len << " B..." << std::endl;
		ssize_t writelen = ::write(send_fd, buf, len);
		log() << "Wrote " << len << " B" << std::endl;

		if (static_cast<ssize_t>(len) != writelen)
			throw std::runtime_error("Didn't write expected number of bytes");

		return writelen;
	}

	void *shm() {
		return _shm.memory();
	}

	/// Returns the effect to the client
	void run() {
		std::array<char, 2048> buf;

		// Start by returning the constructed effect to the client
		msg::send_return_payload(*this, buf, *effect);

		// ...and then just process messages
		msg::receive_message(*this, buf);

		log::log() << "Finished" << std::endl;
	}
};

struct globals_t {
	server_t *server;
};

globals_t globals;

VstIntPtr VSTCALLBACK audio_master(AEffect *effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt) {
	printf("SERVER: master: effect %p, opcode %d, index %d, value %ld, ptr %p, opt %f\n", effect, opcode, index, value, ptr, opt);
	std::array<char, 2048> buf;

	return msg::send_dispatcher(*globals.server, buf, opcode, index, value, ptr, opt);
}

std::string wstr_conv(const WCHAR *str) {
	std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> cvt;
	return cvt.to_bytes(reinterpret_cast<const char16_t *>(str));
}

int actual_win_main() {
	std::cout.rdbuf()->pubsetbuf(0, 0);
	std::cerr.rdbuf()->pubsetbuf(0, 0);

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
	int recv_fd = ::open(recv_path.c_str(), O_RDONLY);

	if (recv_fd == -1) {
		std::stringstream ss;
		ss << "Opening recv_path " << recv_path << " failed: " << ::strerror(errno);
		throw std::runtime_error(ss.str());
	}

	auto send_path = wstr_conv(argv[3]);
	int send_fd = ::open(send_path.c_str(), O_WRONLY);

	if (send_fd == -1) {
		std::stringstream ss;
		ss << "Opening send_path " << send_path << " failed: " << ::strerror(errno);
		throw std::runtime_error(ss.str());
	}

	std::cout << "SERVER: recv: " << recv_fd << ", send: " << send_fd << std::endl;

	// open the shared memory
	auto shm_path = wstr_conv(argv[4]);
	shm::shm_t shm{shm::shm_t::open(shm_path)};

	// now unlink the shared memory, because we do not need the file anymore
	shm.unlink();
	
	// load the library and find the VSTPluginMain procedure
	HMODULE lib = LoadLibraryW(argv[1]);
	std::cout << "SERVER: lib: " << lib << std::endl;
	vst_plugin_main_t vst_plugin_main = (vst_plugin_main_t) GetProcAddress(lib, "VSTPluginMain");
	std::cout << "SERVER: vst plugin main: " << vst_plugin_main << std::endl;

	{
		// instantiate the server
		server_t server{send_fd, recv_fd, std::move(shm)};

		// initialize the globals for audio master
		globals.server = &server;
		AEffect *effect = vst_plugin_main(&audio_master);

		if (!effect)
			throw std::runtime_error("SERVER: Got null effect");

		// store the AEffect pointer inside server
		server.effect = effect;

		printf("effect: %p\n", effect);
		server.run();
	}

	log::log() << "Destroyed" << std::endl;

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
