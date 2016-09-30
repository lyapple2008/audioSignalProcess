#include "../include/apm_ns.h"
extern "C"{
#include "highpass_filter.h"
}
#include "webrtc/modules/audio_processing/ns/include/noise_suppression.h"
#include "webrtc/modules/audio_processing/audio_buffer.h"

using namespace webrtc;

bool APM_NS::initNsModule(unsigned int frequency, int ns_mode, int input_frames, int input_channels)
{
	m_frequency = frequency;
	m_channels = input_channels;
	m_ns_mode = ns_mode;

	if (m_channels < 0) {
		return false;
	}
	m_handles.clear();
	for (int i = 0; i < m_channels; i++) {
		NsHandle *handle = NULL;
		if (WebRtcNs_Create(&handle)){
			return false;
		}
		if (WebRtcNs_Init(handle, frequency)){
			return false;
		}
		if (WebRtcNs_set_policy(handle, ns_mode)){
			return false;
		}
		m_handles.push_back(handle);
	}

	capture_buffer = new AudioBuffer(input_frames, input_channels,
									 input_frames, input_channels,
									 input_frames);
	if (!capture_buffer){
		return false;
	}

	channels_ptr_i = new short*[input_channels];
	if (!channels_ptr_i){
		return false;
	}

	init_flag = true;
	return true;
}

void APM_NS::processCaptureStream(float* data, int samples_per_channel, int input_channels)
{
	if (!init_flag){
		return;// do not process data if did not initial
	}

	AudioBuffer *capture_buffer_t = (AudioBuffer *)capture_buffer;

	//convert interleave data to de-interleave data
	float *interleave_data = data;
	for (int i = 0; i < input_channels; i++) {
		short* deinterleave_data = capture_buffer_t->data(i);
		int interleaved_idx = i;
		for (int j = 0; j < samples_per_channel; j++) {
			deinterleave_data[j] = FloatToS16(interleave_data[interleaved_idx]);
			interleaved_idx += input_channels;
		}
	}

	if (m_frequency == 32000 || m_frequency == 48000){
		capture_buffer_t->SplitIntoFrequencyBands();
	}
	for (int ch = 0; ch < m_channels; ch++){
		WebRtcNs_Analyze(m_handles[ch], capture_buffer_t->split_bands_const_f(ch)[kBand0To8kHz]);
		WebRtcNs_Process(m_handles[ch], capture_buffer_t->split_bands_const_f(ch),
						capture_buffer_t->num_bands(),
						capture_buffer_t->split_bands_f(ch));
	}
	if (m_frequency == 32000 || m_frequency == 48000){
		capture_buffer_t->MergeFrequencyBands();
	}

	//convert de-interleave data to interleave data
	interleave_data = data;
	for (int i = 0; i < input_channels; i++) {
		short *deinterleave_data = capture_buffer_t->data(i);
		int interleave_idx = i;
		for (int j = 0; j < samples_per_channel; j++) {
			interleave_data[interleave_idx] = S16ToFloat(deinterleave_data[j]);
			interleave_idx += input_channels;
		}
	}
}

void APM_NS::processCaptureStream(short* data, int samples_per_channel, int input_channels)
{
	if (!init_flag){
		return; // do not process data if did not initial
	}
	AudioBuffer *capture_buffer_t = (AudioBuffer *)capture_buffer;

	//convert interleave data to de-interleave data
	short *interleave_data = data;
	for (int i = 0; i < input_channels; i++) {
		short* deinterleave_data = capture_buffer_t->data(i);
		int interleaved_idx = i;
		for (int j = 0; j < samples_per_channel; j++) {
			deinterleave_data[j] = interleave_data[interleaved_idx];
			interleaved_idx += input_channels;
		}
	}

	if (m_frequency == 32000 || m_frequency == 48000){
		capture_buffer_t->SplitIntoFrequencyBands();
	}
	for (int ch = 0; ch < m_channels; ch++){
		WebRtcNs_Analyze(m_handles[ch], capture_buffer_t->split_bands_const_f(ch)[kBand0To8kHz]);
		WebRtcNs_Process(m_handles[ch], capture_buffer_t->split_bands_const_f(ch),
						capture_buffer_t->num_bands(),
						capture_buffer_t->split_bands_f(ch));
	}
	if (m_frequency == 32000 || m_frequency == 48000){
		capture_buffer_t->MergeFrequencyBands();
	}

	//convert de-interleave data to interleave data
	interleave_data = data;
	for (int i = 0; i < input_channels; i++) {
		short *deinterleave_data = capture_buffer_t->data(i);
		int interleave_idx = i;
		for (int j = 0; j < samples_per_channel; j++) {
			interleave_data[interleave_idx] = deinterleave_data[j];
			interleave_idx += input_channels;
		}
	}
}

APM_NS::~APM_NS(){
	for (int ch = 0; ch < m_channels; ch++) {
		WebRtcNs_Free(m_handles[ch]);
	}

	delete capture_buffer;
	delete [] channels_ptr_i;
}