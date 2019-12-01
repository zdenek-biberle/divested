#include <algorithm>
#include <random>
#include <string>

#include "common/util/random_string.hpp"

namespace util {
	constexpr const char *charset = {"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};

	std::string random_string(size_t length) {
		std::mt19937 gen{std::random_device{}()};
		std::uniform_int_distribution<> distrib{0, 61};

		std::string str(length, 0);
		std::generate(str.begin(), str.end(), [&](){
			return charset[distrib(gen)];
		});
		return str;
	}
}
