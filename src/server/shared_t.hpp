#ifndef SHARED_T_HPP
#define SHARED_T_HPP

#include <map>
#include <mutex>
#include <windows.h>
#include "server/server_t.hpp"

struct server_t;

struct shared_t {
	std::mutex tid_to_server_mutex;
	std::map<DWORD, server_t*> tid_to_server; // TODO: make the map writes atomic

	server_t &get_server();
	void add_server(server_t *server);
	void remove_server();
};

#endif
