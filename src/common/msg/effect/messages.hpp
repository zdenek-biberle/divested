#ifndef MSG_EFFECT_MESSAGES_HPP
#define MSG_EFFECT_MESSAGES_HPP

#include "common/msg/base.hpp"
#include "common/vst/ostream.hpp"
#include "pluginterfaces/vst2.x/aeffect.h"
#include "pluginterfaces/vst2.x/aeffectx.h"

#define MSG_EFFECT_MESSAGES(F) \
	F(open) \
	F(close) \
	F(set_program) \
	F(get_program) \
	F(set_program_name) \
	F(get_program_name) \
	F(get_param_label) \
	F(get_param_display) \
	F(get_param_name) \
	F(get_vu) \
	F(set_sample_rate) \
	F(set_block_size) \
	F(mains_changed) \
	F(edit_get_rect) \
	/* F(edit_open) */ \
	F(edit_close) \
	/* F(edit_draw) */ \
	/* F(edit_mouse) */ \
	/* F(edit_key) */ \
	F(edit_idle) \
	/* F(edit_top) */ \
	/* F(edit_sleep) */ \
	F(identify) \
	F(get_chunk) \
	/* F(set_chunk) */ \
	F(process_events) \
	F(can_be_automated) \
	F(string2parameter) \
	F(get_num_program_categories) \
	F(get_program_name_indexed) \
	/* F(copy_program) */ \
	F(connect_input) \
	F(connect_output) \
	F(get_input_properties) \
	F(get_output_properies) \
	F(get_plug_category) \
	F(get_current_position) \
	/* F(get_destionation_buffer) */ \
	/* F(offline_notify) */ \
	/* F(offline_prepare) */ \
	/* F(offline_run) */ \
	/* F(process_var_io) */ \
	/* F(set_speaker_arrangement) */ \
	/* F(set_block_size_and_sample_rate) */ \
	F(set_bypass) \
	F(get_effect_name) \
	F(get_error_text) \
	F(get_vendor_string) \
	F(get_product_string) \
	F(get_vendor_version) \
	/* F(vendor_specific) */ \
	F(can_do) \
	F(get_tail_size) \
	F(idle) \
	/* F(get_icon) */ \
	/* F(set_view_position) */ \
	F(get_parameter_properties) \
	/* F(keys_required) */ \
	F(get_vst_version) \
	F(edit_key_down) \
	F(edit_key_up) \
	F(set_edit_knob_mode) \
	/* F(get_midi_program_name) */ \
	/* F(get_current_midi_program) */ \
	/* F(get_midi_program_category) */ \
	F(has_midi_programs_changed) \
	/* F(get_midi_key_name) */ \
	F(begin_set_program) \
	F(end_set_program) \
	/* F(get_speaker_arrangement) */ \
	F(shell_get_next_plugin) \
	F(start_process) \
	F(stop_process) \
	F(set_total_sample_to_process) \
	F(set_pan_law) \
	/* F(begin_load_bank) */ \
	/* F(begin_load_program) */ \
	F(set_process_precision) \
	F(get_num_midi_input_channels) \
	F(get_num_midi_output_channels)

namespace msg::effect {
	struct open : public msg<effOpen> {};
	struct close : public msg<effClose>, plain_ret {};
	struct set_program : public msg<effSetProgram>, value {};
	struct get_program : public msg<effGetProgram>, plain_ret {};
	struct set_program_name : public msg<effSetProgramName>, str_in_ptr {};
	struct get_program_name : public msg<effGetProgramName>, str_out_ptr<kVstMaxProgNameLen> {};
	struct get_param_label : public msg<effGetParamLabel>, index, str_out_ptr<kVstMaxParamStrLen> {};
	struct get_param_display : public msg<effGetParamDisplay>, index, str_out_ptr<kVstMaxParamStrLen> {};
	struct get_param_name : public msg<effGetParamName>, index, str_out_ptr<kVstMaxParamStrLen> {};
	struct get_vu : public msg<effGetVu>, plain_ret {};
	struct set_sample_rate : public msg<effSetSampleRate>, opt {};
	struct set_block_size : public msg<effSetBlockSize>, value {};
	struct mains_changed : public msg<effMainsChanged>, value {};
	struct edit_get_rect : public msg<effEditGetRect>, payload_ptr<casted_ptr<ERect *, rect_out>>, plain_ret {};
	// struct edit_open : public msg<effEditOpen> {};
	struct edit_close : public msg<effEditClose> {};
	// struct edit_draw : public msg<effEditDraw> {};
	// struct edit_mouse : public msg<effEditMouse> {};
	// struct edit_key : public msg<effEditKey> {};
	struct edit_idle : public msg<effEditIdle> {};
	// struct edit_top : public msg<effEditTop> {};
	// struct edit_sleep : public msg<effEditSleep> {};
	struct identify : public msg<effIdentify>, plain_ret {};
	struct get_chunk : public msg<effGetChunk>, index, payload_ptr<casted_ptr<char *, chunk_out>>, plain_ret {};
	// struct set_chunk : public msg<effSetChunk> {};
	struct process_events : public msg<effProcessEvents>, payload_ptr<casted_ptr<VstEvents, vst_events_out>>, plain_ret {};
	struct can_be_automated : public msg<effCanBeAutomated>, index, plain_ret {};
	struct string2parameter : public msg<effString2Parameter>, index, str_in_ptr, plain_ret {};
	struct get_num_program_categories : public msg<effGetNumProgramCategories>, plain_ret {};
	struct get_program_name_indexed : public msg<effGetProgramNameIndexed>, index, str_out_ptr<kVstMaxProgNameLen>, plain_ret {};
	// struct copy_program : public msg<effCopyProgram> {};
	struct connect_input : public msg<effConnectInput>, index, value, plain_ret {};
	struct connect_output : public msg<effConnectOutput>, index, value, plain_ret {};
	struct get_input_properties : public msg<effGetInputProperties>, index, ptr_to_1<VstPinProperties, shm_inout_1>, plain_ret {};
	struct get_output_properies : public msg<effGetOutputProperties>, index, ptr_to_1<VstPinProperties, shm_inout_1>, plain_ret {};
	struct get_plug_category : public msg<effGetPlugCategory>, plain_ret {};
	struct get_current_position : public msg<effGetCurrentPosition>, plain_ret {};
	// struct get_destionation_buffer : public msg<effGetDestionationBuffer> {};
	// struct offline_notify : public msg<effOfflineNotify> {};
	// struct offline_prepare : public msg<effOfflinePrepare> {};
	// struct offline_run : public msg<effOfflineRun> {};
	// struct process_var_io : public msg<effProcessVarIo> {};
	// struct set_speaker_arrangement : public msg<effSetSpeakerArrangement> {};
	// struct set_block_size_and_sample_rate : public msg<effSetBlockSizeAndSampleRate> {};
	struct set_bypass : public msg<effSetBypass>, value {};
	struct get_effect_name : public msg<effGetEffectName>, str_out_ptr<kVstMaxEffectNameLen> {};
	struct get_error_text : public msg<effGetErrorText>, str_out_ptr<256> {}; // 256 chars seems about right
	struct get_vendor_string : public msg<effGetVendorString>, str_out_ptr<kVstMaxVendorStrLen> {};
	struct get_product_string : public msg<effGetProductString>, str_out_ptr<kVstMaxProductStrLen> {};
	struct get_vendor_version : public msg<effGetVendorVersion>, plain_ret {};
	// struct vendor_specific : public msg<effVendorSpecific> {};
	struct can_do : public msg<effCanDo>, str_in_ptr, plain_ret {};
	struct get_tail_size : public msg<effGetTailSize>, plain_ret {};
	struct idle : public msg<effIdle>, plain_ret {};
	// struct get_icon : public msg<effGetIcon> {};
	// struct set_view_position : public msg<effSetViewPosition> {};
	struct get_parameter_properties : public msg<effGetParameterProperties>, index, ptr_to_1<VstParameterProperties, shm_inout_1>, plain_ret {};
	// struct keys_required : public msg<effKeysRequired> {};
	struct get_vst_version : public msg<effGetVstVersion>, plain_ret {};
	struct edit_key_down : public msg<effEditKeyDown>, index, value, opt, plain_ret {};
	struct edit_key_up : public msg<effEditKeyUp>, index, value, opt, plain_ret {};
	struct set_edit_knob_mode : public msg<effSetEditKnobMode>, value {};
	// struct get_midi_program_name : public msg<effGetMidiProgramName> {};
	// struct get_current_midi_program
	// struct get_midi_program_category
	struct has_midi_programs_changed : public msg<effHasMidiProgramsChanged>, index, plain_ret {};
	// struct get_midi_key_name : public msg<effGetMidiKeyName> {};
	struct begin_set_program : public msg<effBeginSetProgram> {};
	struct end_set_program : public msg<effEndSetProgram> {};
	// struct get_speaker_arrangement : public msg<effGetSpeakerArrangement> {};
	struct shell_get_next_plugin : public msg<effShellGetNextPlugin>, str_out_ptr<kVstMaxProductStrLen>, plain_ret {};
	struct start_process : public msg<effStartProcess> {};
	struct stop_process : public msg<effStopProcess> {};
	struct set_total_sample_to_process : public msg<effSetTotalSampleToProcess>, value {};
	struct set_pan_law : public msg<effSetPanLaw>, value, opt {};
	// struct begin_load_bank : public msg<effBeginLoadBank> {};
	// struct begin_load_program : public msg<effBeginLoadProgram> {};
	struct set_process_precision : public msg<effSetProcessPrecision>, value {};
	struct get_num_midi_input_channels : public msg<effGetNumMidiInputChannels>, plain_ret {};
	struct get_num_midi_output_channels : public msg<effGetNumMidiOutputChannels>, plain_ret {};
}

#endif
