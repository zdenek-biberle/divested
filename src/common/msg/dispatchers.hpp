#ifndef MSG_DISPATCHERS_HPP
#define MSG_DISPATCHERS_HPP

#include "common/msg/effect/messages.hpp"
#include "common/msg/io/buf.hpp"
#include "common/msg/io/msg.hpp"
#include "common/msg/master/messages.hpp"
#include "common/msg/base.hpp"
#include "common/msg/processor.hpp"
#include "common/msg/receiver.hpp"
#include "common/msg/sender.hpp"

namespace msg {

	[[noreturn]] static void throw_unknown_opcode(const char *prefix, VstInt32 opcode) {
		std::stringstream ss;
		ss << prefix << ": unknown opcode " << opcode;
		throw std::runtime_error(ss.str());
	}

	// message directed at the host's dispatcher
	struct host_dispatcher {
		static constexpr type_t type = type_t::dispatcher;

		template <typename Handler, size_t BufLen>
		static void handle(Handler &handler, std::array<char, BufLen> &buf, size_t offset) {
			VstInt32 opcode;
			offset += io::read_data(buf.data(), offset, opcode);

			#define OPCODE_HANDLER(MSG) case master::MSG::opcode: \
				process_message<master::MSG, Handler, dispatcher_request, dispatcher_response, &run<master::MSG, Handler>>(handler, buf, offset); \
				break;

			switch (opcode) {
				MSG_MASTER_MESSAGES(OPCODE_HANDLER)
				default: throw_unknown_opcode("host_dispatcher", opcode);
			}

			#undef OPCODE_HANDLER
		}

		template <typename Handler, size_t BufLen>
		static VstIntPtr send(Handler &handler, std::array<char, BufLen> &buf, VstInt32 opcode, const dispatcher_request &request) {
			#define OPCODE_HANDLER(MSG) case master::MSG::opcode: return send_message<master::MSG, dispatcher_response>(handler, buf, type_t::dispatcher, request).response;

			switch (opcode) {
				MSG_MASTER_MESSAGES(OPCODE_HANDLER)
				default: throw_unknown_opcode("host_dispatcher", opcode);
			}

			#undef OPCODE_HANDLER
		}

		template <typename Msg, typename Handler>
		static auto run(Handler &handler, const dispatcher_request &request) {
			return dispatcher_response{handler.dispatcher(Msg::opcode, request.index, request.value, request.ptr, request.opt)};
		}
	};

	// message directed at the effect's dispatcher
	struct effect_dispatcher {
		static constexpr type_t type = type_t::dispatcher;
	
		template <typename Handler, size_t BufLen>
		static void handle(Handler &handler, std::array<char, BufLen> &buf, size_t offset) {
			VstInt32 opcode;
			offset += io::read_data(buf.data(), offset, opcode);

			#define OPCODE_HANDLER(MSG) case effect::MSG::opcode: \
				process_message<effect::MSG, Handler, dispatcher_request, dispatcher_response, &run<effect::MSG, Handler>>(handler, buf, offset); \
				break;

			switch (opcode) {
				MSG_EFFECT_MESSAGES(OPCODE_HANDLER)
				default: throw_unknown_opcode("effect_dispatcher", opcode);
			}

			#undef OPCODE_HANDLER
		}

		template <typename Handler, size_t BufLen>
		static VstIntPtr send(Handler &handler, std::array<char, BufLen> &buf, VstInt32 opcode, const dispatcher_request &request) {
			#define OPCODE_HANDLER(MSG) case effect::MSG::opcode: return send_message<effect::MSG, dispatcher_response>(handler, buf, type_t::dispatcher, request).response;

			switch (opcode) {
				MSG_EFFECT_MESSAGES(OPCODE_HANDLER)
				default: throw_unknown_opcode("effect_dispatcher", opcode);
			}

			#undef OPCODE_HANDLER
		}

		template <typename Msg, typename Handler>
		static auto run(Handler &handler, const dispatcher_request &request) {
			return dispatcher_response{handler.dispatcher(Msg::opcode, request.index, request.value, request.ptr, request.opt)};
		}
	};
}

#endif
