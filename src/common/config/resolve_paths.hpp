#ifndef CONFIG_RESOLVE_PATHS_HPP
#define CONFIG_RESOLVE_PATHS_HPP

#include <filesystem>
#include "common/config/config_t.hpp"

namespace config {
	/// This resolves all the paths in the config relative to the client path.
	void resolve_paths(config_t &config, const std::filesystem::path &client_path);
}

#endif
