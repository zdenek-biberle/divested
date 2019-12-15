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
		log::log() << "Sending plain return" << std::endl;
		offset += io::write_data(buf.data(), offset, type_t::return_);
		handler.message_write(buf.data(), offset);
	}

	template <typename Handler, size_t BufLen, typename Payload>
	void send_return_payload(Handler &handler, std::array<char, BufLen> &buf, const Payload &payload) {
		size_t offset = 0;
		log::log() << "Sending return with payload of size " << sizeof(Payload) << std::endl;
		offset += io::write_data(buf.data(), offset, type_t::return_);
		offset += io::write_data(buf.data(), offset, payload);
		handler.message_write(buf.data(), offset);
	}

	template <typename Msg, typename Handler, size_t BufLen>
	void send_return_dispatcher(Handler &handler, std::array<char, BufLen> &buf, size_t shm_offset, size_t shm_size, void *ptr, VstIntPtr response) {
		log::log() << "Sending return for dispatcher message " << message_name<Msg> << std::endl;

		io::buf pipe{buf.data(), 0};
		io::buf shm{handler.shm(), shm_offset};
		auto resp_ctx = mk_payload_ctx(pipe, shm, handler);

		pipe.write_data(type_t::return_);
		io::write_response<Msg>(resp_ctx, ptr, response);

		if (shm.total() || shm.total() != shm_size)
			log::log() << "write_response wrote " << shm.total() << " B of shm (compared to " << shm_size << " B)" << std::endl;

		handler.shm_pop(shm_size);
		handler.message_write(buf.data(), pipe.total());	
	}

	template <typename Msg, typename Handler, size_t BufLen>
	VstIntPtr send_dispatcher(Handler &handler, std::array<char, BufLen> &buf, VstInt32 index, VstIntPtr value, void *ptr, float opt) {
		log::log() << "Sending dispatcher call for message " << message_name<Msg> << std::endl;
		log::log() << "index: " << index << ", value: " << value << ", ptr: " << ptr << ", opt: " << opt << std::endl;

		size_t shm_offset = handler.shm_offset();

		io::buf req_pipe{buf.data(), 0};
		io::buf req_shm{handler.shm(), shm_offset};
		auto req_ctx = mk_payload_ctx(req_pipe, req_shm, handler);

		req_pipe.write_data(type_t::dispatcher);
		req_pipe.write_data(Msg::opcode);
		io::write_request<Msg>(req_ctx, index, value, ptr, opt);

		handler.shm_push(req_shm.total());
		handler.message_write(buf.data(), req_pipe.total());

		auto resp_offset = receive_message(handler, buf);

		io::buf resp_pipe{buf.data(), resp_offset};
		io::buf resp_shm{handler.shm(), shm_offset};
		auto resp_ctx = mk_payload_ctx(resp_pipe, resp_shm, handler);

		VstIntPtr response = 0;
		io::read_response<Msg>(resp_ctx, ptr, response);

		if (req_shm.total() != resp_shm.total())
			log::log() << "Written shm size (" << req_shm.total() << " B) does not equal read shm size (" << resp_shm.total() << " B)" << std::endl;

		handler.shm_pop(req_shm.total());

		return response;
	}

	template <typename Handler, size_t BufLen>
	VstIntPtr send_dispatcher(Handler &handler, std::array<char, BufLen> &buf, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt) {
		return Handler::message_configuration::dispatcher_sent::send(handler, buf, opcode, index, value, ptr, opt);
	}
}

#endif
