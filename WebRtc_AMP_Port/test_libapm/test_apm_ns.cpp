#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../finalVersion/include/apm_ns.h"
#include "../webrtc/typedefs.h"
extern "C"{
#include "../wav_io.h"
//test
}

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
	APM_NS ns_module;
	ns_module.initNsModule(frequency, Ns_Mode_Mideum, length, channels);
	int16_t *input = (int16_t *)new short[length];
	int16_t *output = (int16_t *)new short[length];

	memset(input, 0, length*sizeof(int16_t));
	memset(output, 0, length*sizeof(int16_t));

	int32_t frm_cnt = 0;

	while (!feof(fr))
	{
		read_samples(input, length, &header, fr);

		ns_module.processCaptureStream(input, length, channels);

		write_samples(input, length, &header, fw);

		printf("Frame #%d\n", frm_cnt++);
	}

	fclose(fr);
	fclose(fw);

	delete[]input;
	delete[]output;

	return 0;
}
