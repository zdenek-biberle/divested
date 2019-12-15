#ifndef MSG_IO_MSG_HPP
#define MSG_IO_MSG_HPP

namespace msg::io {
	template <typename T, typename Pipe, typename Shm>
	void write_request(Pipe &pipe, Shm &shm, VstInt32 index, VstIntPtr value, void *ptr, float opt) {
		if constexpr (has_index<T>)
			pipe.write_data(index);

		if constexpr (has_value<T>)
			pipe.write_data(value);

		if constexpr (has_payload_ptr<T>)
			T::payload::write_request(pipe, shm, ptr);

		if constexpr (has_opt<T>)
			pipe.write_data(opt);
	}

	template <typename T, typename Pipe, typename Shm>
	void read_request(Pipe &pipe, Shm &shm, VstInt32 &index, VstIntPtr &value, void *&ptr, float &opt) {
		if constexpr (has_index<T>)
			pipe.read_data(index);

		if constexpr (has_value<T>)
			pipe.read_data(value);

		if constexpr (has_payload_ptr<T>)
			T::payload::read_request(pipe, shm, ptr);

		if constexpr (has_opt<T>)
			pipe.read_data(opt);
	}

	template <typename T, typename Pipe, typename Shm>
	void write_response(Pipe &pipe, Shm &shm, void *ptr, VstIntPtr response) {
		if constexpr (has_payload_ptr<T>)
			T::payload::write_response(pipe, shm, ptr);

		if constexpr (has_plain_ret<T>)
			pipe.write_data(response);
	}

	template <typename T, typename Pipe, typename Shm>
	void read_response(Pipe &pipe, Shm &shm, void *ptr, VstIntPtr &response) {
		if constexpr (has_payload_ptr<T>)
			T::payload::read_response(pipe, shm, ptr);

		if constexpr (has_plain_ret<T>)
			pipe.read_data(response);
	}
}

#endif
