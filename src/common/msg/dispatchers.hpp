#ifndef MSG_DISPATCHERS_HPP
#define MSG_DISPATCHERS_HPP

#include "common/msg/effect/messages.hpp"
#include "common/msg/master/messages.hpp"
#include "common/msg/base.hpp"
#include "common/msg/receiver.hpp"
#include "common/msg/sender.hpp"

namespace msg {
	template <typename T, typename Handler, size_t BufLen>
	static void receive_dispatcher_message(Handler &handler, std::array<char, BufLen> &buf, size_t offset) {
		VstInt32 index;
		VstIntPtr value;
		void* ptr;
		float opt;
		log::log() << "Receiving dispatcher message " << message_name<T> << std::endl;
		offset += read_request<T>(buf.data() + offset, index, value, ptr, opt);
		auto shm_offset = handler.shm_offset();
		size_t shm_size = read_request_shm<T>(handler.shm(), shm_offset, ptr);
		handler.shm_push(shm_size);
		log::log() << "Read " << offset << " B in total" << std::endl;
		log::log() << "Read " << shm_size << " B of shm at offset " << shm_offset << std::endl;
		auto response = handler.dispatcher(T::opcode, index, value, ptr, opt);
		// TODO: cleanup response data
		send_return_dispatcher<T>(handler, buf, shm_offset, response);
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
			offset += read_data(buf.data(), offset, opcode);

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
			offset += read_data(buf.data(), offset, opcode);

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
