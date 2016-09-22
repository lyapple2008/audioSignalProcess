#include <string.h>
#include "webrtc/modules/audio_processing/aec/include/echo_cancellation.h"
#include "webrtc/modules/interface/module_common_types.h"
#include "webrtc/modules/audio_processing/audio_buffer.h"
extern "C"{
#include "wav_io.h"
}

using namespace webrtc;

typedef void AecmHandle;

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		printf("Usage: pro.exe mic.wav speaker.wav aec_result.wav\n");
		return -1;
	}

	FILE *mic_file, *speaker_file, *result_file;
	if (fopen_s(&mic_file, argv[1], "rb") ||
		fopen_s(&speaker_file, argv[2], "rb") ||
		fopen_s(&result_file, argv[3], "wb")){
		printf("Fail to open file !!!\n");
		return -1;
	}
	WAV_HEADER mic_header, speaker_header;
	if (read_header(&mic_header, mic_file) != 0){
		printf("Fail to parse wav file: %s\n", argv[1]);
		return -1;
	}
	if (read_header(&speaker_header, speaker_file) != 0){
		printf("Fail to parse wav file: %s\n", argv[2]);
		return -1;
	}

	if (mic_header.format.bits_per_sample != 16 ||
		speaker_header.format.bits_per_sample != 16){
		printf("Now only support 16 bits per sample!\n");
		return -1;
	}
	assert(mic_header.format.sample_per_sec == speaker_header.format.sample_per_sec);
	assert(mic_header.format.channels == 1);
	assert(speaker_header.format.channels == 1);

	write_header(&mic_header, result_file);

	uint32_t frequency = mic_header.format.sample_per_sec;//16000;
	uint16_t length = frequency / 100;
	int16_t channels = mic_header.format.channels;

	int16_t *mic_buf = (int16_t*)new int16_t[length];
	int16_t *speaker_buf = (int16_t *)new int16_t[length];

	memset(mic_buf, 0, length*sizeof(int16_t));
	memset(speaker_buf, 0, length*sizeof(int16_t));

	AecmHandle *aecm_handle = NULL;
	WebRtcAec_Create(&aecm_handle);
	WebRtcAec_Init(aecm_handle, frequency, 48000);
	AudioFrame render_frame;
	AudioBuffer render_buffer(length, channels, length, channels, length);
	AudioFrame capture_frame;
	AudioBuffer capture_buffer(length, channels, length, channels, length);
	int32_t frm_cnt = 0;
	int32_t stream_delay_ms = 0;// 410;
	int32_t stream_drift_samples = 0;

	while (!feof(mic_file) && !feof(speaker_file)){
		read_samples(speaker_buf, length, &speaker_header, speaker_file);
		render_frame.UpdateFrame(0, 0, speaker_buf, length, frequency,
			AudioFrame::kNormalSpeech, AudioFrame::kVadUnknown, channels);
		render_buffer.DeinterleaveFrom(&render_frame);
		WebRtcAec_BufferFarend(aecm_handle, render_buffer.split_bands_const_f(0)[0],
			render_buffer.samples_per_split_channel());

		read_samples(mic_buf, length, &mic_header, mic_file);
		capture_frame.UpdateFrame(0, 0, mic_buf, length, frequency,
			AudioFrame::kNormalSpeech, AudioFrame::kVadUnknown, channels);
		capture_buffer.DeinterleaveFrom(&capture_frame);

		WebRtcAec_Process(aecm_handle, capture_buffer.split_bands_const_f(0),
			capture_buffer.num_bands(),
			capture_buffer.split_bands_f(0),
			capture_buffer.samples_per_split_channel(),
			stream_delay_ms,
			stream_drift_samples);

		capture_buffer.InterleaveTo(&capture_frame, true);

		write_samples(capture_frame.data_, length, &mic_header, result_file);

		printf("Frame #%d\n", frm_cnt++);
	}

	WebRtcAec_Free(aecm_handle);

	fclose(mic_file);
	fclose(speaker_file);
	fclose(result_file);
	delete[]mic_buf;
	delete[]speaker_buf;

	return 0;
}

