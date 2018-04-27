#include "../include/apm_ns.h"
#include "webrtc/modules/audio_processing/ns/include/noise_suppression.h"
#include "webrtc/modules/audio_processing/agc/legacy/gain_control.h"
#include "webrtc/modules/audio_processing/audio_buffer.h"

extern "C"{
#include "../../rnnoise/rnnoise.h"
}

using namespace webrtc;

bool APM_NS::initNsModule(unsigned int frequency, int ns_mode, 
                          int input_frames, int input_channels,
                          bool doAgc)
{
	m_frequency = frequency;
	m_channels = input_channels;
	m_ns_mode = ns_mode;
    m_doAgc = doAgc;

	if (m_channels < 0) {
		return false;
	}
	m_nsHandles.clear();
    if (doAgc) {
        m_agcHandles.clear();
    }

	for (int i = 0; i < m_channels; i++) {
        if (doAgc) {
            ApmAgcHandle *agcHandle = NULL;
            if (WebRtcAgc_Create(&agcHandle)) {
                return false;
            }
            if (WebRtcAgc_Init(agcHandle, 0, 255, kAgcAdaptiveDigital, frequency)) {
                return false;
            }

            WebRtcAgcConfig agcConfig;
            agcConfig.limiterEnable = 1;
            agcConfig.compressionGaindB = 20;
            agcConfig.targetLevelDbfs = 3;
            if (WebRtcAgc_set_config(agcHandle, agcConfig)) {
                return false;
            }

            m_agcHandles.push_back(agcHandle);
            m_captureLevel.push_back(0);
        }

		ApmNsHandle *nsHandle = NULL;
		if (WebRtcNs_Create(&nsHandle)){
			return false;
		}
		if (WebRtcNs_Init(nsHandle, frequency)){
			return false;
		}
		if (WebRtcNs_set_policy(nsHandle, ns_mode)){
			return false;
		}
		m_nsHandles.push_back(nsHandle);
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

	return true;
}

void APM_NS::processCaptureStream(float* data, int samples_per_channel, int input_channels)
{
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

    int32_t captureLevel = 0;
    uint8_t stream_is_saturated = 0;
	for (int ch = 0; ch < m_channels; ch++){
        if (m_doAgc) {
            WebRtcAgc_Process(
                m_agcHandles[ch],
                capture_buffer_t->split_bands_const(ch),
                capture_buffer_t->num_bands(),
                static_cast<int16_t>(capture_buffer_t->samples_per_split_channel()),
                capture_buffer_t->split_bands(ch),
                m_captureLevel[ch],
                &captureLevel,
                0,
                &stream_is_saturated);
            // update capture level for next process
            m_captureLevel[ch] = captureLevel;
        }
		WebRtcNs_Analyze(m_nsHandles[ch], capture_buffer_t->split_bands_const_f(ch)[kBand0To8kHz]);
		WebRtcNs_Process(m_nsHandles[ch], capture_buffer_t->split_bands_const_f(ch),
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

    int32_t captureLevel = 0;
    uint8_t stream_is_saturated = 0;
	for (int ch = 0; ch < m_channels; ch++){
        if (m_doAgc) {
            WebRtcAgc_Process(
                m_agcHandles[ch],
                capture_buffer_t->split_bands_const(ch),
                capture_buffer_t->num_bands(),
                static_cast<int16_t>(capture_buffer_t->samples_per_split_channel()),
                capture_buffer_t->split_bands(ch),
                m_captureLevel[ch],
                &captureLevel,
                0,
                &stream_is_saturated);

            // update capture level for next process
            m_captureLevel[ch] = captureLevel;
        }
		WebRtcNs_Analyze(m_nsHandles[ch], capture_buffer_t->split_bands_const_f(ch)[kBand0To8kHz]);
		WebRtcNs_Process(m_nsHandles[ch], capture_buffer_t->split_bands_const_f(ch),
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

bool APM_NS::initRnnoiseModule(unsigned int frequency, int input_frames, int input_channels, bool doAgc)
{
    if (frequency != 48000 || 
        input_frames != 480) {
        return false;
    }

    if (!capture_buffer) {
        capture_buffer = new AudioBuffer(input_frames, input_channels,
                                        input_frames, input_channels,
                                        input_frames);
        if (!capture_buffer){
            return false;
        }
    }

    m_frequency = frequency;
    m_channels = input_channels;
    m_doAgc = doAgc;
    for (int i = 0; i < input_channels; i++) {
        if (doAgc) {
            ApmAgcHandle *agcHandle = NULL;
            if (WebRtcAgc_Create(&agcHandle)) {
                return false;
            }
            if (WebRtcAgc_Init(agcHandle, 0, 255, kAgcAdaptiveDigital, frequency)) {
                return false;
            }

            WebRtcAgcConfig agcConfig;
            agcConfig.limiterEnable = 1;
            agcConfig.compressionGaindB = 20;
            agcConfig.targetLevelDbfs = 3;
            if (WebRtcAgc_set_config(agcHandle, agcConfig)) {
                return false;
            }

            m_agcHandles.push_back(agcHandle);
            m_captureLevel.push_back(0);
        }

        DenoiseState* handle = rnnoise_create();
        m_rnnoiseHandles.push_back(handle);
    }

    return true;
}

#define RNN_FRAME_SIZE  480
void APM_NS::processUseRnnoise(short* data, int samples_per_channel, int input_channels)
{
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

    if (m_doAgc) {
        if (m_frequency == 32000 || m_frequency == 48000){
            capture_buffer_t->SplitIntoFrequencyBands();
        }
    }

    int32_t captureLevel = 0;
    uint8_t stream_is_saturated = 0;
    for (int ch = 0; ch < input_channels; ch++){
        if (m_doAgc) {
            WebRtcAgc_Process(
                m_agcHandles[ch],
                capture_buffer_t->split_bands_const(ch),
                capture_buffer_t->num_bands(),
                static_cast<int16_t>(capture_buffer_t->samples_per_split_channel()),
                capture_buffer_t->split_bands(ch),
                m_captureLevel[ch],
                &captureLevel,
                0,
                &stream_is_saturated);

            // update capture level for next process
            m_captureLevel[ch] = captureLevel;
        }

        // do denoise use rnnoise
        short* inData = capture_buffer_t->data(ch);
        for (int i = 0; i < RNN_FRAME_SIZE; i++) {
            rnnBuf[i] = inData[i] * 1.0;
        }
        rnnoise_process_frame(m_rnnoiseHandles[ch], rnnBuf, rnnBuf);
        for (int i = 0; i < RNN_FRAME_SIZE; i++) {
            inData[i] = rnnBuf[i];
        }
    }

    if (m_doAgc) {
        if (m_frequency == 32000 || m_frequency == 48000){
            capture_buffer_t->MergeFrequencyBands();
        }
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
    for (int ch = 0; ch < m_nsHandles.size(); ch++) {
        WebRtcNs_Free(m_nsHandles[ch]);
    }
    
    for (int ch = 0; ch < m_agcHandles.size(); ch++) {
        WebRtcAgc_Free(m_agcHandles[ch]);
    }

    for (int ch = 0; ch < m_rnnoiseHandles.size(); ch++) {
        rnnoise_destroy(m_rnnoiseHandles[ch]);
    }

	delete capture_buffer;
	delete [] channels_ptr_i;
}