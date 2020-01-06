#ifndef SHARED_T_HPP
#define SHARED_T_HPP

#include <memory>
#include <mutex>
#include <vector>
#include "client/client_state_t.hpp"
#include "pluginterfaces/vst2.x/aeffect.h"

struct shared_t {
	AEffect effect;
	audioMasterCallback cb;
	client_state_t state = client_state_t::creating;

	inline shared_t(audioMasterCallback cb):
		cb{cb}
	{}

	/// This allocates a chunk. Thread-safe.
	char *allocate_chunk(size_t size);

	/// This removes all allocated chunks. Thread-safe.
	void clear_chunks();

private:
	std::vector<std::unique_ptr<char[]>> allocated_chunks;
	std::mutex allocated_chunks_mutex;
};

#endif
