#include "common/config/config_t.hpp"

namespace config {
	std::ostream &operator<<(std::ostream &os, const config_t &config) {
		return os << "config_t{ server_path=" << config.server_path << " plugin_path=" << config.plugin_path << " }";
	}
}
