#include <fcntl.h>
#include <unistd.h>
#include <windows.h>
#include "common/path/build_for_handler.hpp"
#include "server/server_t.hpp"
#include "server/shared_t.hpp"

shm::shm_t open_shm(const std::string &base, int index) {
	auto path = path::build_for_handler(base, index);
	LOG_TRACE("Opening shm " << path);

	// open the shared memory
	shm::shm_t shm{shm::shm_t::open(path)};

	// now unlink the shared memory, because we do not need the file anymore
	shm.unlink();
	
	// and return it
	return shm;
}

int open_pipe(const std::string &base, int index, int omode) {
	// Get the pipe's path
	auto path = path::build_for_handler(base, index);

	LOG_TRACE("Opening pipe " << path);

	// Try to open the pipe
	int fd = ::open(path.c_str(), omode);

	// Throw if it failed
	if (fd == -1) {
		std::stringstream ss;
		ss << "Opening pipe " << path << " failed: " << ::strerror(errno);
		throw std::runtime_error(ss.str());
	}
	
	// and return it otherwise
	return fd;
}

server_t::server_t(shared_t &shared, const std::string &send_base_path, const std::string &recv_base_path, const std::string &shm_base_path, int index):
	handler::with_shm{open_shm(shm_base_path, index)},
	shared{shared},
	send_fd{open_pipe(send_base_path, index, O_WRONLY)},
	recv_fd{open_pipe(recv_base_path, index, O_RDONLY)},
	effect{nullptr},
	send_base_path{send_base_path},
	recv_base_path{recv_base_path},
	shm_base_path{shm_base_path},
	index{index}
{
	LOG_TRACE("Instantiated server with index=" << index << " send_base_path=" << send_base_path << " recv_base_path=" << recv_base_path << " shm_base_path=" << shm_base_path);
}

server_t::~server_t() {
	::close(send_fd);
	::close(recv_fd);
}

struct start_thread_t {
	const server_t *base_server;
	int index;
};

static DWORD WINAPI start_thread(void *ptr) {
	auto data = static_cast<start_thread_t *>(ptr);
	auto [base_server, index] = *data;
	delete data;

	LOG_TRACE("Starting new server with index " << index);
	try {
		server_t server{base_server->shared, base_server->send_base_path, base_server->recv_base_path, base_server->shm_base_path, index};
		server.effect = base_server->effect;
		base_server->shared.add_server(&server);
		server.run();
		base_server->shared.remove_server();
	} catch (std::exception &ex) {
		LOG_TRACE("Server with index " << index << " failed: " << ex.what());
		return 1;
	}

	return 0;
}

VstIntPtr server_t::dispatcher(VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {
	return effect->dispatcher(effect, opcode, index, value, ptr, opt);
}

float server_t::get_parameter(VstInt32 index) {
	return effect->getParameter(effect, index);
}

void server_t::set_parameter(VstInt32 index, float opt) {
	effect->setParameter(effect, index, opt);
}

void server_t::process(float **inputs, float **outputs, VstInt32 samples) {
	effect->process(effect, inputs, outputs, samples);
}

void server_t::process_replacing(float **inputs, float **outputs, VstInt32 samples) {
	effect->processReplacing(effect, inputs, outputs, samples);
}

void server_t::process_double_replacing(double **inputs, double **outputs, VstInt32 samples) {
	effect->processDoubleReplacing(effect, inputs, outputs, samples);
}

void server_t::instantiate_handler(int index) {
	LOG_TRACE("Starting thread for server with index " << index);
	auto data = new start_thread_t{this, index};
	CreateThread(nullptr, 0, &start_thread, data, 0, nullptr);
	LOG_TRACE("Thread started");
}


ssize_t server_t::message_read(char *buf, size_t len) {
	LOG_TRACE("Reading " << len << " B...");
	ssize_t readlen = ::read(recv_fd, buf, len);
	LOG_TRACE("Read " << readlen << " B");

	if (readlen == 0)
		throw std::runtime_error("Unexpected end of pipe");
	
	return readlen;
}

ssize_t server_t::message_write(const char *buf, size_t len) {
	LOG_TRACE("Writing " << len << " B...");
	ssize_t writelen = ::write(send_fd, buf, len);
	LOG_TRACE("Wrote " << len << " B");

	if (static_cast<ssize_t>(len) != writelen)
		throw std::runtime_error("Didn't write expected number of bytes");

	return writelen;
}

/// Return the AEffect to the client
void server_t::return_effect() {
	LOG_TRACE("Server " << index << " returning AEffect to client");
	std::array<char, 2048> buf;
	msg::send_return_payload(*this, buf, *effect);
}

/// Keep processing messages until a return is received
void server_t::run() {
	LOG_TRACE("Server " << index << " will now start handling messages");
	std::array<char, 2048> buf;
	msg::receive_message(*this, buf);
	LOG_TRACE("Server " << index << " finished");
}
