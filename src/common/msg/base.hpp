#ifndef MSG_BASE_HPP
#define MSG_BASE_HPP

#include "common/msg/payload.hpp"
#include "pluginterfaces/vst2.x/aeffect.h"

namespace msg {
	struct opcode_tag {};

	template <VstInt32 op>
	struct msg : public opcode_tag {
		static constexpr VstInt32 opcode = op;
	};

	template <typename T> constexpr bool has_opcode = std::is_base_of_v<opcode_tag, T>;

	struct index {};
	template <typename T> constexpr bool has_index = std::is_base_of_v<index, T>;

	struct value {};
	template <typename T> constexpr bool has_value = std::is_base_of_v<value, T>;

	struct opt {};
	template <typename T> constexpr bool has_opt = std::is_base_of_v<opt, T>;

	struct payload_ptr_tag {};

	/// A payload_ptr means that ptr is an actual pointer to some
	/// payload defined by Payload.
	template <typename Payload>
	struct payload_ptr : public payload_ptr_tag {
		using payload = Payload;
	};
	
	template <typename T> constexpr bool has_payload_ptr = std::is_base_of_v<payload_ptr_tag, T>;

	// This is just a shortcut for payload_ptr of out_array<char, ...>
	template <size_t Size>
	using str_out_ptr = payload_ptr<casted_ptr<char, out_array<char, Size, out_array_show_str>>>;

	// This is just a shortcut for payload_ptr of in_str.
	using str_in_ptr = payload_ptr<casted_ptr<char, in_str>>;

	struct plain_ret {};
	template <typename T> constexpr bool has_plain_ret = std::is_base_of_v<plain_ret, T>;
}

#endif
