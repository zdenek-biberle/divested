#ifndef MSG_MASTER_MESSAGES_HPP
#define MSG_MASTER_MESSAGES_HPP

#include "common/msg/base.hpp"
#include "pluginterfaces/vst2.x/aeffect.h"

#define MSG_MASTER_MESSAGES(F) \
	F(automate) \
	F(version) \
	F(current_id) \
	F(idle)

namespace msg::master {
	struct automate : public msg<audioMasterAutomate>, index, opt {};
	struct version : public msg<audioMasterVersion>, plain_ret {};
	struct current_id : public msg<audioMasterCurrentId>, plain_ret {};
	struct idle : public msg<audioMasterIdle> {};
}

#endif
