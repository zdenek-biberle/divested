#include "common/msg/type.hpp"

namespace msg {
	const char *type_to_name(type_t type) {
		switch (type) {
			#define MSG_TYPE_HANDLER(TYPE) case type_t::TYPE: return #TYPE;
			MSG_TYPES(MSG_TYPE_HANDLER)
			#undef MSG_TYPE_HANDLER
			default: {
				std::stringstream ss;
				ss << "Unknown msg::type_t value: " << static_cast<int>(type);
				throw std::runtime_error(ss.str());
			}
		}
	}
}
