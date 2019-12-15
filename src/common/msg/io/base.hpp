#ifndef MSG_IO_BASE_HPP
#define MSG_IO_BASE_HPP

#include "common/msg/base.hpp"

namespace msg::io {
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

	template <typename T, typename Buf>
	inline size_t write_data_array(Buf *buf, size_t offset, const T* data, size_t size) {
		size_t total_size = sizeof(T) * size;
		::memcpy(to_buf(buf) + offset, data, total_size);
		return total_size;
	}

	template <size_t size, typename T, typename Buf>
	inline size_t write_data_array(Buf *buf, size_t offset, const T* data) {
		return write_data_array(buf, offset, data, size);
	}

	template <typename T, typename Buf>
	inline size_t read_data(const Buf *buf, size_t offset, T& data) {
		::memcpy(&data, to_buf(buf) + offset, sizeof(T));
		return sizeof(T);
	}

	template <typename T, typename Buf>
	inline size_t read_data_array(Buf *buf, size_t offset, T* data, size_t size) {
		size_t total_size = sizeof(T) * size;
		::memcpy(data, to_buf(buf) + offset, total_size);
		return total_size;
	}

	/// This takes an offset into a buffer and bumps it a bit so that it is
	/// aligned to the size of T.
	/// This ignores the actual buffer, which is fine because we only ever
	/// use this for aligning offsets into shm, which itself is aligned
	/// enough.
	template <typename T>
	inline size_t align_offset(size_t offset) {
		size_t alignment = alignof(T);
		return (offset + alignment - 1) & ~(alignment - 1);
	}

	template <typename T>
	inline size_t map_data_array(void *shm, size_t offset, T *&data, size_t size) {
		auto aligned_offset = align_offset<T>(offset);
		size_t total_size = sizeof(T) * size;
		data = reinterpret_cast<T *>(to_buf(shm) + aligned_offset);
		return total_size + aligned_offset - offset;
	}

	/// Just skip memory - do not actually write anything into it.
	template <typename T>
	inline size_t skip_data_array(size_t offset, size_t size) {
		return sizeof(T) * size + align_offset<T>(offset) - offset;
	}
}

#endif
