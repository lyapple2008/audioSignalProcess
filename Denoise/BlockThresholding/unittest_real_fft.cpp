#include <stdint.h>
#include <math.h>
#include "../../common/kiss_fft/kiss_fftr.h"

#define FORWARD_FFT 0
#define BACKWARD_FFT 1

void saveResult_real(kiss_fft_scalar *result, int32_t len, const char *outfile)
{
	FILE *pOutFile;
	fopen_s(&pOutFile, outfile, "w");

	for (int32_t i = 0; i < len; i++) {
		fprintf(pOutFile, "%f\n", result[i]);
	}

	fclose(pOutFile);
}

int main(int argc, char *argv[])
{
	int32_t len = 552;
	kiss_fft_scalar *fin = (kiss_fft_scalar *)malloc(len * sizeof(kiss_fft_scalar));
	kiss_fft_cpx *fout = (kiss_fft_cpx *)malloc(len * sizeof(kiss_fft_cpx));
	if (!fin || !fout) {
		fprintf(stderr, "error: malloc");
		return -1;
	}
	for (int32_t i = 0; i < len; i++) {
		fin[i] = sin(i*1.0);
		fout[i].r = 0.0;
		fout[i].i = 0.0;
	}

	saveResult_real(fin, len, "init_file.txt");

	kiss_fftr_cfg fftr_cfg = kiss_fftr_alloc(len, FORWARD_FFT, 0, 0);

	kiss_fftr(fftr_cfg, fin, fout);

	// inverse fft
	fprintf(stdout, "do inverse fft!!!\n");
	kiss_fftr_cfg inv_fftr_cfg = kiss_fftr_alloc(len, BACKWARD_FFT, 0, 0);
	kiss_fftri(inv_fftr_cfg, fout, fin);
	for (int32_t i = 0; i < len; i++) {
		fin[i] = fin[i] / len;
	}
	saveResult_real(fin, len, "kiss_inv_fft_result.txt");
	fprintf(stdout, "finish!\n");

	free(fin);
	free(fout);
	free(fftr_cfg);
	free(inv_fftr_cfg);

	return 0;
}