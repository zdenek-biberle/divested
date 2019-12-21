#include "server/shared_t.hpp"

server_t &shared_t::get_server() {
	auto tid = GetCurrentThreadId();
	auto server = tid_to_server[tid];
	LOG_TRACE("Server for TID " << tid << " requested, got " << reinterpret_cast<void *>(server)); 
	return *server;
}

void shared_t::add_server(server_t *server) {
	std::lock_guard lock{tid_to_server_mutex};
	auto tid = GetCurrentThreadId();
	LOG_TRACE("Adding server " << reinterpret_cast<void *>(server) << " for TID " << tid);
	tid_to_server.insert({tid, server});
}

void shared_t::remove_server() {
	std::lock_guard lock{tid_to_server_mutex};
	tid_to_server.erase(GetCurrentThreadId());
}
