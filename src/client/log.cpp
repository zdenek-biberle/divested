#include <iostream>
#include <unistd.h>
#include "common/log/log.hpp"

namespace log {
	void log(const std::stringstream &msg) {
		std::stringstream ss;
		ss << "CLIENT " << gettid() << ": " << msg.str() << std::endl;
		std::cerr << ss.str();
	}
}
