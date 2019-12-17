#ifndef VST_OSTREAM_HPP
#define VST_OSTREAM_HPP

#include <ostream>
#include "pluginterfaces/vst2.x/aeffectx.h"

/// Implements streaming to ostream for VstParameterProperties
std::ostream &operator<<(std::ostream &os, const VstParameterProperties& obj);

#endif
