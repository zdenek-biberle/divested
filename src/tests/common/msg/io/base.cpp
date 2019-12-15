#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <tuple>

#include "common/msg/io/base.hpp"

struct alignas(16) A {};
struct alignas(32) B {};
struct alignas(64) C {};

using align_offset_types = std::tuple<char, short, int, double, A, B, C>;

BOOST_AUTO_TEST_CASE_TEMPLATE(align_offset, T, align_offset_types) {
	for (size_t i = 0; i < 100; i++) {
		BOOST_TEST(msg::io::align_offset<T>(i) % alignof(T) == 0);
	}
}
