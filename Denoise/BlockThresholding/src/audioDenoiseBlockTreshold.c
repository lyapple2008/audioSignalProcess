#include "audioDenoiseBlockTreshold.h"


int32_t blockThreshold_init(MarsBlockThreshold_t *handle,
							int32_t time_win, int32_t fs)
{
	if (time_win<=0 || fs<=0 || !handle) {
		return MARS_ERROR_PARAMS;
	}

	handle->win_size = fs / 1000 * time_win;
	handle->half_win_size = handle->win_size / 2;
	handle->max_nblk_time = 8;
	handle->max_nblk_freq = 15;
	handle->nblk_time = 3;
	handle->nblk_freq = 5;
	handle->sigma_noise = 0.047;
	
	handle->macro_size = handle->half_win_size * handle->max_nblk_time;
	handle->inbuf = (float *)malloc(sizeof(float) * handle->macro_size);
	if (!(handle->inbuf)) {
		return MARS_ERROR_MEMORY;
	}
	handle->num_inbuf = 0;
	handle->output_ready = 0;


	return MARS_OK;
}

// The conversion functions use the following naming convention:
// S16:      int16_t [-32768, 32767]
// Float:    float   [-1.0, 1.0]
// FloatS16: float   [-32768.0, 32767.0]
static inline int16_t FloatToS16(float v) {
	if (v > 0)
		return v >= 1 ? (INT16_MAX) :
		(int16_t)(v * (INT16_MAX) + 0.5f);
	return v <= -1 ? (INT16_MIN) : 
		(int16_t)(v * (INT16_MIN) - 0.5);
}

static inline float S16ToFloat(int16_t v) {
	static const float kMaxInt16Inverse = 1.f / (INT16_MAX);
	static const float kMinInt16Inverse = 1.f / (INT16_MIN);
	return v * (v > 0 ? kMaxInt16Inverse : -kMinInt16Inverse);
}

int32_t blockThreshold_denoise(MarsBlockThreshold_t *handle,
								int32_t *in, int32_t in_len)
{
	// prepare in samples
	int32_t need_samples = handle->macro_size - handle->num_inbuf;
	int32_t leave_samples = in_len;
	float *p_inbuf = handle->inbuf;
	if (in_len < need_samples) {
		for (int32_t i = 0; i < in_len; i++) {
			p_inbuf[handle->num_inbuf + i] = S16ToFloat(*in);
			in++;
			handle->num_inbuf++;
		}
		leave_samples = 0;
	} else {
		for (int32_t i = 0; i < need_samples; i++) {
			p_inbuf[handle->num_inbuf + i] = S16ToFloat(*in);
			in++;
			handle->num_inbuf++;
			leave_samples--;
		}
	}

	// denoise


	// save leave samples
	if (leave_samples > 0) {
		handle->num_inbuf = 0;
		for (int32_t i = 0; i < leave_samples; i++) {
			p_inbuf[i] = *in;
			in++;
		}
	}

}