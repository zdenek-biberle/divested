#include <array>
#include <unistd.h>
#include "client/array_buffer.hpp"
#include "client/client_t.hpp"
#include "common/log/log.hpp"

client_t::client_t(int send_fd, int recv_fd, shm::shm_t &&shm, shared_t &shared):
	handler::with_shm{std::move(shm)},
	send_fd{send_fd},
	recv_fd{recv_fd},
	shared{shared}
{
	LOG_TRACE("Instantiated client with send_fd " << send_fd << ", recv_fd " << recv_fd << " & shm " << _shm.name());
}

client_t::client_t(client_t &&other):
	handler::with_shm{std::move(other)},
	send_fd(other.send_fd),
	recv_fd(other.recv_fd),
	shared(other.shared),
	rect(other.rect)
{
	other.send_fd = -1;
	other.recv_fd = -1;
}

static void close_pipe(int fd) {
	if (fd != -1)
		::close(fd);
}

client_t::~client_t() {
	close_pipe(send_fd);
	close_pipe(recv_fd);
}

VstIntPtr client_t::dispatcher(VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {

	auto effect_ptr = shared.state == client_state_t::created ? &shared.effect : nullptr;
	LOG_TRACE("Dispatching to master, effect " << reinterpret_cast<void*>(effect_ptr)
		<< ", opcode " << opcode
		<< ", index " << index
		<< ", value " << value
		<< ", ptr " << ptr
		<< ", opt " << opt);
	auto result = shared.cb(effect_ptr, opcode, index, value, ptr, opt);
	LOG_TRACE("Got result " << result);
	return result;
}

ssize_t client_t::message_read(char *buf, size_t len) {
	LOG_TRACE("Reading " << len << " B...");
	ssize_t readlen = ::read(recv_fd, buf, len);
	LOG_TRACE("Read " << readlen << " B");

	if (readlen == 0)
		throw std::runtime_error("Unexpected end of pipe");
	
	return readlen;
}

ssize_t client_t::message_write(const char *buf, size_t len) {
	LOG_TRACE("Writing " << len << " B...");
	ssize_t writelen = ::write(send_fd, buf, len);
	LOG_TRACE("Wrote " << len << " B");

	if (static_cast<ssize_t>(len) != writelen)
		throw std::runtime_error("Didn't write expected number of bytes");

	return writelen;
}

char *client_t::allocate_chunk(size_t size) {
	auto chunk = std::make_unique<char[]>(size);
	auto ptr = chunk.get();
	shared.allocated_chunks.push_back(std::move(chunk));
	return ptr;
}

ERect &client_t::get_rect() {
	return rect;
}

void client_t::instantiate_handler(int index) {
	array_buffer buf;
	msg::send_instantiate_handler(*this, buf, {index}); 
}
