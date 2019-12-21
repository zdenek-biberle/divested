#ifndef CLIENT_T_HPP
#define CLIENT_T_HPP

#include "client/shared_t.hpp"
#include "common/handler/shm.hpp"
#include "common/msg/dispatchers.hpp"

struct client_t : public handler::with_shm {
	int send_fd;
	int recv_fd;
	shared_t &shared;
	ERect rect;

	struct message_configuration {
		using dispatcher_received = msg::host_dispatcher;
		using dispatcher_sent = msg::effect_dispatcher;
		static constexpr bool is_plugin = false;
		static constexpr std::array<msg::type_t, 1> supported_messages = {msg::type_t::dispatcher};
	};

	client_t(int send_fd, int recv_fd, shm::shm_t &&shm, shared_t &shared);
	client_t(client_t &&other);
	~client_t();

	VstIntPtr dispatcher(VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);

	ssize_t message_read(char *buf, size_t len);
	ssize_t message_write(const char *buf, size_t len);

	char *allocate_chunk(size_t size);
	ERect &get_rect();

	void instantiate_handler(int index);
};

#endif
