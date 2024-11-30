#pragma once

#include <types.h>

#define max(x, y) (x) > (y) ? (x) : (y)
#define min(x, y) (x) < (y) ? (x) : (y)

#define abs(x) (x) > 0 ? (x) : (-(x))
#define pow2(x) ((x) * (x))
#define pow3(x) ((x) * (x) * (x))

static inline int pow(int x, int e) __attribute__((always_inline));
static inline double powlf(double x, int e) __attribute__((always_inline));
static inline int log2n(size_t n) __attribute__((always_inline));
//static inline int log2n_int(size_t n) __attribute__((always_inline));

static inline int pow(int x, int e)
{
	int ret = 1;
	for (int i = 0; i < e; i++)
		ret *= x;

	return ret;
}

static inline double powlf(double x, int e)
{
	double ret = 1.0;
	for (int i = 0; i < e; i++)
		ret *= x;

	return ret;
}

static inline int log2n(size_t n)
{
	int i = 0, size_i = 1;
	while (size_i <= n) {
		i++;
		size_i <<= 1;
	}
	return i - 1;
}

int rand(void);