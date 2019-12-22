#ifndef CONFIG_READ_HPP
#define CONFIG_READ_HPP

#include <filesystem>
#include "common/config/config_t.hpp"

namespace config {
	void read(config_t &config, const std::filesystem::path &path);
}

#endif

