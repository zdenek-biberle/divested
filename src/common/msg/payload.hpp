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

	/// A C-style string that works as an input to the dispatcher - it is stored
	/// in shared memory on the way in and that's it.
	/// TODO: I don't like all the strlen calls, perhaps store the length in shm
	/// as well?
	struct in_str {
		template <typename Ctx>
		static void write_request(const Ctx &ctx, char *ptr) {
			ctx.shm.write_data_array(ptr, ::strlen(ptr));
		}

		template <typename Ctx>
		static void read_request(const Ctx &ctx, char *&ptr) {
			char *tmp_ptr;
			ctx.shm.map_data_array(tmp_ptr, 0);
			size_t len = ::strlen(tmp_ptr);
			ctx.shm.map_data_array(ptr, len);
		}

		template <typename Ctx, typename Response>
		static void write_response(const Ctx &ctx, char *ptr, const Response &response) {
			ctx.shm.template skip_data_array<char>(::strlen(ptr));
		}

		template <typename Ctx, typename Response>
		static void read_response(const Ctx &ctx, char *ptr, const Response &response) {
			ctx.shm.template skip_data_array<char>(::strlen(ptr));
		}

		static inline std::ostream &show_request(std::ostream &os, char *ptr) {
			return os << "\"" << ptr << "\"";
		}

		template <typename Response>
		static std::ostream &show_response(std::ostream &os, char *ptr, const Response &response) {
			return os << "N/A";
		}
	};

	/// Shows the contents of out_array as a string.
	struct out_array_show_str {
		static inline std::ostream &show(std::ostream &os, char *ptr, size_t size) {
			return os << util::str_writer{ptr, size};
		}
	};

	/// Shows the contents of out_single directly.
	template <typename T>
	struct out_single_direct {
		static std::ostream &show(std::ostream &os, T *ptr, size_t size) {
			return os << *ptr;
		}
	};

	/// An array of items that serves as an output from the dispatcher. It is
	/// allocated within shm, filled within the dispatcher and then copied
	/// to its expected location.
	template <typename T, size_t Size, typename Show>
	struct out_array {
		template <typename Ctx>
		static void write_request(const Ctx &ctx, T *ptr) {
			ctx.shm.template skip_data_array<T>(Size);
		}

		template <typename Ctx>
		static void read_request(const Ctx &ctx, T *&ptr) {
			ctx.shm.map_data_array(ptr, Size);
		}

		template <typename Ctx, typename Response>
		static void write_response(const Ctx &ctx, T *ptr, const Response &response) {
			ctx.shm.template skip_data_array<T>(Size);
		}

		template <typename Ctx, typename Response>
		static void read_response(const Ctx &ctx, T *ptr, const Response &response) {
			ctx.shm.read_data_array(ptr, Size);
		}

		static inline std::ostream &show_request(std::ostream &os, T *ptr) {
			return os << "N/A";
		}

		template <typename Response>
		static inline std::ostream &show_response(std::ostream &os, T *ptr, const Response &response) {
			return Show::show(os, ptr, Size);
		}
	};

	template <typename T>
	struct out_single : public out_array<T, 1, out_single_direct<T>>
	{};


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
