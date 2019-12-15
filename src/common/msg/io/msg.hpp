#ifndef MSG_IO_MSG_HPP
#define MSG_IO_MSG_HPP

namespace msg::io {
	template <typename T, typename Ctx, typename Body>
	void write_request(const Ctx &ctx, const Body &body) {
		if constexpr (has_opcode<T>)
			ctx.pipe.write_data(T::opcode);

		if constexpr (has_index<T>)
			ctx.pipe.write_data(body.index);

		if constexpr (has_value<T>)
			ctx.pipe.write_data(body.value);

		if constexpr (has_payload_ptr<T>)
			T::payload::write_request(ctx, body.ptr);

		if constexpr (has_opt<T>)
			ctx.pipe.write_data(body.opt);
	}

	template <typename T, typename Ctx, typename Request>
	void read_request(const Ctx &ctx, Request &request) {
		if constexpr (has_index<T>)
			ctx.pipe.read_data(request.index);

		if constexpr (has_value<T>)
			ctx.pipe.read_data(request.value);

		if constexpr (has_payload_ptr<T>)
			T::payload::read_request(ctx, request.ptr);

		if constexpr (has_opt<T>)
			ctx.pipe.read_data(request.opt);
	}

	template <typename T, typename Request, typename Response, typename Ctx>
	void write_response(const Ctx &ctx, Request &request, const Response &response) {
		if constexpr (has_plain_ret<T>)
			ctx.pipe.write_data(response);

		if constexpr (has_payload_ptr<T>)
			T::payload::write_response(ctx, request.ptr, response);
	}

	template <typename T, typename Request, typename Response, typename Ctx>
	void read_response(const Ctx &ctx, const Request &request, Response &response) {
		if constexpr (has_plain_ret<T>)
			ctx.pipe.read_data(response);

		if constexpr (has_payload_ptr<T>)
			T::payload::read_response(ctx, request.ptr, response);
	}
}

#endif
