#ifndef MSG_PROCESSOR_HPP
#define MSG_PROCESSOR_HPP

#include "common/msg/io/msg.hpp"
#include "common/msg/message_name.hpp"
#include "common/msg/sender.hpp"

namespace msg {
	/// Receives a message, runs the worker to process it and sends back the result.
	template <typename T, typename Worker, typename Request, typename Handler, size_t BufLen>
	void process_message(Handler &handler, std::array<char, BufLen> &buf, size_t offset) {
		Request request;
		log::log() << "Receiving message " << message_name<T> << std::endl;

		auto shm_offset = handler.shm_offset();

		io::buf pipe{buf.data(), offset};
		io::buf shm{handler.shm(), shm_offset};
		payload_ctx req_ctx{pipe, shm, handler};
		
		io::read_request<T>(req_ctx, request);
		handler.shm_push(shm.total());

		log::log() << "Read request, " << request << std::endl;
		if constexpr (has_payload_ptr<T>)
			T::payload::show_request(log::log() << "ptr content: ", request.ptr) << std::endl;

		log::log() << "Read " << pipe.offset << " B of pipe data and " << shm.total() << " B of shm at offset " << shm_offset << std::endl;
		auto response = Worker::template run<T>(handler, request);

		log::log() << "Dispatcher called, result: " << response << std::endl;
		if constexpr (has_payload_ptr<T>)
			T::payload::show_response(log::log() << "ptr content: ", request.ptr, response) << std::endl;

		// TODO: cleanup response data
		send_return_dispatcher<T>(handler, buf, shm_offset, shm.total(), request, response);
	}
}

#endif
