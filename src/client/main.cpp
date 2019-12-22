#include <dlfcn.h>
#include <filesystem>
#include "client/plugin_t.hpp"
#include "common/config/config_t.hpp"
#include "common/config/read.hpp"
#include "common/config/resolve_paths.hpp"
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
	auto dladdr_res = ::dladdr(reinterpret_cast<void *>(&VSTPluginMain), &dl_info);
	LOG_TRACE("dladdr=" << dladdr_res);
	fs::path client_path = dl_info.dli_fname;
	LOG_TRACE("client_path=" << client_path);
	
	config::config_t config;

	// read the config if it is present
	// TODO: also look in a user-wide or system-wide config? $XDG_CONFIG_HOME/divested.conf?
	{
		auto conf_path = client_path.replace_extension("conf");
		auto conf_path_type = fs::status(conf_path).type();
		
		switch (conf_path_type) {
			case fs::file_type::not_found:
				// do nothing
				break;

			case fs::file_type::regular:
				config::read(config, conf_path);
				break;

			default:
				LOG_TRACE("Ignoring configuration file " << conf_path << " because it's not a regular file");
				break;
		}
	}

	config::resolve_paths(config, client_path.parent_path());

	// if the final plugin_path points to a directory, append the client_name with a dll extension to it
	if (fs::is_directory(config.plugin_path)) {
		config.plugin_path = config.plugin_path / client_path.filename().replace_extension("dll");
	}

	LOG_TRACE("Final configuration: " << config);

	// now we instantiate the plugin
	plugin_t *plugin = new plugin_t{paths_t{c2s_path, s2c_path, shm_path}, config.server_path, config.plugin_path, audio_master};
	plugin->wait_for_effect();
	auto effect = &(plugin->shared.effect);
	LOG_TRACE("main done with effect " << reinterpret_cast<void*>(effect) << " returned to host");

	return effect;
}
