#include <string.h>
#ifdef NS_FIXED
#include "webrtc/modules/audio_processing/ns/include/noise_suppression_x.h"
#else
#include "webrtc/modules/audio_processing/ns/include/noise_suppression.h"
#endif
#include "webrtc/modules/interface/module_common_types.h"
#include "webrtc/modules/audio_processing/audio_buffer.h"
extern "C"{
#include "wav_io.h"
}

using namespace webrtc;

enum Level
{
	kLow,
	kModerate,
	kHigh,
	kVeryHigh
};

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("Please input parameters\n");
		return -1;
	}

	printf("Process %s -> %s\n", argv[1], argv[2]);
	FILE *fr, *fw;
	if (fopen_s(&fr, argv[1], "rb") ||
		fopen_s(&fw, argv[2], "wb")){
		printf("Fail to open file!!!\n");
		return -1;
	}
	if (!fr || !fw)
	{
		printf("Can't open file!\n");
		return -1;
	}

	WAV_HEADER header;
	if (read_header(&header, fr) != 0){
		printf("Fail to read wav header!\n");
		return -1;
	}
	else{
		print_header(&header);
		write_header(&header, fw);
	}

	if (header.format.bits_per_sample != 16){
		printf("Now only support 16 bits per sample!\n");
		return -1;
	}

	uint32_t frequency = header.format.sample_per_sec;//16000;
	uint16_t length = frequency / 100;
	int16_t channels = header.format.channels;
#ifdef NS_FIXED
	NsxHandle* handle = NULL;
	WebRtcNsx_Create(&handle);
	WebRtcNsx_Init(handle, frequency);
	WebRtcNsx_set_policy(handle, kModerate);
#else
	NsHandle *handle = NULL;
	WebRtcNs_Create(&handle);
	WebRtcNs_Init(handle, frequency);
	WebRtcNs_set_policy(handle, kModerate);
#endif
	int16_t *input = (int16_t *)new short[length];
	int16_t *output = (int16_t *)new short[length];

	memset(input, 0, length*sizeof(int16_t));
	memset(output, 0, length*sizeof(int16_t));

	AudioFrame capture_frame;
	AudioBuffer capture_buffer(length, channels, length, channels, length);
	int32_t frm_cnt = 0;

	while (!feof(fr))
	{
		//fread(input, sizeof(int16_t), length, fr);
		read_samples(input, length, &header, fr);
		capture_frame.UpdateFrame(0, 0, input, length, frequency,
			AudioFrame::kNormalSpeech, AudioFrame::kVadUnknown, channels);
		capture_buffer.DeinterleaveFrom(&capture_frame);
#ifdef NS_FIXED
		WebRtcNsx_Process(handle, capture_buffer.split_bands_const(0),
				capture_buffer.num_bands(), capture_buffer.split_bands(0));
#else
		if (frequency == 32000 || frequency == 48000){
			capture_buffer.SplitIntoFrequencyBands();
		}
		WebRtcNs_Analyze(handle, capture_buffer.split_bands_const_f(0)[0]);
		WebRtcNs_Process(handle, capture_buffer.split_bands_const_f(0),
				capture_buffer.num_bands(), capture_buffer.split_bands_f(0));
		if (frequency == 32000 || frequency == 48000){
			capture_buffer.MergeFrequencyBands();
		}
#endif
		//fwrite(output, sizeof(int16_t), length, fw);
		capture_buffer.InterleaveTo(&capture_frame, true);
		write_samples(capture_frame.data_, length, &header, fw);

		printf("Frame #%d\n", frm_cnt++);
	}
#ifdef NS_FIXED
	WebRtcNsx_Free(handle);
#else
	WebRtcNs_Free(handle);
#endif
	fclose(fr);
	fclose(fw);

	delete[]input;
	delete[]output;

	return 0;
}

