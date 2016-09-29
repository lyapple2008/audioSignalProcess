#include "highpass_filter.h"

#define COEF_LEN 15
const float filterCoef[COEF_LEN] = {
	-0.04498110712, -0.008693917654, -0.009345070459, -0.009919825941, -0.01037237514,
	-0.01073910017, -0.01090841275, 0.989046514, -0.01090841275, -0.01073910017,
	-0.01037237514, -0.009919825941, -0.009345070459, -0.008693917654, -0.04498110712
};

#define INT16_MIN -32768
#define INT16_MAX 32767
#define FLOAT_MIN -1.0
#define FLOAT_MAX 1.0
#define KeepValueRange(Value, Min, Max) ((Value)<(Min)? (Min):((Value)>(Max)? (Max):(Value)))

void initFilter(FilterState_t *state)
{
	state->filterCoef = filterCoef;
	memset(state->w, 0, sizeof(short) * COEF_LEN);
}

void filter_int16(FilterState_t *state, short *data, int len)
{
	short *out = data;
	float sum = 0;

	for (int i = 0; i < len; i++) {
		state->w[0] = *data++;
		sum = 0;
		for (int j = 0; j < COEF_LEN; j++) {
			sum += state->w[j] * state->filterCoef[j];
		}
		*out++ = KeepValueRange(sum, INT16_MIN, INT16_MAX);
		// update state;
		for (int j = COEF_LEN-1; j > 0; j--) {
			state->w[j] = state->w[j - 1];
		}
	}
}

void filter_float(FilterState_t *state, float *data, int len)
{
	float *out = data;
	float sum = 0;

	for (int i = 0; i < len; i++) {
		state->w[0] = *data++;
		sum = 0;
		for (int j = 0; j < COEF_LEN; j++) {
			sum += state->w[j] * state->filterCoef[j];
		}
		*out++ = KeepValueRange(sum, FLOAT_MIN, FLOAT_MAX);
		// update state;
		for (int j = COEF_LEN - 1; j > 0; j--) {
			state->w[j] = state->w[j - 1];
		}
	}
}