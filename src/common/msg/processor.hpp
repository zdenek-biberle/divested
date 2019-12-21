#ifndef MSG_PROCESSOR_HPP
#define MSG_PROCESSOR_HPP

#include <ostream>

#include "common/msg/io/msg.hpp"
#include "common/msg/message_name.hpp"
#include "common/msg/sender.hpp"

namespace msg {
	template <typename Payload, typename T>
	struct request_writer_t {
		const T &data;

		friend std::ostream &operator<<(std::ostream &os, const request_writer_t &writer) {
			return Payload::show_request(os, writer.data);
		}
	};

	template <typename Payload, typename T>
	request_writer_t<Payload, T> request_writer(const T& data) {
		return {data};
	}

	template <typename Payload, typename T, typename U>
	struct response_writer_t {
		const T &data;
		const U &response;

		friend std::ostream &operator<<(std::ostream &os, const response_writer_t &writer) {
			return Payload::show_response(os, writer.data, writer.response);
		}
	};

	template <typename Payload, typename T, typename U>
	response_writer_t<Payload, T, U> response_writer(const T& data, const U& response) {
		return {data, response};
	}

	/// Receives a message, runs the worker to process it and sends back the result.
	template <typename T, typename Handler, typename Request, typename Response, Response (*F)(Handler&, const Request&), size_t BufLen>
	void process_message(Handler &handler, std::array<char, BufLen> &buf, size_t offset) {
		Request request;
		LOG_TRACE("Receiving message " << message_name<T>);

		auto shm_offset = handler.shm_offset();

		io::buf pipe{buf.data(), offset};
		io::buf shm{handler.shm(), shm_offset};
		payload_ctx req_ctx{pipe, shm, handler};
		
		io::read_request<T>(req_ctx, request);
		handler.shm_push(shm.total());

		LOG_TRACE("Read request, " << request);
		if constexpr (has_payload_ptr<T>)
			LOG_TRACE("ptr content: " << request_writer<typename T::payload>(request.ptr))

		LOG_TRACE("Read " << pipe.offset << " B of pipe data and " << shm.total() << " B of shm at offset " << shm_offset);
		auto response = F(handler, request);

		LOG_TRACE("Dispatcher called, result: " << response);
		if constexpr (has_payload_ptr<T>)
			LOG_TRACE("ptr content: " << response_writer<typename T::payload>(request.ptr, response));

		// TODO: cleanup response data
		send_return_dispatcher<T>(handler, buf, shm_offset, shm.total(), request, response);
	}
}

#endif
