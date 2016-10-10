#include "audioDenoiseBlockTreshold.h"

#define _USE_MATH_DEFINES // to use macro M_PI in math.h
#include <math.h>

#define FORWARD_FFT 0
#define BACKWARD_FFT 1
#define SAFE_FREE(mem) do{if(!mem) free(mem);}while(0);

static void make_hanning_window(float *win, int32_t win_size)
{
	int32_t half_win = (win_size - 1) / 2;
	for (int32_t i = 0; i < half_win; i++) {
		win[i] = 0.5 - 0.5*cos(2*M_PI*i/(win_size-1));
		win[win_size - 1 - i] = win[i];
	}
}

int32_t blockThreshold_init(MarsBlockThreshold_t *handle,
							int32_t time_win, int32_t fs)
{
	if (time_win<=0 || fs<=0 || !handle) {
		return MARS_ERROR_PARAMS;
	}

	// Compute hanning window
	handle->win_size = fs / 1000 * time_win;
	if (handle->win_size % 2 != 0) {
		handle->win_size += 1;// even window
	}
	handle->half_win_size = (handle->win_size) / 2;
	handle->win_hanning = (float *)malloc(sizeof(float) * (handle->win_size));
	if (!(handle->win_hanning)) {
		return MARS_ERROR_MEMORY;
	}
	make_hanning_window(handle->win_hanning, handle->win_size);

	//Compute block params
	handle->max_nblk_time = 8;
	handle->max_nblk_freq = 15;
	handle->nblk_time = 3;
	handle->nblk_freq = 5;
	handle->sigma_noise = 0.047;	
	handle->macro_size = handle->half_win_size * handle->max_nblk_time;
	handle->have_nblk_time = 0;

	handle->inbuf = (float *)malloc(sizeof(float) * handle->win_size);
	if (!(handle->inbuf)) {
		SAFE_FREE(handle->win_hanning);
		return MARS_ERROR_MEMORY;
	}
	handle->inbuf_win = (kiss_fft_cpx *)malloc(sizeof(kiss_fft_cpx) * handle->win_size);
	if (!(handle->inbuf_win)) {
		SAFE_FREE(handle->win_hanning);
		SAFE_FREE(handle->inbuf);
		return MARS_ERROR_MEMORY;
	}
	for (int32_t i = 0; i < handle->win_size; i++) {
		(handle->inbuf_win)[i].i = 0.0;
		(handle->inbuf_win)[i].r = 0.0;
	}

	handle->outbuf = (float *)malloc(sizeof(float) * handle->macro_size);
	if (!(handle->outbuf)) {
		SAFE_FREE(handle->win_hanning);
		SAFE_FREE(handle->inbuf);
		SAFE_FREE(handle->inbuf_win);
		return MARS_ERROR_MEMORY;
	}
	handle->num_inbuf = 0;
	handle->output_ready = 0;

	handle->stft_coef = (kiss_fft_cpx **)malloc(sizeof(kiss_fft_cpx *) * (handle->max_nblk_time));
	if (!(handle->stft_coef)) {
		SAFE_FREE(handle->win_hanning);
		SAFE_FREE(handle->inbuf);
		SAFE_FREE(handle->outbuf);
		SAFE_FREE(handle->inbuf_win);
		return MARS_ERROR_MEMORY;
	}
	for (int32_t i = 0; i < handle->max_nblk_freq; i++) {
		handle->stft_coef[i] = (kiss_fft_cpx *)malloc(sizeof(kiss_fft_cpx) * (handle->win_size));
		//to-do: need check malloc result
	}

	handle->forward_fft_cfg = kiss_fft_alloc(handle->win_size, FORWARD_FFT, 0, 0);
	if (!(handle->forward_fft_cfg)) {
		SAFE_FREE(handle->win_hanning);
		SAFE_FREE(handle->inbuf);
		SAFE_FREE(handle->outbuf);
		SAFE_FREE(handle->inbuf_win);
		return MARS_ERROR_MEMORY;
	}
	handle->backward_fft_cfg = kiss_fft_alloc(handle->win_size, BACKWARD_FFT, 0, 0);
	if (!(handle->backward_fft_cfg)) {
		SAFE_FREE(handle->win_hanning);
		SAFE_FREE(handle->inbuf);
		SAFE_FREE(handle->outbuf);
		SAFE_FREE(handle->inbuf_win);
		SAFE_FREE(handle->forward_fft_cfg);
		return MARS_ERROR_MEMORY;
	}

	return MARS_OK;
}

// The conversion functions use the following naming convention:
// S16:      int16_t [-32768, 32767]
// Float:    float   [-1.0, 1.0]
// FloatS16: float   [-32768.0, 32767.0]
static int16_t FloatToS16(float v) {
	if (v > 0)
		return v >= 1 ? (INT16_MAX) :
		(int16_t)(v * (INT16_MAX) + 0.5f);
	return v <= -1 ? (INT16_MIN) : 
		(int16_t)(v * (INT16_MIN) - 0.5);
}

static float S16ToFloat(int16_t v) {
	static const float kMaxInt16Inverse = 1.f / (INT16_MAX);
	static const float kMinInt16Inverse = 1.f / (INT16_MIN);
	return v * (v > 0 ? kMaxInt16Inverse : -kMinInt16Inverse);
}

static void blockTreshold_STFT(MarsBlockThreshold_t *handle)
{
	//filter with window
	for (int32_t i = 0; i < handle->win_size; i++) {
		(handle->inbuf_win)[i].r *= (handle->win_hanning)[i];
	}

	kiss_fft(handle->forward_fft_cfg, handle->inbuf_win, 
			 handle->stft_coef[handle->have_nblk_time]);

	(handle->have_nblk_time)++;
}

static void blockThreshold_core(MarsBlockThreshold_t *handle)
{

}

int32_t blockThreshold_denoise(MarsBlockThreshold_t *handle,
								int32_t *in, int32_t in_len)
{
	if (in_len != handle->half_win_size) {
		return MARS_ERROR_PARAMS;
	}
	// Prepare inbuf
	int32_t half_win_size = handle->half_win_size;
	memcpy(handle->inbuf, handle->inbuf + half_win_size, sizeof(float) * half_win_size);
	memcpy(handle->inbuf + half_win_size, in, sizeof(float) * half_win_size);

	// do STFT
	blockThreshold_STFT(handle);

	// one macro block
	if (handle->have_nblk_time == handle->max_nblk_time) {

	}


}