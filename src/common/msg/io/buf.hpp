#ifndef MSG_IO_BUF_HPP
#define MSG_IO_BUF_HPP

#include "common/log/log.hpp"
#include "common/msg/io/base.hpp"

namespace msg::io {
	struct buf {
		char *memory;
		size_t initial_offset;
		size_t offset;

		inline buf(char *memory, size_t offset):
			memory(memory), initial_offset(offset), offset(offset)
		{}

		template <typename T>
		void write_data(const T& data) {
			offset += ::msg::io::write_data(memory, offset, data);
		}

		template <typename T>
		void write_data_array(const T* array, size_t size) {
			offset += ::msg::io::write_data_array(memory, offset, array, size);
		}

		template <typename T>
		void skip_data_array(size_t size) {
			offset += ::msg::io::skip_data_array<T>(offset, size);
		}

		template <typename T>
		void skip_data() {
			skip_data_array<T>(1);
		}

		template <typename T>
		void map_data(T *&data) {
			offset += ::msg::io::map_data_array(memory, offset, data, 1);
		}

		template <typename T>
		void map_data_array(T *&data, size_t size) {
			offset += ::msg::io::map_data_array(memory, offset, data, size);
		}

		template <typename T>
		void read_data(T &data) {
			offset += ::msg::io::read_data(memory, offset, data);
		}

		template <typename T>
		void read_data_array(T *data, size_t size) {
			offset += ::msg::io::read_data_array(memory, offset, data, size);
		}
	
		/// Returns the total number of bytes written/read after this
		/// buf has been created.
		inline size_t total() {
			return offset - initial_offset;
		}
	};
}

#endif
