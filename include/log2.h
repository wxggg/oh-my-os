#pragma once

#include <types.h>
#include <bitops.h>

static inline unsigned long ilog2_roundup(unsigned long n)
{
	if (n == 0)
		return 0;

	return fls(n - 1);
}

static inline unsigned long ilog2_rounddown(unsigned long n)
{
	return fls(n) - 1;
}

#define ilog2(n) ilog2_rounddown(n)

static inline bool is_power_of_2(unsigned long n)
{
	return (n != 0 && ((n & (n - 1)) == 0));
}

static inline unsigned long roundup_pow_of_two(unsigned long n)
{
	return 1UL << ilog2_roundup(n);
}

static inline unsigned long rounddown_pow_of_two(unsigned long n)
{
	return 1UL << ilog2_rounddown(n);
}
