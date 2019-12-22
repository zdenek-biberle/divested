#include <libconfig.h++>
#include "common/config/read.hpp"

namespace config {
	namespace fs = std::filesystem;

	void read(config_t &outcfg, const fs::path &path) {
		libconfig::Config incfg;
		incfg.readFile(path.native().c_str());
		
		for (auto &setting : incfg.getRoot()) {
			std::string name = setting.getName();

			if (name == "server_path")
				outcfg.server_path = fs::path{setting};
			else if (name == "plugin_path")
				outcfg.plugin_path = fs::path{setting};
			else {
				std::stringstream ss;
				ss << "Unknown setting: " << name;
				throw std::runtime_error(ss.str());
			}
		}
	}

}
