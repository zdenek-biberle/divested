#ifndef MSG_TYPE_HPP
#define MSG_TYPE_HPP

#include <sstream>
#include <stdexcept>

#include "pluginterfaces/vst2.x/aeffect.h"

#define MSG_TYPES(F) \
	F(return_) \
	F(dispatcher) \
	F(process) \
	F(set_parameter) \
	F(get_parameter) \
	F(process_replacing) \
	F(process_double_replacing) \
	F(instantiate_handler)

namespace msg {
	enum class type_t {
		// sent when a message contains the return value of a call
		return_ = 0x5a5aa5a5,

		// call the dispatcher procedure
		dispatcher,

		// call the `setParameter` procedure
		set_parameter,

		// call the `getParameter` procedure
		get_parameter,

		// call the `process` procedure
		process,

		// call the `processReplacing` procedure
		process_replacing,

		// call the `processDoubleReplacing` procedure
		process_double_replacing,

		// instantiate a handler for a new thread
		instantiate_handler,
	};

	const char *type_to_name(type_t type);

	struct dispatcher_request {
		VstInt32 index = 0;
		VstIntPtr value = 0;
		void *ptr = nullptr;
		float opt = 0.f;

		inline friend std::ostream &operator<<(std::ostream &os, const dispatcher_request &req) {
			return os << "dispatcher_request{index: " << req.index << ", value: " << req.value << ", ptr: " << req.ptr << ", opt: " << req.opt << "}";
		}
	};

	struct dispatcher_response {
		VstIntPtr response = 0;

		inline friend std::ostream &operator<<(std::ostream &os, const dispatcher_response &res) {
			return os << "dispatcher_response{response: " << res.response << "}";
		}
	};

	struct get_parameter_request {
		VstInt32 index;

		inline friend std::ostream &operator<<(std::ostream &os, const get_parameter_request &req) {
			return os << "get_parameter_request{index: " << req.index << "}";
		}
	};

	struct get_parameter_response {
		float value;

		inline friend std::ostream &operator<<(std::ostream &os, const get_parameter_response &res) {
			return os << "get_parameter_response{index: " << res.value << "}";
		}
	};

	struct set_parameter_request {
		VstInt32 index;
		float opt;

		inline friend std::ostream &operator<<(std::ostream &os, const set_parameter_request &req) {
			return os << "set_parameter_request{index: " << req.index << ", opt: " << req.opt << "}";
		}
	};

	struct set_parameter_response {
		inline friend std::ostream &operator<<(std::ostream &os, const set_parameter_response &req) {
			return os << "set_parameter_response{}";
		}
	};

	template <typename T>
	struct process_request {
		VstInt32 index;
		VstInt32 num_inputs;
		VstInt32 num_outputs;
		T **inputs;
		T **outputs;

		inline friend std::ostream &operator<<(std::ostream &os, const process_request &req) {
			return os << "process_request{ num_inputs=" << req.num_inputs << " num_outputs=" << req.num_outputs << " inputs=" << reinterpret_cast<void *>(req.inputs) << " output=" << reinterpret_cast<void *>(req.outputs) << " }";
		}
	};

	struct process_response {
		inline friend std::ostream &operator<<(std::ostream &os, const process_response &req) {
			return os << "process_response{}";
		}
	};

	struct instantiate_handler_request {
		int index;

		inline friend std::ostream &operator<<(std::ostream &os, const instantiate_handler_request &req) {
			return os << "instantiate_handler_request{ index=" << req.index << " }";
		}
	};

	struct instantiate_handler_response {
		inline friend std::ostream &operator<<(std::ostream &os, const instantiate_handler_response &res) {
			return os << "instantiate_handler_response{}";
		}
	};
}

#endif
