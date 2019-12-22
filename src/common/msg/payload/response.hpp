#ifndef MSG_PAYLOAD_RESPONSE_HPP
#define MSG_PAYLOAD_RESPONSE_HPP

#include "common/vst/ostream.hpp"

namespace msg::payload {
	template <typename T, typename Sub>
	struct casted_ret {
		template <typename Ctx, typename Response>
		static void write_response(const Ctx &ctx, VstIntPtr ptr, const Response &response) {
			Sub::write_response(ctx, reinterpret_cast<T *>(ptr), response);
		}

		template <typename Ctx, typename Response>
		static void read_response(const Ctx &ctx, VstIntPtr &ptr, const Response &response) {
			Sub::read_response(ctx, reinterpret_cast<T *&>(ptr), response);
		}

		template <typename Response>
		static std::ostream &show_response(std::ostream &os, VstIntPtr ptr, const Response &response) {
			return Sub::show_response(os, reinterpret_cast<T *>(ptr), response);
		}
	};

	struct vst_time_info_out {
		template <typename Ctx, typename Response>
		static void write_response(Ctx &ctx, VstTimeInfo *ptr, const Response &response) {
			if (ptr) {
				ctx.pipe.write_data(true);
				ctx.pipe.write_data(*ptr);
			} else {
				ctx.pipe.write_data(false);
			}
		}

		template <typename Ctx, typename Response>
		static void read_response(Ctx &ctx, VstTimeInfo *&ptr, const Response &response) {
			bool present;
			ctx.pipe.read_data(present);

			if (present) {
				ptr = &ctx.allocator.get_time_info();
				ctx.pipe.read_data(*ptr);
			} else {
				ptr = nullptr;
			}
		}

		template <typename Response>
		static std::ostream &show_response(std::ostream &os, VstTimeInfo *ptr, const Response &response) {
			return os << *ptr;
		}
	};
}

#endif
