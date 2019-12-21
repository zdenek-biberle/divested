#ifndef PATHS_T_HPP
#define PATHS_T_HPP

#include <string>

struct paths_t {
	std::string send;
	std::string recv;
	std::string shm;

	paths_t(const std::string &send, const std::string &recv, const std::string &shm);
	paths_t(const paths_t &base, int index);
};

#endif
