#ifndef CREATED_RESOURCES_T_HPP
#define CREATED_RESOURCES_T_HPP

#include "client/paths_t.hpp"
#include "common/shm/shm.hpp"

struct created_resources_t {
	paths_t paths;
	shm::shm_t shm;
};

#endif
