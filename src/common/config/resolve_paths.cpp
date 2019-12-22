#include "common/config/resolve_paths.hpp"

namespace config {
	namespace fs = std::filesystem;
	void resolve_paths(config_t &config, const fs::path &client_path) {
		config.server_path = client_path / config.server_path;
		config.plugin_path = client_path / config.plugin_path;
	}
}
