#ifndef MSG_IO_MSG_HPP
#define MSG_IO_MSG_HPP

#include "common/msg/io/audio.hpp"

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

		if constexpr (has_num_inputs<T>)
			ctx.pipe.write_data(body.num_inputs);

		if constexpr (has_num_outputs<T>)
			ctx.pipe.write_data(body.num_outputs);

		if constexpr (has_inputs<T>)
			write_audio_arrays(ctx, body.index, body.num_inputs, body.inputs);

		if constexpr (has_accumulating_outputs<T>)
			write_audio_arrays(ctx, body.index, body.num_outputs, body.outputs);
		else if constexpr (has_replacing_outputs<T>)
			skip_audio_arrays(ctx, body.index, body.num_outputs, body.outputs);
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

		if constexpr (has_num_inputs<T>)
			ctx.pipe.read_data(request.num_inputs);

		if constexpr (has_num_outputs<T>)
			ctx.pipe.read_data(request.num_outputs);

		if constexpr (has_inputs<T>)
			map_audio_arrays(ctx, request.index, request.num_inputs, request.inputs);

		if constexpr (has_outputs<T>)
			map_audio_arrays(ctx, request.index, request.num_outputs, request.outputs);
	}

	template <typename T, typename Request, typename Response, typename Ctx>
	void write_response(const Ctx &ctx, Request &request, const Response &response) {
		if constexpr (has_plain_ret<T>)
			ctx.pipe.write_data(response);

		if constexpr (has_payload_ptr<T>)
			T::payload::write_response(ctx, request.ptr, response);

		if constexpr (has_payload_ret<T>)
			T::return_payload::write_response(ctx, response.response, response);

		if constexpr (has_inputs<T>)
			skip_audio_arrays(ctx, request.index, request.num_inputs, request.inputs);

		if constexpr (has_outputs<T>)
			skip_audio_arrays(ctx, request.index, request.num_outputs, request.outputs);
	}

	template <typename T, typename Request, typename Response, typename Ctx>
	void read_response(const Ctx &ctx, const Request &request, Response &response) {
		if constexpr (has_plain_ret<T>)
			ctx.pipe.read_data(response);

		if constexpr (has_payload_ptr<T>)
			T::payload::read_response(ctx, request.ptr, response);

		if constexpr (has_payload_ret<T>)
			T::return_payload::read_response(ctx, response.response, response);

		if constexpr (has_inputs<T>)
			skip_audio_arrays(ctx, request.index, request.num_inputs, request.inputs);

		if constexpr (has_outputs<T>)
			read_audio_arrays(ctx, request.index, request.num_outputs, request.outputs);
	}
}

#endif
