#include <codecvt>
#include <locale>
#include "common/log/log.hpp"
#include "pluginterfaces/vst2.x/aeffect.h"
#include "server/globals_t.hpp"
#include "server/server_t.hpp"
#include "server/shared_t.hpp"

typedef AEffect *(VSTCALLBACK *vst_plugin_main_t)(audioMasterCallback);

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
		LOG_TRACE(i << ": " << wstr_conv(argv[i]));

	LOG_TRACE("sizeof wchar_t: " << sizeof(wchar_t));
	LOG_TRACE("argc: " << argc);
	LOG_TRACE("cmd line: " << wstr_conv(cmd_line));

	// open the pipes
	auto recv_path = wstr_conv(argv[2]);
	auto send_path = wstr_conv(argv[3]);
	auto shm_path = wstr_conv(argv[4]);
	
	// load the library and find the VSTPluginMain procedure
	HMODULE lib = LoadLibraryW(argv[1]);
	LOG_TRACE("lib: " << lib);
	vst_plugin_main_t vst_plugin_main = (vst_plugin_main_t) GetProcAddress(lib, "VSTPluginMain");
	LOG_TRACE("vst plugin main: " << reinterpret_cast<void *>(vst_plugin_main))

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

		// return the effect to the client
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
		LOG_TRACE("Caught " << typeid(e).name() << ": " << e.what());
		return 1;
	}
}
