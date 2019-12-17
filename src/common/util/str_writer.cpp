#include "common/util/str_writer.hpp"

namespace util {
	static bool has_zero(const str_writer &str) {
		for (size_t i = 0; i < str.size; i++)
			if (str.ptr[i] == '\0')
				return true;

		return false;
	}

	std::ostream &operator<<(std::ostream &os, const str_writer &str) {
		if (has_zero(str))
			return os << "\"" << str.ptr << "\"";
		else
			return os << "invalid";
	}
}
