#ifndef MSG_SENDER_HPP
#define MSG_SENDER_HPP

#include "common/msg/base.hpp"
#include "common/msg/message_name.hpp"
#include "common/msg/receiver.hpp"
#include "common/msg/type.hpp"

namespace msg {
	template <typename Handler, size_t BufLen>
	void send_return(Handler &handler, std::array<char, BufLen> &buf) {
		size_t offset = 0;
		log::log() << "Sending plain return" << std::endl;
		offset += write_data(buf.data(), offset, type_t::return_);
		handler.message_write(buf.data(), offset);
	}

	template <typename Handler, size_t BufLen, typename Payload>
	void send_return_payload(Handler &handler, std::array<char, BufLen> &buf, const Payload &payload) {
		size_t offset = 0;
		log::log() << "Sending return with payload of size " << sizeof(Payload) << std::endl;
		offset += write_data(buf.data(), offset, type_t::return_);
		offset += write_data(buf.data(), offset, payload);
		handler.message_write(buf.data(), offset);
	}

	template <typename Msg, typename Handler, size_t BufLen>
	void send_return_dispatcher(Handler &handler, std::array<char, BufLen> &buf, size_t shm_offset, VstIntPtr response) {
		size_t offset = 0;
		log::log() << "Sending return for dispatcher message " << message_name<Msg> << std::endl;
		offset += write_data(buf.data(), offset, type_t::return_);
		offset += write_response<Msg>(buf.data() + offset, response);
		size_t shm_size = write_response_shm<Msg>(handler.shm(), shm_offset, response);
		handler.shm_pop(shm_size);
		handler.message_write(buf.data(), offset);	
	}

	template <typename Msg, typename Handler, size_t BufLen>
	VstIntPtr send_dispatcher(Handler &handler, std::array<char, BufLen> &buf, VstInt32 index, VstIntPtr value, void *ptr, float opt) {
		size_t offset = 0;
		log::log() << "Sending dispatcher call for message " << message_name<Msg> << std::endl;
		log::log() << "index: " << index << ", value: " << value << ", ptr: " << ptr << ", opt: " << opt << std::endl;
		offset += write_data(buf.data(), offset, type_t::dispatcher);
		offset += write_data(buf.data(), offset, Msg::opcode);
		offset += write_request<Msg>(buf.data() + offset, index, value, ptr, opt);
		size_t shm_offset = handler.shm_offset();
		size_t written_shm_size = write_request_shm<Msg>(handler.shm(), shm_offset, ptr);
		handler.shm_push(written_shm_size);
		handler.message_write(buf.data(), offset);

		offset = 0;
		offset += receive_message(handler, buf);
		VstIntPtr response = 0;
		offset += read_response<Msg>(buf.data() + offset, response);
		size_t read_shm_size = read_response_shm<Msg>(handler.shm(), shm_offset, response, ptr);

		if (written_shm_size != read_shm_size) {
			std::stringstream ss;
			ss << "Written shm size (" << written_shm_size << " B) does not equal read shm size (" << read_shm_size << " B)";
			throw std::runtime_error(ss.str());
		}

		handler.shm_pop(read_shm_size);

		return response;
	}

	template <typename Handler, size_t BufLen>
	VstIntPtr send_dispatcher(Handler &handler, std::array<char, BufLen> &buf, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt) {
		return Handler::message_configuration::dispatcher_sent::send(handler, buf, opcode, index, value, ptr, opt);
	}
}

#endif
