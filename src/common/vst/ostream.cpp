#include "common/vst/ostream.hpp"

#define OBJ_START(x) return os << #x "{ "
#define OUT(x) << #x "=" << obj.x << " "
#define OUT_STR(x) << #x "=" << str_writer{obj.x, sizeof(obj.x)} << " "
#define OBJ_END << "}" << std::endl

/// This is a simple helper for writing out strings.
struct str_writer {
	const char * const ptr;
	const size_t size;
	
	bool has_zero() const {
		for (size_t i = 0; i < size; i++)
			if (ptr[i] == '\0')
				return true;

		return false;
	}

	friend std::ostream &operator<<(std::ostream &os, const str_writer &str) {
		if (str.has_zero())
			return os << "\"" << str.ptr << "\"";
		else
			return os << "invalid";
	}

};

std::ostream &operator<<(std::ostream &os, const VstParameterProperties& obj) {
	OBJ_START(VstParameterProperties)
		OUT(stepFloat)
		OUT(smallStepFloat)
		OUT(largeStepFloat)
		OUT_STR(label)
		OUT(flags)
		OUT(minInteger)
		OUT(maxInteger)
		OUT(stepInteger)
		OUT(largeStepInteger)
		OUT_STR(shortLabel)
		OUT(displayIndex)
		OUT(category)
		OUT(numParametersInCategory)
		OUT(reserved)
		OUT_STR(categoryLabel)
	OBJ_END;
}
