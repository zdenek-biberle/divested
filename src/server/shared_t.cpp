#include "server/shared_t.hpp"

server_t &shared_t::get_server() {
	auto tid = GetCurrentThreadId();
	std::shared_lock lock{tid_to_server_mutex};
	auto server = tid_to_server[tid];
	LOG_TRACE("Server for TID " << tid << " requested, got " << reinterpret_cast<void *>(server)); 
	return *server;
}

void shared_t::add_server(server_t *server) {
	auto tid = GetCurrentThreadId();
	std::unique_lock lock{tid_to_server_mutex};
	LOG_TRACE("Adding server " << reinterpret_cast<void *>(server) << " for TID " << tid);
	tid_to_server.insert({tid, server});
}

void shared_t::remove_server() {
	std::unique_lock lock{tid_to_server_mutex};
	tid_to_server.erase(GetCurrentThreadId());
}
