#ifndef MSG_SERIALIZERS_HPP
#define MSG_SERIALIZERS_HPP

#include <cstring>

#include "common/log/log.hpp"

namespace msg {
	template <typename T, typename Sub>
	struct casted_ptr {
		template <typename Pipe, typename Shm>
		static void write_request(Pipe &pipe, Shm &shm, void *ptr) {
			Sub::write_request(pipe, shm, reinterpret_cast<T *>(ptr));
		}

		template <typename Pipe, typename Shm>
		static void read_request(Pipe &pipe, Shm &shm, void *&ptr) {
			Sub::read_request(pipe, shm, *reinterpret_cast<T **>(&ptr));
		}

		template <typename Pipe, typename Shm>
		static void write_response(Pipe &pipe, Shm &shm, void *ptr) {
			Sub::write_response(pipe, shm, reinterpret_cast<T *>(ptr));
		}

		template <typename Pipe, typename Shm>
		static void read_response(Pipe &pipe, Shm &shm, void *ptr) {
			Sub::read_response(pipe, shm, reinterpret_cast<T *>(ptr));
		}

		static inline std::ostream &show_request(std::ostream &os, void *ptr) {
			return Sub::show_request(os, reinterpret_cast<T *>(ptr));
		}

		static inline std::ostream &show_response(std::ostream &os, void *ptr) {
			return Sub::show_response(os, reinterpret_cast<T *>(ptr));
		}
	};

	/// A C-style string that works as an input to the dispatcher - it is stored
	/// in shared memory on the way in and that's it.
	/// TODO: I don't like all the strlen calls, perhaps store the length in shm
	/// as well?
	struct in_str {
		template <typename Pipe, typename Shm>
		static void write_request(Pipe &pipe, Shm &shm, char *ptr) {
			shm.write_data_array(ptr, ::strlen(ptr));
		}

		template <typename Pipe, typename Shm>
		static void read_request(Pipe &pipe, Shm &shm, char *&ptr) {
			char *tmp_ptr;
			shm.map_data_array(tmp_ptr, 0);
			size_t len = ::strlen(tmp_ptr);
			shm.map_data_array(ptr, len);
		}

		template <typename Pipe, typename Shm>
		static void write_response(Pipe &pipe, Shm &shm, char *ptr) {
			shm.template skip_data_array<char>(::strlen(ptr));
		}

		template <typename Pipe, typename Shm>
		static void read_response(Pipe &pipe, Shm &shm, char *ptr) {
			shm.template skip_data_array<char>(::strlen(ptr));
		}

		static inline std::ostream &show_request(std::ostream &os, char *ptr) {
			return os << "\"" << ptr << "\"";
		}

		static inline std::ostream &show_response(std::ostream &os, char *ptr) {
			return os << "N/A";
		}
	};

	/// An array of items that serves as an output from the dispatcher. It is
	/// allocated within shm, filled within the dispatcher and then copied
	/// to its expected location.
	template <typename T, size_t Size, typename Show>
	struct out_array {
		template <typename Pipe, typename Shm>
		static void write_request(Pipe &pipe, Shm &shm, T *ptr) {
			shm.template skip_data_array<T>(Size);
		}

		template <typename Pipe, typename Shm>
		static void read_request(Pipe &pipe, Shm &shm, T *&ptr) {
			shm.map_data_array(ptr, Size);
		}

		template <typename Pipe, typename Shm>
		static void write_response(Pipe &pipe, Shm &shm, T *ptr) {
			shm.template skip_data_array<T>(Size);
		}

		template <typename Pipe, typename Shm>
		static void read_response(Pipe &pipe, Shm &shm, T *ptr) {
			shm.read_data_array(ptr, Size);
		}

		static inline std::ostream &show_request(std::ostream &os, char *ptr) {
			return os << "N/A";
		}

		static inline std::ostream &show_response(std::ostream &os, char *ptr) {
			return Show::show(os, ptr);
		}
	};

	/// Shows the contens of out_array as a string.
	struct out_array_show_str {
		static inline std::ostream &show(std::ostream &os, char *ptr) {
			return os << "\"" << ptr << "\"";
		}
	};
}

#endif
