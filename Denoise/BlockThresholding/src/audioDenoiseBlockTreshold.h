#ifndef AUDIO_DENOISE_BLOCK_THRESHOLD_H_
#define AUDIO_DENOISE_BLOCK_THRESHOLD_H_

#include <stdint.h>
#include "../../../common/kiss_fft/kiss_fftr.h"

#define MARS_OK                 0x00
#define MARS_ERROR_MEMORY       0x01
#define MARS_ERROR_PARAMS       0x02
#define MARS_NEED_MORE_SAMPLES  0x10
#define MARS_CAN_OUTPUT         0x20

typedef struct MarsBlockThreshold{
    int32_t win_size;    // window size--odd window
    int32_t half_win_size; // half window size
    float *win_hanning; // hanning window

    int32_t max_nblk_time;
    int32_t max_nblk_freq;
    int32_t nblk_time;  // the number of block in time dimension
    int32_t nblk_freq;  // the number of block in frequency dimension
    int32_t macro_size; // the number of sample in one macro block
    int32_t have_nblk_time;
    float **SURE_matrix;

    float sigma_noise;  // assumption the sigma of gaussian white noise
    float sigma_hanning_noise;
    float *inbuf;       // internal buffer for keep one window size input samples
    float *inbuf_win;   
    float *outbuf;      // internal buffer for keep one macro block output samples

    kiss_fft_cpx **stft_coef;
    kiss_fft_cpx **stft_thre;
    kiss_fft_cpx **stft_coef_block;
    kiss_fft_cpx **stft_coef_block_norm;
    kiss_fftr_cfg forward_fftr_cfg;
    kiss_fftr_cfg backward_fftr_cfg;
}MarsBlockThreshold_t;

/*
 * time_win: ms
 * fs: sample rate
 */
int32_t blockThreshold_init(MarsBlockThreshold_t *handle,
                            int32_t time_win, int32_t fs);

int32_t blockThreshold_reset(MarsBlockThreshold_t *handle);

int32_t blockThreshold_denoise_int16(MarsBlockThreshold_t *handle,
                                int16_t *in, int32_t in_len);
int32_t blockThreshold_denoise_float(MarsBlockThreshold_t *handle,
                                float *in, int32_t in_len);
/*
 * out[in]: output buffer
 * out_len[in]: the length of the output buffer
 * return: the numbers of output samples(int16_t)
 */
int32_t blockThreshold_output_int16(MarsBlockThreshold_t *handle,
                            int16_t *out, int32_t out_len);
int32_t blockThreshold_output_float(MarsBlockThreshold_t *handle,
                            float *out, int32_t out_len);

int32_t blockThreshold_flush_int16(MarsBlockThreshold_t *handle, 
                        int16_t *out, int32_t out_len);
int32_t blockThreshold_flush_float(MarsBlockThreshold_t *handle,
                        float *out, int32_t out_len);

void blockThreshold_free(MarsBlockThreshold_t *handle);

// compute the output length of one MarcroBlock
int32_t blockThreshold_max_output(const MarsBlockThreshold_t *handle);

int32_t blockThreshold_samples_per_time(const MarsBlockThreshold_t *handle);

#endif