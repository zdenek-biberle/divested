#ifndef MSG_DISPATCHERS_HPP
#define MSG_DISPATCHERS_HPP

#include "common/msg/effect/messages.hpp"
#include "common/msg/io/buf.hpp"
#include "common/msg/io/msg.hpp"
#include "common/msg/master/messages.hpp"
#include "common/msg/base.hpp"
#include "common/msg/receiver.hpp"
#include "common/msg/sender.hpp"

namespace msg {
	template <typename T, typename Handler, size_t BufLen>
	static void receive_dispatcher_message(Handler &handler, std::array<char, BufLen> &buf, size_t offset) {
		VstInt32 index = 0;
		VstIntPtr value = 0;
		void* ptr = nullptr;
		float opt = 0.f;
		log::log() << "Receiving dispatcher message " << message_name<T> << std::endl;

		auto shm_offset = handler.shm_offset();

		io::buf pipe{buf.data(), offset};
		io::buf shm{handler.shm(), shm_offset};
		auto req_ctx = mk_payload_ctx(pipe, shm, handler);
		
		io::read_request<T>(req_ctx, index, value, ptr, opt);
		handler.shm_push(shm.total());

		log::log() << "Read dispatcher request, index: " << index << ", value: " << value << ", ptr: " << ptr << ", opt: " << opt << std::endl;
		if constexpr (has_payload_ptr<T>)
			T::payload::show_request(log::log() << "ptr content: ", ptr) << std::endl;

		log::log() << "Read " << pipe.offset << " B in total" << std::endl;
		log::log() << "Read " << shm.total() << " B of shm at offset " << shm_offset << std::endl;
		auto response = handler.dispatcher(T::opcode, index, value, ptr, opt);

		log::log() << "Dispatcher called, result: " << response << std::endl;
		if constexpr (has_payload_ptr<T>)
			T::payload::show_response(log::log() << "ptr content: ", ptr, response) << std::endl;

		// TODO: cleanup response data
		send_return_dispatcher<T>(handler, buf, shm_offset, shm.total(), ptr, response);
	}

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

			#define OPCODE_HANDLER(MSG) case master::MSG::opcode: receive_dispatcher_message<master::MSG>(handler, buf, offset); break;

			switch (opcode) {
				MSG_MASTER_MESSAGES(OPCODE_HANDLER)
				default: throw_unknown_opcode("host_dispatcher", opcode);
			}

			#undef OPCODE_HANDLER
		}

		template <typename Handler, size_t BufLen>
		static VstIntPtr send(Handler &handler, std::array<char, BufLen> &buf, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {
			#define OPCODE_HANDLER(MSG) case master::MSG::opcode: return send_dispatcher<master::MSG>(handler, buf, index, value, ptr, opt);

			switch (opcode) {
				MSG_MASTER_MESSAGES(OPCODE_HANDLER)
				default: throw_unknown_opcode("host_dispatcher", opcode);
			}

			#undef OPCODE_HANDLER
		}
	};

	// message directed at the effect's dispatcher
	struct effect_dispatcher {
		static constexpr type_t type = type_t::dispatcher;
	
		template <typename Handler, size_t BufLen>
		static void handle(Handler &handler, std::array<char, BufLen> &buf, size_t offset) {
			VstInt32 opcode;
			offset += io::read_data(buf.data(), offset, opcode);

			#define OPCODE_HANDLER(MSG) case effect::MSG::opcode: receive_dispatcher_message<effect::MSG>(handler, buf, offset); break;

			switch (opcode) {
				MSG_EFFECT_MESSAGES(OPCODE_HANDLER)
				default: throw_unknown_opcode("effect_dispatcher", opcode);
			}

			#undef OPCODE_HANDLER
		}

		template <typename Handler, size_t BufLen>
		static VstIntPtr send(Handler &handler, std::array<char, BufLen> &buf, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {
			#define OPCODE_HANDLER(MSG) case effect::MSG::opcode: return send_dispatcher<effect::MSG>(handler, buf, index, value, ptr, opt);

			switch (opcode) {
				MSG_EFFECT_MESSAGES(OPCODE_HANDLER)
				default: throw_unknown_opcode("effect_dispatcher", opcode);
			}

			#undef OPCODE_HANDLER
		}
	};
}

#endif
