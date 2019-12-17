#ifndef UTIL_STR_WRITER_HPP
#define UTIL_STR_WRITER_HPP

#include <ostream>

namespace util {
	/// This is a simple helper for writing out C strings that might not include
	/// a terminating zero.
	struct str_writer {
		const char * const ptr;
		const size_t size;

		template <size_t Size>
		str_writer(const char (&str)[Size]):
			ptr{str}, size{Size}
		{}

		str_writer(const char *ptr, size_t size):
			ptr{ptr}, size{size}
		{}

		friend std::ostream &operator<<(std::ostream &os, const str_writer &str);
	};
}

#endif
