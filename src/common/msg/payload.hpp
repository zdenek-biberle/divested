#ifndef MSG_SERIALIZERS_HPP
#define MSG_SERIALIZERS_HPP

#include <cstring>

#include "common/msg/type.hpp"
#include "common/util/str_writer.hpp"
#include "pluginterfaces/vst2.x/aeffect.h"

namespace msg {
	template <typename Pipe, typename Shm, typename Allocator>
	struct payload_ctx {
		Pipe &pipe;
		Shm &shm;
		Allocator &allocator;
	};

	template <typename Pipe, typename Shm, typename Allocator>
	payload_ctx(Pipe &, Shm &, Allocator &) -> payload_ctx<Pipe, Shm, Allocator>;

	template <typename T, typename Sub>
	struct casted_ptr {
		template <typename Ctx>
		static void write_request(const Ctx &ctx, void *ptr) {
			Sub::write_request(ctx, reinterpret_cast<T *>(ptr));
		}

		template <typename Ctx>
		static void read_request(const Ctx &ctx, void *&ptr) {
			Sub::read_request(ctx, *reinterpret_cast<T **>(&ptr));
		}

		template <typename Ctx, typename Response>
		static void write_response(const Ctx &ctx, void *ptr, const Response &response) {
			Sub::write_response(ctx, reinterpret_cast<T *>(ptr), response);
		}

		template <typename Ctx, typename Response>
		static void read_response(const Ctx &ctx, void *ptr, const Response &response) {
			Sub::read_response(ctx, reinterpret_cast<T *>(ptr), response);
		}

		static inline std::ostream &show_request(std::ostream &os, void *ptr) {
			return Sub::show_request(os, reinterpret_cast<T *>(ptr));
		}

		template <typename Response>
		static std::ostream &show_response(std::ostream &os, void *ptr, const Response &response) {
			return Sub::show_response(os, reinterpret_cast<T *>(ptr), response);
		}
	};

	/// Using sized_in_array to transmit C-style strings.
	struct sized_in_array_str {
		static inline int size(const char *ptr) {
			return ::strlen(ptr) + 1;
		}

		static inline std::ostream &show(std::ostream &os, const char *ptr) {
			return os << "\"" << ptr << "\"";
		}
	};

	/// An array of elements with a dynamic size that works as an input to the
	/// dispatcher - it is stored in shared memory on the way in and then mapped.
	/// The pointer can be null as well.
	template <typename T, typename Spec>
	struct sized_in_array {
		template <typename Ctx>
		static void write_request(const Ctx &ctx, char *ptr) {
			if (ptr) {
				int size = Spec::size(ptr);
				ctx.shm.write_data(size);
				ctx.shm.write_data_array(ptr, size);
			} else {
				ctx.shm.template write_data<int>(~0);
			}
		}

		template <typename Ctx>
		static void read_request(const Ctx &ctx, char *&ptr) {
			int size;
			ctx.shm.read_data(size);

			if (size == ~0) {
				ptr = nullptr;
			} else {
				ctx.shm.map_data_array(ptr, size);
			}
		}

		template <typename Ctx, typename Response>
		static void write_response(const Ctx &ctx, char *ptr, const Response &response) {
			int size;
			ctx.shm.read_data(size);
			if (size != ~0)
				ctx.shm.template skip_data_array<char>(size);
		}

		template <typename Ctx, typename Response>
		static void read_response(const Ctx &ctx, char *ptr, const Response &response) {
			int size;
			ctx.shm.read_data(size);
			if (size != ~0)
				ctx.shm.template skip_data_array<char>(::strlen(ptr) + 1);
		}

		static inline std::ostream &show_request(std::ostream &os, char *ptr) {
			if (ptr)
				return Spec::show(os, ptr);
			else
				return os << "nullptr";
		}

		template <typename Response>
		static std::ostream &show_response(std::ostream &os, char *ptr, const Response &response) {
			return os << "N/A";
		}
	};

	/// Shows the contents of a pointer to chars as a C string.
	struct show_str {
		static inline std::ostream &show_request(std::ostream &os, char *ptr, size_t size) {
			return os << util::str_writer{ptr, size};
		}

		template <typename Response>
		static std::ostream &show_response(std::ostream &os, char *ptr, size_t size, const Response &response) {
			return os << util::str_writer{ptr, size};
		}
	};

	/// Showing the contents of a pointer directly.
	struct show_direct {
		template <typename T>
		static std::ostream &show_request(std::ostream &os, T *ptr, size_t size) {
			return os << *ptr;
		}

		template <typename T, typename Response>
		static std::ostream &show_response(std::ostream &os, T *ptr, size_t size, const Response &response) {
			return os << *ptr;
		}
	};

	/// Possible request/response behaviours of shm_array
	struct shm_array_behaviour {
		// Data will be copied to the shm when sending the request
		template <typename T, size_t Size, typename Show>
		struct request_write {
			template <typename Ctx>
			static void write_request(const Ctx &ctx, T *ptr) {
				ctx.shm.write_data_array(ptr, Size);
			}

			static inline std::ostream &show_request(std::ostream &os, T *ptr) {
				return Show::show_request(os, ptr, Size);
			}
		};

		// Data won't be copied to the shm when sending the request
		template <typename T, size_t Size, typename Show>
		struct request_skip {
			template <typename Ctx>
			static void write_request(const Ctx &ctx, T *ptr) {
				ctx.shm.template skip_data_array<T>(Size);
			}

			static inline std::ostream &show_request(std::ostream &os, T *ptr) {
				return os << "N/A";
			}
		};

		// Data will be read from the shm when receiving the response
		template <typename T, size_t Size, typename Show>
		struct response_read {
			template <typename Ctx, typename Response>
			static void read_response(const Ctx &ctx, T *ptr, const Response &response) {
				ctx.shm.template skip_data_array<T>(Size);
			}

			template <typename Response>
			static inline std::ostream &show_response(std::ostream &os, T *ptr, const Response &response) {
				return os << "N/A";
			}
		};

		// Data won't be read from the shm when receiving the response
		template <typename T, size_t Size, typename Show>
		struct response_skip {
			template <typename Ctx, typename Response>
			static void read_response(const Ctx &ctx, T *ptr, const Response &response) {
				ctx.shm.read_data_array(ptr, Size);
			}

			template <typename Response>
			static inline std::ostream &show_response(std::ostream &os, T *ptr, const Response &response) {
				return Show::show_response(os, ptr, Size, response);
			}
		};
	};

	/// An array of elements that's mapped to the shm. Used either as input,
	/// output or both.
	template <
		typename T,
		size_t Size,
		typename Show,
		template <typename, size_t, typename> typename RequestBehaviour,
		template <typename, size_t, typename> typename ResponseBehaviour
	>
	struct shm_array : RequestBehaviour<T, Size, Show>, ResponseBehaviour<T, Size, Show> {
		template <typename Ctx>
		static void read_request(const Ctx &ctx, T *&ptr) {
			ctx.shm.map_data_array(ptr, Size);
		}

		template <typename Ctx, typename Response>
		static void write_response(const Ctx &ctx, T *ptr, const Response &response) {
			ctx.shm.template skip_data_array<T>(Size);
		}
	};

	template <typename T, size_t Size, typename Show> using shm_array_in = shm_array<T, Size, Show, shm_array_behaviour::request_write, shm_array_behaviour::response_skip>;
	template <typename T, size_t Size, typename Show> using shm_array_out = shm_array<T, Size, Show, shm_array_behaviour::request_skip, shm_array_behaviour::response_read>;
	template <typename T, size_t Size, typename Show> using shm_array_inout = shm_array<T, Size, Show, shm_array_behaviour::request_write, shm_array_behaviour::response_read>;

	template <typename T> using shm_in_1 = shm_array_in<T, 1, show_direct>;
	template <typename T> using shm_out_1 = shm_array_out<T, 1, show_direct>;
	template <typename T> using shm_inout_1 = shm_array_inout<T, 1, show_direct>;


	/// This is the output of getChunk. A pointer is allocated within the shm,
	/// a pointer to this pointer is passed to the plugin and that plugin then
	/// modifies the pointer. When writing the response, the pointer is 
	/// replaced by its contents.
	///
	/// The contens will obviously be larger than the pointer, but that's okay.
	struct chunk_out {
		template <typename Ctx>
		static void write_request(const Ctx &ctx, char **ptr) {
			// ignore the value of ptr, just write a single nullptr to the
			// shared memory
			ctx.shm.template write_data<char *>(nullptr);
		}

		template <typename Ctx>
		static void read_request(const Ctx &ctx, char **&ptr) {
			// map ptr so that it points to the pointer in shm, the plugin
			// will then fill in that pointer
			ctx.shm.map_data_array(ptr, 1);
		}

		template <typename Ctx>
		static void write_response(const Ctx &ctx, char **ptr, const dispatcher_response &response) {
			// ptr now points to the pointer to the data array and response contains
			// the length, so we just write that to shm
			auto size = response.response;
			ctx.shm.write_data_array(*ptr, size);
		}

		template <typename Ctx>
		static void read_response(const Ctx &ctx, char **ptr, const dispatcher_response &response) {
			// now, allocate a chunk of memory and copy what's in shm to it
			auto size = response.response;
			char *chunk = ctx.allocator.allocate_chunk(size);
			ctx.shm.read_data_array(chunk, size);

			// and now point the pointer that ptr points to to it
			ptr = &chunk;
		}

		static inline std::ostream &show_request(std::ostream &os, char **ptr) {
			return os << "N/A";
		}

		static inline std::ostream &show_response(std::ostream &os, char **ptr, const dispatcher_response &response) {
			return os << "chunk(" << response.response << ")";
		}
	};

	/// This is used for effEditGetRect, which takes a pointer to a pointer to an ERect.
	/// Its job is to point the pointer either to null or to a valid ERect.
	/// There's no mention of ever freeing this ERect, so we'll just assume
	/// that there is only a single one that lives as part of the plugin state.
	///
	/// Similarly to chunk_out, we store the pointer whose address is passed to
	/// the plugin in shm.
	struct rect_out {
		template <typename Ctx>
		static void write_request(const Ctx &ctx, ERect **ptr) {
			// ignore the value of ptr, just write a single nullptr to the
			// shared memory
			ctx.shm.template write_data<ERect *>(nullptr);
		}

		template <typename Ctx>
		static void read_request(const Ctx &ctx, ERect **&ptr) {
			// map ptr so that it points to the pointer in shm, the plugin
			// will then fill in that pointer
			ctx.shm.map_data_array(ptr, 1);
		}

		template <typename Ctx>
		static void write_response(const Ctx &ctx, ERect **ptr, const dispatcher_response &response) {
			// ptr now points to the pointer to the ERect - we leave that in shm and, if present,
			// add the ERect itself
			ctx.shm.template skip_data<ERect *>();
			if (*ptr)
				ctx.shm.write_data(**ptr);
		}

		template <typename Ctx>
		static void read_response(const Ctx &ctx, ERect **ptr, const dispatcher_response &response) {
			// now, read the pointer - if it's not null, read the ERect
			// keep in mind that erect_ptr has no meaning here - we'll just use
			// it to decide if it's null or not
			ERect *rect_ptr;
			ctx.shm.read_data(rect_ptr);

			if (rect_ptr) {
				ERect &rect = ctx.allocator.get_rect();
				ctx.shm.read_data(rect);
				*ptr = &rect;
			} else {
				*ptr = nullptr;
			}
		}

		static inline std::ostream &show_request(std::ostream &os, ERect **ptr) {
			return os << reinterpret_cast<void *>(ptr);
		}

		static inline std::ostream &show_response(std::ostream &os, ERect **ptr, const dispatcher_response &response) {
			if (auto rect = *ptr)
				return os << "ERect{top: " << rect->top << ", left: " << rect->left << ", bottom: " << rect->bottom << ", right: " << rect->right << "}";
			else
				return os << "nullptr to ERect";
		}
	};
}

#endif
