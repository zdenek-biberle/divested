#ifndef LOG_LOG_HPP
#define LOG_LOG_HPP

#include <sstream>

namespace log {
	/// A global logger! Yay!
	void log(const std::stringstream &msg);

	#define LOG_TRACE(x) do { \
		std::stringstream ss; \
		ss << x; \
		::log::log(ss); \
	} while(false);
}

#endif
