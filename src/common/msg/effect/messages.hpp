#ifndef MSG_EFFECT_MESSAGES_HPP
#define MSG_EFFECT_MESSAGES_HPP

#include "common/msg/base.hpp"
#include "common/vst/ostream.hpp"
#include "pluginterfaces/vst2.x/aeffect.h"
#include "pluginterfaces/vst2.x/aeffectx.h"

#define MSG_EFFECT_MESSAGES(F) \
	F(open) \
	F(close) \
	F(setProgram) \
	F(getProgram) \
	/* F(setProgramName) */ \
	F(getProgramName) \
	F(getParamLabel) \
	F(getParamDisplay) \
	F(getParamName) \
	/* F(getVu) */ \
	F(setSampleRate) \
	F(setBlockSize) \
	F(mainsChanged) \
	F(editGetRect) \
	/* F(editOpen) */ \
	F(editClose) \
	/* F(editDraw) */ \
	/* F(editMouse) */ \
	/* F(editKey) */ \
	F(editIdle) \
	/* F(editTop) */ \
	/* F(editSleep) */ \
	/* F(identify) */ \
	F(getChunk) \
	/* F(setChunk) */ \
	/* F(processEvents) */ \
	F(canBeAutomated) \
	/* F(string2Parameter) */ \
	/* F(getNumProgramCategories) */ \
	F(getProgramNameIndexed) \
	/* F(copyProgram) */ \
	/* F(connectInput) */ \
	/* F(connectOutput) */ \
	/* F(getInputProperties) */ \
	/* F(getOutputProperies) */ \
	F(getPlugCategory) \
	/* F(getCurrentPosition) */ \
	/* F(getDestionationBuffer) */ \
	/* F(offlineNotify) */ \
	/* F(offlinePrepare) */ \
	/* F(offlineRun) */ \
	/* F(processVarIo) */ \
	/* F(setSpeakerArrangement) */ \
	/* F(setBlockSizeAndSampleRate) */ \
	F(setBypass) \
	F(getEffectName) \
	F(getErrorText) \
	F(getVendorString) \
	F(getProductString) \
	F(getVendorVersion) \
	F(canDo) \
	F(getTailSize) \
	F(idle) \
	/* F(getIcon) */ \
	/* F(setViewPosition) */ \
	F(getParameterProperties) \
	/* F(keysRequired) */ \
	F(getVstVersion) \
	F(editKeyDown) \
	F(editKeyUp) \
	F(setEditKnobMode) \
	/* F(getMidiProgramName) */ \
	/* F(getCurrentMidiProgram) */ \
	/* F(getMidiProgramCategory) */ \
	F(hasMidiProgramsChanged) \
	/* F(getMidiKeyName) */ \
	F(beginSetProgram) \
	F(endSetProgram) \
	/* F(getSpeakerArrangement) */ \
	F(shellGetNextPlugin) \
	F(startProcess) \
	F(stopProcess) \
	F(setTotalSampleToProcess) \
	F(setPanLaw) \
	/* F(beginLoadBank) */ \
	/* F(beginLoadProgram) */ \
	F(setProcessPrecision) \
	F(getNumMidiInputChannels) \
	F(getNumMidiOutputChannels)

namespace msg::effect {
	struct open : public msg<effOpen> {};
	struct close : public msg<effClose>, plain_ret {};
	struct setProgram : public msg<effSetProgram>, value {};
	struct getProgram : public msg<effGetProgram>, plain_ret {};
	// struct setProgramName : public msg<effSetProgramName> {};
	struct getProgramName : public msg<effGetProgramName>, str_out_ptr<kVstMaxProgNameLen> {};
	struct getParamLabel : public msg<effGetParamLabel>, index, str_out_ptr<kVstMaxParamStrLen> {};
	struct getParamDisplay : public msg<effGetParamDisplay>, index, str_out_ptr<kVstMaxParamStrLen> {};
	struct getParamName : public msg<effGetParamName>, index, str_out_ptr<kVstMaxParamStrLen> {};
	// struct getVu : public msg<effGetVu> {};
	struct setSampleRate : public msg<effSetSampleRate>, opt {};
	struct setBlockSize : public msg<effSetBlockSize>, value {};
	struct mainsChanged : public msg<effMainsChanged>, value {};
	struct editGetRect : public msg<effEditGetRect>, payload_ptr<casted_ptr<ERect *, rect_out>>, plain_ret {};
	// struct editOpen : public msg<effEditOpen> {};
	struct editClose : public msg<effEditClose> {};
	// struct editDraw : public msg<effEditDraw> {};
	// struct editMouse : public msg<effEditMouse> {};
	// struct editKey : public msg<effEditKey> {};
	struct editIdle : public msg<effEditIdle> {};
	// struct editTop : public msg<effEditTop> {};
	// struct editSleep : public msg<effEditSleep> {};
	// struct identify : public msg<effIdentify> {};
	struct getChunk : public msg<effGetChunk>, index, payload_ptr<casted_ptr<char *, chunk_out>>, plain_ret {};
	// struct setChunk : public msg<effSetChunk> {};
	// struct processEvents : public msg<effProcessEvents> {};
	struct canBeAutomated : public msg<effCanBeAutomated>, index, plain_ret {};
	// struct string2Parameter : public msg<effString2Parameter> {};
	// struct getNumProgramCategories : public msg<effGetNumProgramCategories> {};
	struct getProgramNameIndexed : public msg<effGetProgramNameIndexed>, index, str_out_ptr<kVstMaxProgNameLen>, plain_ret {};
	// struct copyProgram : public msg<effCopyProgram> {};
	// struct connectInput : public msg<effConnectInput> {};
	// struct connectOutput : public msg<effConnectOutput> {};
	// struct getInputProperties : public msg<effGetInputProperties> {};
	// struct getOutputProperies : public msg<effGetOutputProperies> {};
	struct getPlugCategory : public msg<effGetPlugCategory>, plain_ret {};
	// struct getCurrentPosition : public msg<effGetCurrentPosition> {};
	// struct getDestionationBuffer : public msg<effGetDestionationBuffer> {};
	// struct offlineNotify : public msg<effOfflineNotify> {};
	// struct offlinePrepare : public msg<effOfflinePrepare> {};
	// struct offlineRun : public msg<effOfflineRun> {};
	// struct processVarIo : public msg<effProcessVarIo> {};
	// struct setSpeakerArrangement : public msg<effSetSpeakerArrangement> {};
	// struct setBlockSizeAndSampleRate : public msg<effSetBlockSizeAndSampleRate> {};
	struct setBypass : public msg<effSetBypass>, value {};
	struct getEffectName : public msg<effGetEffectName>, str_out_ptr<kVstMaxEffectNameLen> {};
	struct getErrorText : public msg<effGetErrorText>, str_out_ptr<256> {}; // 256 chars seems about right
	struct getVendorString : public msg<effGetVendorString>, str_out_ptr<kVstMaxVendorStrLen> {};
	struct getProductString : public msg<effGetProductString>, str_out_ptr<kVstMaxProductStrLen> {};
	struct getVendorVersion : public msg<effGetVendorVersion>, plain_ret {};
	struct canDo : public msg<effCanDo>, str_in_ptr {};
	struct getTailSize : public msg<effGetTailSize>, plain_ret {};
	struct idle : public msg<effIdle>, plain_ret {};
	// struct getIcon : public msg<effGetIcon> {};
	// struct setViewPosition : public msg<effSetViewPosition> {};
	struct getParameterProperties : public msg<effGetParameterProperties>, index, payload_ptr<casted_ptr<VstParameterProperties, out_single<VstParameterProperties>>>, plain_ret {};
	// struct keysRequired : public msg<effKeysRequired> {};
	struct getVstVersion : public msg<effGetVstVersion>, plain_ret {};
	struct editKeyDown : public msg<effEditKeyDown>, index, value, opt, plain_ret {};
	struct editKeyUp : public msg<effEditKeyUp>, index, value, opt, plain_ret {};
	struct setEditKnobMode : public msg<effSetEditKnobMode>, value {};
	// struct getMidiProgramName : public msg<effGetMidiProgramName> {};
	// getCurrentMidiProgram
	// getMidiProgramCategory
	struct hasMidiProgramsChanged : public msg<effHasMidiProgramsChanged>, index, plain_ret {};
	// struct getMidiKeyName : public msg<effGetMidiKeyName> {};
	struct beginSetProgram : public msg<effBeginSetProgram> {};
	struct endSetProgram : public msg<effEndSetProgram> {};
	// struct getSpeakerArrangement : public msg<effGetSpeakerArrangement> {};
	struct shellGetNextPlugin : public msg<effShellGetNextPlugin>, str_out_ptr<kVstMaxProductStrLen>, plain_ret {};
	struct startProcess : public msg<effStartProcess> {};
	struct stopProcess : public msg<effStopProcess> {};
	struct setTotalSampleToProcess : public msg<effSetTotalSampleToProcess>, value {};
	struct setPanLaw : public msg<effSetPanLaw>, value, opt {};
	// struct beginLoadBank : public msg<effBeginLoadBank> {};
	// struct beginLoadProgram : public msg<effBeginLoadProgram> {};
	struct setProcessPrecision : public msg<effSetProcessPrecision>, value {};
	struct getNumMidiInputChannels : public msg<effGetNumMidiInputChannels>, plain_ret {};
	struct getNumMidiOutputChannels : public msg<effGetNumMidiOutputChannels>, plain_ret {};
}

#endif
