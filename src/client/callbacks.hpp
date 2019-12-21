#ifndef CALLBACKS_HPP
#define CALLBACKS_HPP

#include "pluginterfaces/vst2.x/aeffect.h"

VstIntPtr VSTCALLBACK aeffect_dispatcher_proc(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
void VSTCALLBACK aeffect_process_proc(AEffect* effect, float** inputs, float** outputs, VstInt32 sampleFrames);
void VSTCALLBACK aeffect_process_double_proc(AEffect* effect, double** inputs, double** outputs, VstInt32 sampleFrames);
void VSTCALLBACK aeffect_set_parameter_proc(AEffect* effect, VstInt32 index, float parameter);
float VSTCALLBACK aeffect_get_parameter_proc(AEffect* effect, VstInt32 index);

#endif
