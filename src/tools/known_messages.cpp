#include <iostream>

#include "common/msg/effect/messages.hpp"
#include "common/msg/master/messages.hpp"
#include "common/msg/base.hpp"

template <typename Msg>
void print_message_info(const char *name) {
	std::cout << "\t" << static_cast<int>(Msg::opcode) << "\t" << name;

	if constexpr (msg::has_index<Msg>)
		std::cout << "\tindex";

	if constexpr (msg::has_value<Msg>)
		std::cout << "\tvalue";

	if constexpr (msg::has_payload_ptr<Msg>)
		std::cout << "\tpayload_ptr";

	if constexpr (msg::has_opt<Msg>)
		std::cout << "\topt";

	if constexpr (msg::has_plain_ret<Msg>)
		std::cout << "\tplain_ret";

	std::cout << std::endl;
}

#define MASTER_HANDLER(MSG) print_message_info<msg::master::MSG>(#MSG);
#define EFFECT_HANDLER(MSG) print_message_info<msg::effect::MSG>(#MSG);

int main() {

	std::cout << "Master messages:" << std::endl;
	MSG_MASTER_MESSAGES(MASTER_HANDLER)
	std::cout << std::endl << "Effect messages:" << std::endl;
	MSG_EFFECT_MESSAGES(EFFECT_HANDLER)
}
