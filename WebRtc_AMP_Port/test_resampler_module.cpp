#include <stdlib.h>
#include <string.h>
#include "webrtc/common_audio/resampler/include/resampler.h"
extern "C"{
#include "wav_io.h"
}

using namespace webrtc;


int main(int argc, char* argv[])
{
	if (argc < 4)
	{
		printf("Usage: pro.exe in_file out_file out_rate\n");
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
		header.format.sample_per_sec = atoi(argv[3]);
		write_header(&header, fw);
	}

	if (header.format.bits_per_sample != 16){
		printf("Now only support 16 bits per sample!\n");
		return -1;
	}

	uint32_t frequency = header.format.sample_per_sec;//16000;
	uint16_t length = frequency / 100;
	int16_t channels = header.format.channels;

	Resampler resampler(frequency, atoi(argv[3]), kResamplerSynchronous);

	int16_t *input = (int16_t *)new short[length];
	int16_t *output = (int16_t *)new short[4*length];

	memset(input, 0, length*sizeof(int16_t));
	memset(output, 0, length*sizeof(int16_t));

	int32_t frm_cnt = 0;
	int32_t out_len = 0;

	while (!feof(fr))
	{
		//fread(input, sizeof(int16_t), length, fr);
		read_samples(input, length, &header, fr);
		
		resampler.Push(input, length, output, 4 * length, out_len);

		write_samples(output, out_len, &header, fw);

		printf("Frame #%d\n", frm_cnt++);
	}

	fclose(fr);
	fclose(fw);

	delete[]input;
	delete[]output;

	return 0;
}

