#include "common/util/str_writer.hpp"
#include "common/vst/ostream.hpp"

#define OBJ_START(x) return os << #x "{ "
#define OUT(x) << #x "=" << obj.x << " "
#define OUT_STR(x) << #x "=" << util::str_writer{obj.x} << " "
#define OBJ_END << "}" << std::endl

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
