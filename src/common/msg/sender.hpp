#ifndef MSG_SENDER_HPP
#define MSG_SENDER_HPP

#include "common/msg/io/base.hpp"
#include "common/msg/io/buf.hpp"
#include "common/msg/base.hpp"
#include "common/msg/message_name.hpp"
#include "common/msg/receiver.hpp"
#include "common/msg/type.hpp"

namespace msg {
	template <typename Handler, size_t BufLen>
	void send_return(Handler &handler, std::array<char, BufLen> &buf) {
		size_t offset = 0;
		LOG_TRACE("Sending plain return");
		offset += io::write_data(buf.data(), offset, type_t::return_);
		handler.message_write(buf.data(), offset);
	}

	template <typename Handler, size_t BufLen, typename Payload>
	void send_return_payload(Handler &handler, std::array<char, BufLen> &buf, const Payload &payload) {
		size_t offset = 0;
		LOG_TRACE("Sending return with payload of size " << sizeof(Payload));
		offset += io::write_data(buf.data(), offset, type_t::return_);
		offset += io::write_data(buf.data(), offset, payload);
		handler.message_write(buf.data(), offset);
	}

	template <typename Msg, typename Response, typename Request, typename Handler, size_t BufLen>
	void send_return_dispatcher(Handler &handler, std::array<char, BufLen> &buf, size_t shm_offset, size_t shm_size, const Request &request, const Response &response) {
		LOG_TRACE("Sending return for dispatcher message " << message_name<Msg>);

		io::buf pipe{buf.data(), 0};
		io::buf shm{handler.shm(), shm_offset};
		payload_ctx resp_ctx{pipe, shm, handler};

		pipe.write_data(type_t::return_);
		io::write_response<Msg>(resp_ctx, request, response);

		if (shm.total() || shm.total() != shm_size)
			LOG_TRACE("write_response wrote " << shm.total() << " B of shm (compared to " << shm_size << " B)");

		handler.shm_pop(shm_size);
		handler.message_write(buf.data(), pipe.total());	
	}

	template <typename Msg, typename Response, typename Handler, size_t BufLen, typename Body>
	Response send_message(Handler &handler, std::array<char, BufLen> &buf, type_t type, const Body &body) {
		LOG_TRACE("Sending dispatcher call for message " << message_name<Msg>);
		LOG_TRACE(body);

		size_t shm_offset = handler.shm_offset();

		io::buf req_pipe{buf.data(), 0};
		io::buf req_shm{handler.shm(), shm_offset};
		payload_ctx req_ctx{req_pipe, req_shm, handler};

		req_pipe.write_data(type);
		io::write_request<Msg>(req_ctx, body);

		handler.shm_push(req_shm.total());
		handler.message_write(buf.data(), req_pipe.total());

		auto resp_offset = receive_message(handler, buf);

		io::buf resp_pipe{buf.data(), resp_offset};
		io::buf resp_shm{handler.shm(), shm_offset};
		payload_ctx resp_ctx{resp_pipe, resp_shm, handler};

		Response response;
		io::read_response<Msg>(resp_ctx, body, response);

		if (req_shm.total() != resp_shm.total())
			LOG_TRACE("Written shm size (" << req_shm.total() << " B) does not equal read shm size (" << resp_shm.total() << " B)");

		handler.shm_pop(req_shm.total());

		return response;
	}

	template <typename Handler, size_t BufLen>
	VstIntPtr send_dispatcher(Handler &handler, std::array<char, BufLen> &buf, VstInt32 opcode, const dispatcher_request &request) {
		return Handler::message_configuration::dispatcher_sent::send(handler, buf, opcode, request);
	}

	template <typename Handler, size_t BufLen>
	float send_get_parameter(Handler &handler, std::array<char, BufLen> &buf, const get_parameter_request &request) {
		return send_message<general::get_parameter, get_parameter_response>(handler, buf, type_t::get_parameter, request).value;
	}

	template <typename Handler, size_t BufLen>
	void send_set_parameter(Handler &handler, std::array<char, BufLen> &buf, const set_parameter_request &request) {
		send_message<general::set_parameter, set_parameter_response>(handler, buf, type_t::set_parameter, request);
	}

	template <typename Handler, size_t BufLen>
	void send_instantiate_handler(Handler &handler, std::array<char, BufLen> &buf, const instantiate_handler_request &request) {
		send_message<general::instantiate_handler, instantiate_handler_response>(handler, buf, type_t::instantiate_handler, request);
	}
}

#endif
