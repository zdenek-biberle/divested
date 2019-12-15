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
	static void handle_process(Handler &handler, char *buf, size_t offset) {
		throw std::runtime_error("Don't yet know how to handle processing.");
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
	static void handle_process_replacing(Handler &handler, char *buf, size_t offset) {
		throw std::runtime_error("Don't yet know how to handle `process_replacing`.");
	}

	template <typename Handler>
	static void handle_process_double_replacing(Handler &handler, char *buf, size_t offset) {
		throw std::runtime_error("Don't yet know how to handle `process_double_replacing`.");
	}

	template <typename Handler, size_t BufLen>
	size_t receive_message(Handler &handler, std::array<char, BufLen> &buf) {
		ssize_t readlen = handler.message_read(buf);

		log::log() << "read got " << readlen << " B" << std::endl;

		size_t offset = 0;
		type_t msg_type;
		offset += io::read_data(buf.data(), offset, msg_type);

		log::log() << "Got message type " << type_to_name(msg_type) << std::endl;

		if (msg_type == type_t::return_)
			return offset;

		if constexpr (Handler::message_configuration::is_plugin) {
			switch (msg_type) {
				case type_t::dispatcher:
					Handler::message_configuration::dispatcher_received::handle(handler, buf, offset);
					break;

				case type_t::process:
					handle_process(handler, buf.data(), offset);
					break;

				case type_t::set_parameter:
					process_message<general::set_parameter, Handler, set_parameter_request, set_parameter_response, &handle_set_parameter<Handler>>(handler, buf, offset);
					break;

				case type_t::get_parameter:
					process_message<general::get_parameter, Handler, get_parameter_request, get_parameter_response, &handle_get_parameter<Handler>>(handler, buf, offset);
					break;

				case type_t::process_replacing:
					handle_process_replacing(handler, buf.data(), offset);
					break;
					
				case type_t::process_double_replacing:
					handle_process_double_replacing(handler, buf.data(), offset);
					break;

				default:
					throw std::runtime_error("Unknown message type");
			}
		} else {
			switch (msg_type) {
				case type_t::dispatcher:
					Handler::message_configuration::dispatcher_received::handle(handler, buf, offset);
					break;

				default:
					throw std::runtime_error("Unknown message type");
			}
		}
		
		return receive_message(handler, buf);
	}
}

#endif
