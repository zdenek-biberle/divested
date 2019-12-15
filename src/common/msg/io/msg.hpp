#ifndef MSG_IO_MSG_HPP
#define MSG_IO_MSG_HPP

namespace msg::io {
	template <typename T, typename Ctx>
	void write_request(const Ctx &ctx, VstInt32 index, VstIntPtr value, void *ptr, float opt) {
		if constexpr (has_index<T>)
			ctx.pipe.write_data(index);

		if constexpr (has_value<T>)
			ctx.pipe.write_data(value);

		if constexpr (has_payload_ptr<T>)
			T::payload::write_request(ctx, ptr);

		if constexpr (has_opt<T>)
			ctx.pipe.write_data(opt);
	}

	template <typename T, typename Ctx>
	void read_request(const Ctx &ctx, VstInt32 &index, VstIntPtr &value, void *&ptr, float &opt) {
		if constexpr (has_index<T>)
			ctx.pipe.read_data(index);

		if constexpr (has_value<T>)
			ctx.pipe.read_data(value);

		if constexpr (has_payload_ptr<T>)
			T::payload::read_request(ctx, ptr);

		if constexpr (has_opt<T>)
			ctx.pipe.read_data(opt);
	}

	template <typename T, typename Ctx>
	void write_response(const Ctx &ctx, void *ptr, VstIntPtr response) {
		if constexpr (has_plain_ret<T>)
			ctx.pipe.write_data(response);

		if constexpr (has_payload_ptr<T>)
			T::payload::write_response(ctx, ptr, response);
	}

	template <typename T, typename Ctx>
	void read_response(const Ctx &ctx, void *ptr, VstIntPtr &response) {
		if constexpr (has_plain_ret<T>)
			ctx.pipe.read_data(response);

		if constexpr (has_payload_ptr<T>)
			T::payload::read_response(ctx, ptr, response);
	}
}

#endif
