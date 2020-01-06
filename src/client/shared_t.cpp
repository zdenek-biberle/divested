#include "client/shared_t.hpp"
#include "common/log/log.hpp"

char *shared_t::allocate_chunk(size_t size) {
	auto chunk = std::make_unique<char[]>(size);
	auto ptr = chunk.get();
	std::lock_guard lock{allocated_chunks_mutex};
	allocated_chunks.push_back(std::move(chunk));
	return ptr;
}

void shared_t::clear_chunks() {
	LOG_TRACE("Cleaning up " << allocated_chunks.size() << " allocated chunks");
	std::lock_guard lock{allocated_chunks_mutex};
	allocated_chunks.clear();
}
