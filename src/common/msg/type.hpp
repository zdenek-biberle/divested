#ifndef MSG_TYPE_HPP
#define MSG_TYPE_HPP

#include <sstream>

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
}

#endif
