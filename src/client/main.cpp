#include <dlfcn.h>
#include <filesystem>
#include "client/plugin_t.hpp"
#include "common/log/log.hpp"
#include "common/util/random_string.hpp"
#include "pluginterfaces/vst2.x/aeffect.h"

namespace fs = std::filesystem;

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
