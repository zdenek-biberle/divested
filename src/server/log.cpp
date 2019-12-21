#include <iostream>
#include <windows.h>
#include "common/log/log.hpp"

namespace log {
	void log(const std::stringstream &msg) {
		std::stringstream ss;
		ss << "SERVER " << GetCurrentThreadId() << ": " << msg.str() << std::endl;
		std::cerr << ss.str();
	}
}
