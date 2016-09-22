#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../libapm/include/apm_ns.h"


int main(int argc, char* argv[])
{
	const char *infile = "C:\\MarshallPolyvWorkspace\\Projects\\POLYVMaster\\Debug\\Video\\{.raw";
	const char *outfile = "C:\\MarshallPolyvWorkspace\\Projects\\POLYVMaster\\Debug\\Video\\{_denoise.raw";
	FILE *fr, *fw;
	if (fopen_s(&fr, infile, "rb") ||
		fopen_s(&fw, outfile, "wb")){
		printf("Fail to open file!!!\n");
		return -1;
	}
	if (!fr || !fw)
	{
		printf("Can't open file!\n");
		return -1;
	}

	unsigned int frequency = 48000;
	unsigned short length = frequency / 100;
	short channels = 2;
	APM_NS ns_module;
	ns_module.initNsModule(frequency, Ns_Mode_Mideum, length, channels);
	float *input = new float[length*channels];

	memset(input, 0, length*channels*sizeof(float));

	int frm_cnt = 0;

	while (!feof(fr))
	{
		fread(input, sizeof(float), length*channels, fr);

		ns_module.processCaptureStream(input, length, channels);

		fwrite(input, sizeof(float), length*channels, fw);

		printf("Frame #%d\n", frm_cnt++);
	}

	fclose(fr);
	fclose(fw);

	printf("%s\n", infile);
	printf("%s\n", outfile);

	delete [] input;

	return 0;
}
