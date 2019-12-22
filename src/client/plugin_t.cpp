#include <fcntl.h>
#include <spawn.h>
#include <sys/stat.h>
#include <unistd.h>
#include "client/array_buffer.hpp"
#include "client/callbacks.hpp"
#include "client/created_resources_t.hpp"
#include "client/plugin_t.hpp"

static created_resources_t create_resources(const paths_t &base_paths, int index) {
	paths_t paths{base_paths, index};

	auto send_res = ::mkfifo(paths.send.c_str(), S_IRUSR | S_IWUSR);
	auto recv_res = ::mkfifo(paths.recv.c_str(), S_IRUSR | S_IWUSR);

	LOG_TRACE("mkfifo send: " << send_res);
	LOG_TRACE("mkfifo recv: " << recv_res);

	shm::shm_t shm{shm::shm_t::create(paths.shm)};

	return {paths, std::move(shm)};
}

static client_t create_client(shared_t &shared, created_resources_t &&res) {
	// open the pipes
	LOG_TRACE("Opening recv pipe " << res.paths.recv);
	int recv_fd = ::open(res.paths.recv.c_str(), O_RDONLY);
	LOG_TRACE("Opening send pipe " << res.paths.send);
	int send_fd = ::open(res.paths.send.c_str(), O_WRONLY);

	// opening the pipes should block until the server has opened them as
	// well, so now we can delete them so that they do not stick around
	// when/if this process dies
	::unlink(res.paths.send.c_str());
	::unlink(res.paths.recv.c_str());

	// now we instantiate the client
	return client_t{send_fd, recv_fd, std::move(res.shm), shared};
}

static client_t create_primary_client(const paths_t &base_paths, const std::string &server_path, const std::string &dll_path, shared_t &shared) {
	// First, create the shared resources
	auto res = create_resources(base_paths, 0);

	// Next, spawn the server so that it can open the shared resources
	const char *const server_argv[] = {
		server_path.c_str(),
		dll_path.c_str(),
		base_paths.send.c_str(),
		base_paths.recv.c_str(),
		base_paths.shm.c_str(),
		nullptr
	};
	pid_t server_pid;
	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);
	posix_spawnattr_t attrs;
	posix_spawnattr_init(&attrs);
	LOG_TRACE(server_path << " " << dll_path << " " << res.paths.send << " " << res.paths.recv << " " << res.paths.shm);
	LOG_TRACE(const_cast<char *const*>(server_argv));
	int spawn_result = ::posix_spawn(
			&server_pid,
			server_path.c_str(),
			&actions,
			&attrs,
			const_cast<char *const*>(server_argv), // according to man exec, the cast should be fine
			environ
		);

	LOG_TRACE("posix_spawn: " << spawn_result);

	if (spawn_result != 0) {
		std::stringstream ss;
		ss << "posix_spawn failed: " << ::strerror(errno);
		throw std::runtime_error(ss.str());
	}

	// Next we instantiate our client
	return create_client(shared, std::move(res));
}

plugin_t::plugin_t(const paths_t &paths, const std::string &server_path, const std::string &dll_path, audioMasterCallback cb):
	base_paths{paths},
	shared{cb},
	primary_client{create_primary_client(base_paths, server_path, dll_path, shared)}
{}

client_t &plugin_t::get_client() {
	auto tid = gettid();
	std::lock_guard lock(primary_client_mutex); // TODO: don't lock the mutex when accessing a client that already exists

	if (auto it = tid_to_client.find(tid) ; it != tid_to_client.end()) {
		return *(it->second.get());
	} else {
		LOG_TRACE("Creating new server handler");
		auto index = tid_to_client.size() + 1;
		auto res{create_resources(base_paths, index)};
		primary_client.instantiate_handler(index);
		auto client = create_client(shared, std::move(res));
		auto client_ptr = std::make_unique<client_t>(std::move(client));
		auto [client_it, inserted] = tid_to_client.insert({tid, std::move(client_ptr)});

		if (!inserted) {
			throw std::runtime_error("Client was already in tid_to_client map");
		}

		return *(client_it->second.get());
	}
}

static void log_proc_addresses(AEffect *eff) {
	LOG_TRACE("Procs: "
		<< "    object: " << eff->object << std::endl
		<< "    user: " << eff->user << std::endl
		<< "    dispatcher: " << reinterpret_cast<void*>(eff->dispatcher) << std::endl
		<< "    process: " << reinterpret_cast<void*>(eff->process) << std::endl
		<< "    setParameter: " << reinterpret_cast<void*>(eff->setParameter) << std::endl
		<< "    getParameter: " << reinterpret_cast<void*>(eff->getParameter) << std::endl
		<< "    processReplacing: " << reinterpret_cast<void*>(eff->processReplacing) << std::endl
		<< "    processDoubleReplacing: " << reinterpret_cast<void*>(eff->processDoubleReplacing));
}

void plugin_t::wait_for_effect() {
	LOG_TRACE("Waiting for AEffect (sizeof " << sizeof(AEffect) << ")");

	array_buffer buf;
	size_t offset = msg::receive_message(primary_client, buf);
	msg::io::read_data(buf.data(), offset, shared.effect);

	LOG_TRACE("Received AEffect, patching...");

	log_proc_addresses(&shared.effect);
	shared.effect.dispatcher = &aeffect_dispatcher_proc;
	shared.effect.process = &aeffect_process_proc;
	shared.effect.setParameter = &aeffect_set_parameter_proc;
	shared.effect.getParameter = &aeffect_get_parameter_proc;
	shared.effect.processReplacing = &aeffect_process_replacing_proc;
	shared.effect.processDoubleReplacing = &aeffect_process_double_replacing_proc;
	shared.effect.user = reinterpret_cast<void*>(this);
	shared.effect.object = nullptr; // TODO: try setting this to a not null value when everything else works to see if the DAW breaks
	shared.effect.flags &= ~effFlagsHasEditor;
	log_proc_addresses(&shared.effect);

	shared.state = client_state_t::created;
}
