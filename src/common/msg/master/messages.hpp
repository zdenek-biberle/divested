#ifndef MSG_MASTER_MESSAGES_HPP
#define MSG_MASTER_MESSAGES_HPP

#include "common/msg/base.hpp"
#include "pluginterfaces/vst2.x/aeffect.h"

#define MSG_MASTER_MESSAGES(F) \
	F(automate) \
	F(version) \
	F(current_id) \
	F(idle) \
	/* F(pin_connected) */ \
	/* F(want_midi) */ \
	/* F(get_time) */ \
	/* F(process_events) */ \
	/* F(set_time) */ \
	/* F(tempo_at) */ \
	/* F(get_num_automatable_parameters) */ \
	/* F(get_parameter_quantization) */ \
	/* F(io_changed) */ \
	/* F(need_idle) */ \
	/* F(size_window) */ \
	/* F(get_sample_rate) */ \
	/* F(get_block_size) */ \
	/* F(get_input_latency) */ \
	/* F(get_output_latency) */ \
	/* F(get_previous_plug) */ \
	/* F(get_next_plug) */ \
	/* F(will_replace_or_accumulate) */ \
	/* F(get_current_process_level) */ \
	/* F(get_automation_state) */ \
	/* F(offline_start) */ \
	/* F(offline_read) */ \
	/* F(offline_write) */ \
	/* F(offline_get_current_pass) */ \
	/* F(offline_get_current_meta_pass) */ \
	/* F(set_output_sample_rate) */ \
	/* F(get_output_speaker_arrangement) */ \
	/* F(get_vendor_string) */ \
	/* F(get_product_string) */ \
	/* F(get_vendor_version) */ \
	/* F(vendor_specific) */ \
	/* F(set_icon) */ \
	/* F(can_do) */ \
	/* F(get_language) */ \
	/* F(open_window) */ \
	/* F(close_window) */ \
	/* F(get_directory) */ \
	/* F(update_display) */ \
	/* F(begin_edit) */ \
	/* F(end_edit) */ \
	/* F(open_file_selector) */ \
	/* F(close_file_selector) */ \
	/* F(edit_file) */ \
	/* F(get_chunk_file) */ \
	/* F(get_input_speaker_arrangement) */

namespace msg::master {
	struct automate : public msg<audioMasterAutomate>, index, opt {};
	struct version : public msg<audioMasterVersion>, plain_ret {};
	struct current_id : public msg<audioMasterCurrentId>, plain_ret {};
	struct idle : public msg<audioMasterIdle> {};
	// struct pin_connected : public msg<audioMasterPinConnected> {};
	// struct want_midi : public msg<audioMasterWantMidi> {};
	// struct get_time : public msg<audioMasterGetTime> {};
	// struct process_events : public msg<audioMasterProcessEvents> {};
	// struct set_time : public msg<audioMasterSetTime> {};
	// struct tempo_at : public msg<audioMasterTempoAt> {};
	// struct get_num_automatable_parameters : public msg<audioMasterGetNumAutomatableParameters> {};
	// struct get_parameter_quantization : public msg<audioMasterGetParameterQuantization> {};
	// struct io_changed : public msg<audioMasterIoChanged> {};
	// struct need_idle : public msg<audioMasterNeedIdle> {};
	// struct size_window : public msg<audioMasterSizeWindow> {};
	// struct get_sample_rate : public msg<audioMasterGetSampleRate> {};
	// struct get_block_size : public msg<audioMasterGetBlockSize> {};
	// struct get_input_latency : public msg<audioMasterGetInputLatency> {};
	// struct get_output_latency : public msg<audioMasterGetOutputLatency> {};
	// struct get_previous_plug : public msg<audioMasterGetPreviousPlug> {};
	// struct get_next_plug : public msg<audioMasterGetNextPlug> {};
	// struct will_replace_or_accumulate : public msg<audioMasterWillReplaceOrAccumulate> {};
	// struct get_current_process_level : public msg<audioMasterGetCurrentProcessLevel> {};
	// struct get_automation_state : public msg<audioMasterGetAutomationState> {};
	// struct offline_start : public msg<audioMasterOfflineStart> {};
	// struct offline_read : public msg<audioMasterOfflineRead> {};
	// struct offline_write : public msg<audioMasterOfflineWrite> {};
	// struct offline_get_current_pass : public msg<audioMasterOfflineGetCurrentPass> {};
	// struct offline_get_current_meta_pass : public msg<audioMasterOfflineGetCurrentMetaPass> {};
	// struct set_output_sample_rate : public msg<audioMasterSetOutputSampleRate> {};
	// struct get_output_speaker_arrangement : public msg<audioMasterGetOutputSpeakerArrangement> {};
	// struct get_vendor_string : public msg<audioMasterGetVendorString> {};
	// struct get_product_string : public msg<audioMasterGetProductString> {};
	// struct get_vendor_version : public msg<audioMasterGetVendorVersion> {};
	// struct vendor_specific : public msg<audioMasterVendorSpecific> {};
	// struct set_icon : public msg<audioMasterSetIcon> {};
	// struct can_do : public msg<audioMasterCanDo> {};
	// struct get_language : public msg<audioMasterGetLanguage> {};
	// struct open_window : public msg<audioMasterOpenWindow> {};
	// struct close_window : public msg<audioMasterCloseWindow> {};
	// struct get_directory : public msg<audioMasterGetDirectory> {};
	// struct update_display : public msg<audioMasterUpdateDisplay> {};
	// struct begin_edit : public msg<audioMasterBeginEdit> {};
	// struct end_edit : public msg<audioMasterEndEdit> {};
	// struct open_file_selector : public msg<audioMasterOpenFileSelector> {};
	// struct close_file_selector : public msg<audioMasterCloseFileSelector> {};
	// struct edit_file : public msg<audioMasterEditFile> {};
	// struct get_chunk_file : public msg<audioMasterGetChunkFile> {};
	// struct get_input_speaker_arrangement : public msg<audioMasterGetInputSpeakerArrangement> {};
}

#endif
