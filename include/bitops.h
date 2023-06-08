#pragma once

#define BIT(i) (1 << i)

#define set_bit(x, bit) (x |= bit)
#define clear_bit(x, bit) (x &= ~bit)
#define is_bit_set(x, bit) (x & bit)

static inline int fls(unsigned int x)
{
	int r = 0;

	if (!x)
		return 0;

	if (x & 0xffff0000) {
		x >>= 16;
		r += 16;
	}

	if (x & 0xff00) {
		x >>= 8;
		r += 8;
	}

	if (x & 0xf0) {
		x >>= 4;
		r += 4;
	}

	if (x & 0xc) {
		x >>= 2;
		r += 2;
	}

	if (x & 0x2) {
		x >>= 1;
		r += 1;
	}

	if (x & 0x1) {
		r += 1;
	}

	return r;
}

