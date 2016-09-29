#ifndef MARFILTER_H
#define MARFILTER_H

struct FilterState{
	float w[15];
	float *filterCoef;
};

typedef struct FilterState FilterState_t;

void initFilter(FilterState_t *state);

void filter_int16(FilterState_t *state, short *data, int len);

void filter_float(FilterState_t *state, float *data, int len);

#endif
