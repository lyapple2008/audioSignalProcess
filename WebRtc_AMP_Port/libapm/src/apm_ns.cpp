#include "../include/apm_ns.h"
#include "webrtc/modules/audio_processing/ns/include/noise_suppression.h"
#include "webrtc/modules/audio_processing/audio_buffer.h"

using namespace webrtc;

bool APM_NS::initNsModule(unsigned int frequency, int ns_mode, int input_frames, int input_channels)
{
	m_frequency = frequency;
	m_channels = input_channels;
	m_ns_mode = ns_mode;

	if (WebRtcNs_Create(&ns_handle)){
		return false;
	}
	if (WebRtcNs_Init(ns_handle, frequency)){
		return false;
	}
	if (WebRtcNs_set_policy(ns_handle, ns_mode)){
		return false;
	}

	capture_buffer = new AudioBuffer(input_frames, input_channels,
									 input_frames, input_channels,
									 input_frames);
	if (!capture_buffer){
		return false;
	}

	channels_ptr_f = new float*[input_channels];
	if (!channels_ptr_f){
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
	for (int ch = 0; ch < input_channels; ch++){
		channels_ptr_f[ch] = capture_buffer_t->data_f(ch);
	}
	float *interleave_data = data;
	for (int i = 0; i < samples_per_channel; i++){
		for (int ch = 0; ch < input_channels; ch++){
			*(channels_ptr_f[ch]) = FloatToFloatS16(*interleave_data);//*interleave_data;
			channels_ptr_f[ch]++;
			interleave_data++;
		}
	}

	for (int ch = 0; ch < input_channels; ch++){
		capture_buffer_t->data(ch);
	}

	if (m_frequency == 32000 || m_frequency == 48000){
		capture_buffer_t->SplitIntoFrequencyBands();
	}
	for (int ch = 0; ch < m_channels; ch++){
		WebRtcNs_Analyze(ns_handle, capture_buffer_t->split_bands_const_f(ch)[kBand0To8kHz]);
		WebRtcNs_Process(ns_handle, capture_buffer_t->split_bands_const_f(ch),
			capture_buffer_t->num_bands(),
			capture_buffer_t->split_bands_f(ch));
	}
	if (m_frequency == 32000 || m_frequency == 48000){
		capture_buffer_t->MergeFrequencyBands();
	}

	//convert de-interleave data to interleave data
	for (int ch = 0; ch < input_channels; ch++){
		channels_ptr_f[ch] = capture_buffer_t->data_f(ch);
	}
	interleave_data = data;
	for (int i = 0; i < samples_per_channel; i++){
		for (int ch = 0; ch < input_channels; ch++){
			*interleave_data = FloatS16ToFloat(*(channels_ptr_f[ch]));//*(channels_ptr_f[ch]);
			channels_ptr_f[ch]++;
			interleave_data++;
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
	for (int ch = 0; ch < input_channels; ch++){
		channels_ptr_i[ch] = capture_buffer_t->data(ch);
	}
	short *interleave_data = data;
	for (int i = 0; i < samples_per_channel; i++){
		for (int ch = 0; ch < input_channels; ch++){
			*(channels_ptr_i[ch]) = *interleave_data;
			channels_ptr_i[ch]++;
			interleave_data++;
		}
	}

	if (m_frequency == 32000 || m_frequency == 48000){
		capture_buffer_t->SplitIntoFrequencyBands();
	}
	for (int ch = 0; ch < m_channels; ch++){
		WebRtcNs_Analyze(ns_handle, capture_buffer_t->split_bands_const_f(ch)[kBand0To8kHz]);
		WebRtcNs_Process(ns_handle, capture_buffer_t->split_bands_const_f(ch),
			capture_buffer_t->num_bands(),
			capture_buffer_t->split_bands_f(ch));
	}
	if (m_frequency == 32000 || m_frequency == 48000){
		capture_buffer_t->MergeFrequencyBands();
	}

	//convert de-interleave data to interleave data
	for (int ch = 0; ch < input_channels; ch++){
		channels_ptr_i[ch] = capture_buffer_t->data(ch);
	}
	interleave_data = data;
	for (int i = 0; i < samples_per_channel; i++){
		for (int ch = 0; ch < input_channels; ch++){
			*interleave_data = *(channels_ptr_i[ch]);
			channels_ptr_i[ch]++;
			interleave_data++;
		}
	}
}

APM_NS::~APM_NS(){
	WebRtcNs_Free(ns_handle);
	delete capture_buffer;
	delete channels_ptr_f;
	delete channels_ptr_i;
}