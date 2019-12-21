#ifndef PLUGIN_T_HPP
#define PLUGIN_T_HPP

#include <map>
#include <mutex>
#include <string>
#include <sys/types.h>
#include "client/client_t.hpp"
#include "client/paths_t.hpp"
#include "client/shared_t.hpp"

struct plugin_t {
	paths_t base_paths;
	shared_t shared;
	client_t primary_client;
	std::mutex primary_client_mutex;
	std::map<pid_t, std::unique_ptr<client_t>> tid_to_client; // This should somehow be protected from concurrent writes

	plugin_t(const paths_t &paths, const std::string &server_path, const std::string &dll_path, audioMasterCallback cb);

	client_t &get_client();

	void wait_for_effect();
};

#endif
