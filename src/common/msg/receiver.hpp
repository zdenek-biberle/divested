#ifndef MSG_RECEIVER_HPP
#define MSG_RECEIVER_HPP

#include <array>
#include <stdexcept>

#include "common/log/log.hpp"
#include "common/msg/type.hpp"
#include "pluginterfaces/vst2.x/aeffect.h"

namespace msg {

	// Forward declaration, because otherwise we'll get circular references.
	template <typename T, typename Handler, typename Request, typename Response, Response (*F)(Handler&, const Request&), size_t BufLen>
	void process_message(Handler &handler, std::array<char, BufLen> &buf, size_t offset);

	template <typename Handler>
	static process_response handle_process(Handler &handler, const process_request<float> &req) {
		handler.process(req.inputs, req.outputs, req.index);
		return process_response{};
	}

	template <typename Handler>
	static set_parameter_response handle_set_parameter(Handler &handler, const set_parameter_request &req) {
		handler.set_parameter(req.index, req.opt);
		return set_parameter_response{};
	}

	template <typename Handler>
	static get_parameter_response handle_get_parameter(Handler &handler, const get_parameter_request &req) {
		return get_parameter_response{handler.get_parameter(req.index)};
	}

	template <typename Handler>
	static process_response handle_process_replacing(Handler &handler, const process_request<float> &req) {
		handler.process_replacing(req.inputs, req.outputs, req.index);
		return process_response{};
	}

	template <typename Handler>
	static process_response handle_process_double_replacing(Handler &handler, const process_request<double> &req) {
		handler.process_double_replacing(req.inputs, req.outputs, req.index);
		return process_response{};
	}

	template <typename Handler>
	static instantiate_handler_response handle_instantiate_handler(Handler &handler, const instantiate_handler_request &req) {
		handler.instantiate_handler(req.index);
		return instantiate_handler_response{};
	}

	template <typename Handler, size_t BufLen>
	void dispatch_received_message_to_handler(Handler &handler, std::array<char, BufLen> &buf, size_t offset, type_t msg_type) {
		if (msg_type == type_t::dispatcher) {
			Handler::message_configuration::dispatcher_received::handle(handler, buf, offset);
			return;
		}

		if constexpr (Handler::message_configuration::is_plugin) {
			switch (msg_type) {
				case type_t::process:
					process_message<general::process, Handler, process_request<float>, process_response, &handle_process<Handler>>(handler, buf, offset);
					return;

				case type_t::set_parameter:
					process_message<general::set_parameter, Handler, set_parameter_request, set_parameter_response, &handle_set_parameter<Handler>>(handler, buf, offset);
					return;

				case type_t::get_parameter:
					process_message<general::get_parameter, Handler, get_parameter_request, get_parameter_response, &handle_get_parameter<Handler>>(handler, buf, offset);
					return;

				case type_t::process_replacing:
					process_message<general::process_replacing, Handler, process_request<float>, process_response, &handle_process_replacing<Handler>>(handler, buf, offset);
					return;
					
				case type_t::process_double_replacing:
					process_message<general::process_double_replacing, Handler, process_request<double>, process_response, &handle_process_double_replacing<Handler>>(handler, buf, offset);
					return;

				case type_t::instantiate_handler:
					process_message<general::instantiate_handler, Handler, instantiate_handler_request, instantiate_handler_response, &handle_instantiate_handler<Handler>>(handler, buf, offset);
					return;

				default:
					break;
			}
		}

		throw std::runtime_error("Unknown message type");
	}

	template <typename Handler, size_t BufLen>
	size_t receive_message(Handler &handler, std::array<char, BufLen> &buf) {
		ssize_t readlen = handler.message_read(buf.data(), buf.size());

		LOG_TRACE("read got " << readlen << " B");

		size_t offset = 0;
		type_t msg_type;
		offset += io::read_data(buf.data(), offset, msg_type);

		LOG_TRACE("Got message type " << type_to_name(msg_type));

		if (msg_type == type_t::return_)
			return offset;

		dispatch_received_message_to_handler(handler, buf, offset, msg_type);
		
		return receive_message(handler, buf);
	}
}

#endif
