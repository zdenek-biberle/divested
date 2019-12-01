#ifndef MSG_BASE_HPP
#define MSG_BASE_HPP

#include "pluginterfaces/vst2.x/aeffect.h"

namespace msg {
	template <VstInt32 op>
	struct msg {
		static constexpr VstInt32 opcode = op;
	};

	struct index {};
	template <typename T> constexpr bool has_index = std::is_base_of_v<index, T>;

	struct value {};
	template <typename T> constexpr bool has_value = std::is_base_of_v<value, T>;

	struct opt {};
	template <typename T> constexpr bool has_opt = std::is_base_of_v<opt, T>;

	// An array_out_ptr means that ptr is a pointer to an array of `len` objects
	// of type `T` and is used solely as an output. I.e. it is written to within
	// the dispatcher and is then read by the caller.
	struct array_out_ptr_tag {};

	template<typename T, size_t size>
	struct array_out_ptr : array_out_ptr_tag {
		using array_out_ptr_type = T;
		static constexpr size_t array_out_ptr_size = size;
	};

	template <typename T> constexpr bool has_array_out_ptr = std::is_base_of_v<array_out_ptr_tag, T>;

	// This is just a shortcut for array_out_ptr of chars.
	template <size_t size>
	struct str_out_ptr : public array_out_ptr<char, size> {};

	struct plain_ret {};
	template <typename T> constexpr bool has_plain_ret = std::is_base_of_v<plain_ret, T>;

	template <typename T>
	inline const char *to_buf(const T *buf) {
		return reinterpret_cast<const char *>(buf);
	}

	template <typename T>
	inline char *to_buf(T *buf) {
		return reinterpret_cast<char *>(buf);
	}

	template <typename T, typename Buf>
	inline size_t write_data(Buf *buf, size_t offset, const T& data) {
		::memcpy(to_buf(buf) + offset, &data, sizeof(T));
		return sizeof(T);
	}

	template <size_t size, typename T, typename Buf>
	inline size_t write_data_array(Buf *buf, size_t offset, const T* data) {
		size_t total_size = sizeof(T) * size;
		::memcpy(to_buf(buf) + offset, data, total_size);
		return total_size;
	}

	template <typename T, typename Buf>
	inline size_t read_data(const Buf *buf, size_t offset, T& data) {
		::memcpy(&data, to_buf(buf) + offset, sizeof(T));
		return sizeof(T);
	}

	template <size_t size, typename T, typename Buf>
	inline size_t read_data_array(Buf *buf, size_t offset, T* data) {
		size_t total_size = sizeof(T) * size;
		::memcpy(data, to_buf(buf) + offset, total_size);
		return total_size;
	}

	// TODO: alignment
	template <size_t size, typename T>
	inline size_t map_data_array(void *shm, size_t offset, T *&data) {
		size_t total_size = sizeof(T) * size;
		data = reinterpret_cast<T *>(to_buf(shm) + offset);
		return total_size;
	}

	/// Just allocate the shared memory - do not actually write anything into it.
	// TODO: alignment
	template <size_t size, typename T>
	inline size_t alloc_data_array() {
		return sizeof(T) * size;
	}

	template <typename T>
	size_t write_request(char *buf, VstInt32 index, VstIntPtr value, void *ptr, float opt) {
		size_t offset = 0;
		
		if constexpr (has_index<T>)
			offset += write_data(buf, offset, index);

		if constexpr (has_value<T>)
			offset += write_data(buf, offset, value);

		if constexpr (has_opt<T>)
			offset += write_data(buf, offset, opt);

		return offset;
	}

	template <typename T>
	size_t write_request_shm(void *shm, size_t start, void *ptr) {
		size_t offset = 0;

		// array_out_ptr just needs to allocate enough bytes in the shm
		if constexpr (has_array_out_ptr<T>)
			offset += alloc_data_array<T::array_out_ptr_size, typename T::array_out_ptr_type>();

		return offset;
	}

	template <typename T>
	size_t read_request(const char *buf, VstInt32 &index, VstIntPtr &value, void *&ptr, float &opt) {
		size_t offset = 0;
		
		if constexpr (has_index<T>)
			offset += read_data(buf, offset, index);

		if constexpr (has_value<T>)
			offset += read_data(buf, offset, value);

		if constexpr (has_opt<T>)
			offset += read_data(buf, offset, opt);

		return offset;
	}

	template <typename T>
	size_t read_request_shm(void *shm, size_t start, void *&ptr) {
		size_t offset = 0;

		// array_out_ptr will write directly to the shm, so just map ptr to it
		if constexpr (has_array_out_ptr<T>)
			offset += map_data_array<T::array_out_ptr_size>(
					shm, start + offset,
					reinterpret_cast<typename T::array_out_ptr_type *&>(ptr));

		return offset;
	}

	template <typename T>
	size_t write_response(char *buf, VstIntPtr response) {
		if constexpr (has_plain_ret<T>)
			return write_data(buf, 0, response);
		else
			return 0;
	}

	template <typename T>
	size_t write_response_shm(void *shm, size_t start, VstIntPtr response) {
		size_t offset = 0;

		// array_out_ptr doesn't need to write anything to the shm - just let be, but consume the space
		if constexpr (has_array_out_ptr<T>)
			offset += alloc_data_array<T::array_out_ptr_size, typename T::array_out_ptr_type>();

		return offset;
	}

	template <typename T>
	size_t read_response(const char *buf, VstIntPtr &response) {
		if constexpr (has_plain_ret<T>)
			return read_data(buf, 0, response);
		else
			return 0;
	}

	template <typename T>
	size_t read_response_shm(const void *shm, size_t start, VstIntPtr &response, void *ptr) {
		size_t offset = 0;

		// copy the output that's in the shm to the space pointed to by ptr
		if constexpr (has_array_out_ptr<T>)
			offset += read_data_array<T::array_out_ptr_size>(
					shm, start + offset,
					reinterpret_cast<typename T::array_out_ptr_type *>(ptr));

		return offset;
	}
	
}

#endif
