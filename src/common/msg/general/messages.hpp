#ifndef MSG_GENERAL_MESSAGES_HPP
#define MSG_GENERAL_MESSAGES_HPP

#include "common/msg/base.hpp"

#define MSG_GENERAL_MESSAGES(F) \
	F(get_parameter) \
	F(set_parameter) \
	F(process) \
	F(process_replacing) \
	F(process_double_replacing) \
	F(instantiate_handler)

namespace msg::general {
	struct get_parameter : index, plain_ret {};
	struct set_parameter : index, opt {};
	struct process : index, accumulating {};
	struct process_replacing : index, replacing {};
	struct process_double_replacing : index, replacing {};
	struct instantiate_handler : index {};
};

#endif
