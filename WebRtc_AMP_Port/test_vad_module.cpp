#include <string.h>
#include "webrtc/common_audio/vad/include/webrtc_vad.h"
#include "webrtc/modules/interface/module_common_types.h"
#include "webrtc/modules/audio_processing/audio_buffer.h"
extern "C"{
#include "wav_io.h"
}

using namespace webrtc;

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		printf("Usage: pro.exe mic.wav vad_result.wav\n");
		return -1;
	}

	FILE *mic_file, *result_file;
	if (fopen_s(&mic_file, argv[1], "rb") ||
		fopen_s(&result_file, argv[2], "wb")){
		printf("Fail to open file !!!\n");
		return -1;
	}
	WAV_HEADER mic_header;
	if (read_header(&mic_header, mic_file) != 0){
		printf("Fail to parse wav file: %s\n", argv[1]);
		return -1;
	}

	if (mic_header.format.bits_per_sample != 16){
		printf("Now only support 16 bits per sample!\n");
		return -1;
	}
	assert(mic_header.format.channels == 1);

	write_header(&mic_header, result_file);

	uint32_t frequency = mic_header.format.sample_per_sec;//16000;
	uint16_t length = frequency / 100;
	int16_t channels = mic_header.format.channels;

	int16_t *mic_buf = (int16_t*)new int16_t[length];
	int16_t *result_buf = (int16_t *)new int16_t[length];

	memset(mic_buf, 0, length*sizeof(int16_t));
	memset(result_buf, 0, length*sizeof(int16_t));

	VadInst *vad_handle = NULL;
	WebRtcVad_Create(&vad_handle);
	WebRtcVad_Init(vad_handle);
	int32_t vad_mode = 2;//aggressive mode
	WebRtcVad_set_mode(vad_handle, vad_mode);
	AudioFrame capture_frame;
	AudioBuffer capture_buffer(length, channels, length, channels, length);
	int32_t frm_cnt = 0;

	while (!feof(mic_file)){
		read_samples(mic_buf, length, &mic_header, mic_file);
		int vad_ret = WebRtcVad_Process(vad_handle, frequency, 
								mic_buf, length);

		if (vad_ret == 1){
			for (int i = 0; i < length; i++){
				result_buf[i] = 16383;
			}
		}else if(vad_ret == 0){
			for (int i = 0; i < length; i++){
				result_buf[i] = -16383;
			}
		}else{
			memset(result_buf, 0, sizeof(int16_t)*length);
		}
		write_samples(result_buf, length, &mic_header, result_file);

		printf("Frame #%d\n", frm_cnt++);
	}

	WebRtcVad_Free(vad_handle);

	fclose(mic_file);
	fclose(result_file);
	delete[]mic_buf;

	return 0;
}

