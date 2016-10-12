#include <stdint.h>
#include <math.h>
#include "../../common/kiss_fft/kiss_fft.h"

#define FORWARD_FFT 0
#define BACKWARD_FFT 1

void saveResult(kiss_fft_cpx *result, int32_t len, const char *outfile)
{
	FILE *pOutFile;
	fopen_s(&pOutFile, outfile, "w");
	
	for (int32_t i = 0; i < len; i++) {
		fprintf(pOutFile, "%f\t%f\n", result[i].r, result[i].i);
	}

	fclose(pOutFile);
}

int main(int argc, char *argv[])
{
	int32_t len = 551;
	kiss_fft_cpx *fin = (kiss_fft_cpx *)malloc(len * sizeof(kiss_fft_cpx));
	kiss_fft_cpx *fout = (kiss_fft_cpx *)malloc(len * sizeof(kiss_fft_cpx));
	if (!fin || !fout) {
		fprintf(stderr, "error: malloc");
		return -1;
	}
	for (int32_t i = 0; i < len; i++) {
		fin[i].r = sin(i*1.0);
		fin[i].i = 0.0;
		fout[i].r = 0.0;
		fout[i].i = 0.0;
	}

	saveResult(fin, len, "init_file.txt");

	kiss_fft_cfg fft_cfg = kiss_fft_alloc(len, FORWARD_FFT, 0, 0);

	fprintf(stdout, "do fft!!!\n");
	kiss_fft(fft_cfg, fin, fout);
	saveResult(fout, len, "kiss_fft_result.txt");
	fprintf(stdout, "finish!\n");

	// inverse fft
	fprintf(stdout, "do inverse fft!!!\n");
	kiss_fft_cfg inv_fft_cfg = kiss_fft_alloc(len, BACKWARD_FFT, 0, 0);
	kiss_fft(inv_fft_cfg, fout, fin);
	saveResult(fin, len, "kiss_inv_fft_result.txt");
	fprintf(stdout, "finish!\n");

	free(fin);
	free(fout);
	free(fft_cfg);
	free(inv_fft_cfg);

	return 0;
}