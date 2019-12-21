#include "client/paths_t.hpp"
#include "common/path/build_for_handler.hpp"

paths_t::paths_t(const std::string &send, const std::string &recv, const std::string &shm):
	send(send),
	recv(recv),
	shm(shm)
{}

paths_t::paths_t(const paths_t &base, int index):
	send{path::build_for_handler(base.send, index)},
	recv{path::build_for_handler(base.recv, index)},
	shm{path::build_for_handler(base.shm, index)}
{}
