#ifndef SHARED_T_HPP
#define SHARED_T_HPP

#include <memory>
#include <vector>
#include "client/client_state_t.hpp"
#include "pluginterfaces/vst2.x/aeffect.h"

struct shared_t {
	AEffect effect;
	audioMasterCallback cb;
	client_state_t state = client_state_t::creating;
	std::vector<std::unique_ptr<char[]>> allocated_chunks; // TODO: this should be either not shared or protected by a mutex

	inline shared_t(audioMasterCallback cb):
		cb{cb}
	{}
};

#endif
