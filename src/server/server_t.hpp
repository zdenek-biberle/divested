#ifndef SERVER_T_HPP
#define SERVER_T_HPP

#include "common/handler/shm.hpp"
#include "common/msg/dispatchers.hpp"
#include "pluginterfaces/vst2.x/aeffect.h"

struct shared_t;

struct server_t : public handler::with_shm {
	shared_t &shared;
	int send_fd;
	int recv_fd;
	AEffect* effect;
	const std::string &send_base_path;
	const std::string &recv_base_path;
	const std::string &shm_base_path;
	int index;

	struct message_configuration {
		using dispatcher_received = msg::effect_dispatcher;
		using dispatcher_sent = msg::host_dispatcher;
		static constexpr bool is_plugin = true;
	};

	server_t(shared_t &shared, const std::string &send_base_path, const std::string &recv_base_path, const std::string &shm_base_path, int index);
	~server_t();

	VstIntPtr dispatcher(VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
	float get_parameter(VstInt32 index);
	void set_parameter(VstInt32 index, float opt);
	void process(float **inputs, float **outputs, VstInt32 samples);
	void process_replacing(float **inputs, float **outputs, VstInt32 samples);
	void process_double_replacing(double **inputs, double **outputs, VstInt32 samples);
	void instantiate_handler(int index);

	ssize_t message_read(char *buf, size_t len);
	ssize_t message_write(const char *buf, size_t len);

	/// Return the AEffect to the client
	void return_effect();

	/// Keep processing messages until a return is received
	void run();
};

#endif
