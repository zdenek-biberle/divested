#include <sstream>

#include "common/path/build_for_handler.hpp"

namespace path {
	std::string build_for_handler(const std::string &base, int index) {
		std::stringstream ss;
		ss << base << "." << index;
		return ss.str();
	}
}
