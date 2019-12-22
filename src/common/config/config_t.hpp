#ifndef CONFIG_CONFIG_T_HPP
#define CONFIG_CONFIG_T_HPP

#include <filesystem>
#include <ostream>
#include "common/build/options.hpp"

namespace config {
	struct config_t {
		/// A path to the server. Either relative to the client or absolute.
		std::filesystem::path server_path = build::default_server_path;

		/// A path to the plugin. Either relative to the client or absolute.
		/// In the case that this points to a directory, the client's filename
		/// with a dll extensions is appended to it.
		std::filesystem::path plugin_path = build::default_plugin_path;
	};

	std::ostream &operator<<(std::ostream &os, const config_t &config);
}

#endif
