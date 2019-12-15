#ifndef MSG_TYPE_HPP
#define MSG_TYPE_HPP

#include <sstream>
#include <stdexcept>

#define MSG_TYPES(F) \
	F(return_) \
	F(dispatcher) \
	F(process) \
	F(set_parameter) \
	F(get_parameter) \
	F(process_replacing) \
	F(process_double_replacing)

namespace msg {
	enum class type_t {
		// sent when a message contains the return value of a call
		return_ = 0x5a5aa5a5,

		// call the dispatcher procedure
		dispatcher,

		// call the `process` procedure
		process,

		// call the `setParameter` procedure
		set_parameter,

		// call the `getParameter` procedure
		get_parameter,

		// call the `processReplacing` procedure
		process_replacing,

		// call the `processDoubleReplacing` procedure
		process_double_replacing,
	};

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

#endif
