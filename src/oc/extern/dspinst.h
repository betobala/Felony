/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Portable C++ versions of the Cortex-M4 DSP instructions, for the virtual O&C (see
 * tools/port.py). Same results as the assembly in the original file.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 */

#ifndef dspinst_h_
#define dspinst_h_

#include <stdint.h>

static inline int32_t dsp_ssat(int64_t v, int bits) {
	const int64_t hi = (int64_t(1) << (bits - 1)) - 1, lo = -(int64_t(1) << (bits - 1));
	return (int32_t) (v > hi ? hi : (v < lo ? lo : v));
}
static inline int32_t dsp_lo16(uint32_t v) { return (int16_t) (v & 0xFFFF); }
static inline int32_t dsp_hi16(uint32_t v) { return (int16_t) (v >> 16); }

// computes limit((val >> rshift), 2**bits)
static inline int32_t signed_saturate_rshift(int32_t val, int bits, int rshift) {
	return dsp_ssat(val >> rshift, bits);
}

// computes limit(val, 2**bits)
static inline int16_t saturate16(int32_t val) {
	return (int16_t) dsp_ssat(val, 16);
}

// computes ((a[31:0] * b[15:0]) >> 16)
static inline int32_t signed_multiply_32x16b(int32_t a, uint32_t b) {
	return (int32_t) (((int64_t) a * dsp_lo16(b)) >> 16);
}

// computes ((a[31:0] * b[31:16]) >> 16)
static inline int32_t signed_multiply_32x16t(int32_t a, uint32_t b) {
	return (int32_t) (((int64_t) a * dsp_hi16(b)) >> 16);
}

// computes (((int64_t)a[31:0] * (int64_t)b[31:0]) >> 32)
static inline int32_t multiply_32x32_rshift32(int32_t a, int32_t b) {
	return (int32_t) (((int64_t) a * b) >> 32);
}

// computes (((uint64_t)a[31:0] * (uint64_t)b[31:0]) >> 32)
static inline uint32_t multiply_u32xu32_rshift32(uint32_t a, uint32_t b) {
	return (uint32_t) (((uint64_t) a * b) >> 32);
}

// computes (((int64_t)a[31:0] * (int64_t)b[31:0] + 0x8000000) >> 32)
static inline int32_t multiply_32x32_rshift32_rounded(int32_t a, int32_t b) {
	return (int32_t) (((int64_t) a * b + 0x80000000LL) >> 32);
}

// computes sum + (((int64_t)a[31:0] * (int64_t)b[31:0] + 0x8000000) >> 32)
static inline int32_t multiply_accumulate_32x32_rshift32_rounded(int32_t sum, int32_t a, int32_t b) {
	return (int32_t) ((((int64_t) sum << 32) + (int64_t) a * b + 0x80000000LL) >> 32);
}

// computes sum - (((int64_t)a[31:0] * (int64_t)b[31:0] + 0x8000000) >> 32)
static inline int32_t multiply_subtract_32x32_rshift32_rounded(int32_t sum, int32_t a, int32_t b) {
	return (int32_t) ((((int64_t) sum << 32) - (int64_t) a * b + 0x80000000LL) >> 32);
}

// computes (a[31:16] | (b[31:16] >> 16))
static inline uint32_t pack_16t_16t(int32_t a, int32_t b) {
	return ((uint32_t) a & 0xFFFF0000u) | ((uint32_t) b >> 16);
}

// computes (a[31:16] | b[15:0])
static inline uint32_t pack_16t_16b(int32_t a, int32_t b) {
	return ((uint32_t) a & 0xFFFF0000u) | ((uint32_t) b & 0x0000FFFFu);
}

// computes ((a[15:0] << 16) | b[15:0])
static inline uint32_t pack_16b_16b(int32_t a, int32_t b) {
	return ((uint32_t) a << 16) | ((uint32_t) b & 0x0000FFFFu);
}

// computes ((a[15:0] << 16) | b[15:0])
static inline uint32_t pack_16x16(int32_t a, int32_t b) {
	return ((uint32_t) a << 16) | ((uint32_t) b & 0x0000FFFFu);
}

// computes (((a[31:16] + b[31:16]) << 16) | (a[15:0 + b[15:0]))  (saturates)
static inline uint32_t signed_add_16_and_16(uint32_t a, uint32_t b) {
	uint32_t hi = (uint16_t) dsp_ssat(dsp_hi16(a) + dsp_hi16(b), 16), lo = (uint16_t) dsp_ssat(dsp_lo16(a) + dsp_lo16(b), 16);
	return (hi << 16) | lo;
}

// computes (((a[31:16] - b[31:16]) << 16) | (a[15:0 - b[15:0]))  (saturates)
static inline int32_t signed_subtract_16_and_16(int32_t a, int32_t b) {
	uint32_t hi = (uint16_t) dsp_ssat(dsp_hi16(a) - dsp_hi16(b), 16), lo = (uint16_t) dsp_ssat(dsp_lo16(a) - dsp_lo16(b), 16);
	return (int32_t) ((hi << 16) | lo);
}

// computes out = (((a[31:16]+b[31:16])/2) <<16) | ((a[15:0]+b[15:0])/2)
static inline int32_t signed_halving_add_16_and_16(int32_t a, int32_t b) {
	uint32_t hi = (uint16_t) ((dsp_hi16(a) + dsp_hi16(b)) >> 1), lo = (uint16_t) ((dsp_lo16(a) + dsp_lo16(b)) >> 1);
	return (int32_t) ((hi << 16) | lo);
}

// computes out = (((a[31:16]-b[31:16])/2) <<16) | ((a[15:0]-b[15:0])/2)
static inline int32_t signed_halving_subtract_16_and_16(int32_t a, int32_t b) {
	uint32_t hi = (uint16_t) ((dsp_hi16(a) - dsp_hi16(b)) >> 1), lo = (uint16_t) ((dsp_lo16(a) - dsp_lo16(b)) >> 1);
	return (int32_t) ((hi << 16) | lo);
}

// computes (sum + ((a[31:0] * b[15:0]) >> 16))
static inline int32_t signed_multiply_accumulate_32x16b(int32_t sum, int32_t a, uint32_t b) {
	return sum + signed_multiply_32x16b(a, b);
}

// computes (sum + ((a[31:0] * b[31:16]) >> 16))
static inline int32_t signed_multiply_accumulate_32x16t(int32_t sum, int32_t a, uint32_t b) {
	return sum + signed_multiply_32x16t(a, b);
}

// computes logical and, forces compiler to allocate register and use single cycle instruction
static inline uint32_t logical_and(uint32_t a, uint32_t b) {
	return a & b;
}

// computes ((a[15:0] * b[15:0]) + (a[31:16] * b[31:16]))
static inline int32_t multiply_16tx16t_add_16bx16b(uint32_t a, uint32_t b) {
	return dsp_lo16(a) * dsp_lo16(b) + dsp_hi16(a) * dsp_hi16(b);
}

// computes ((a[15:0] * b[31:16]) + (a[31:16] * b[15:0]))
static inline int32_t multiply_16tx16b_add_16bx16t(uint32_t a, uint32_t b) {
	return dsp_lo16(a) * dsp_hi16(b) + dsp_hi16(a) * dsp_lo16(b);
}

// // computes sum += ((a[15:0] * b[15:0]) + (a[31:16] * b[31:16]))
static inline int64_t multiply_accumulate_16tx16t_add_16bx16b(int64_t sum, uint32_t a, uint32_t b) {
	return sum + multiply_16tx16t_add_16bx16b(a, b);
}

// // computes sum += ((a[15:0] * b[31:16]) + (a[31:16] * b[15:0]))
static inline int64_t multiply_accumulate_16tx16b_add_16bx16t(int64_t sum, uint32_t a, uint32_t b) {
	return sum + multiply_16tx16b_add_16bx16t(a, b);
}

// computes ((a[15:0] * b[15:0])
static inline int32_t multiply_16bx16b(uint32_t a, uint32_t b) {
	return dsp_lo16(a) * dsp_lo16(b);
}

// computes ((a[15:0] * b[31:16])
static inline int32_t multiply_16bx16t(uint32_t a, uint32_t b) {
	return dsp_lo16(a) * dsp_hi16(b);
}

// computes ((a[31:16] * b[15:0])
static inline int32_t multiply_16tx16b(uint32_t a, uint32_t b) {
	return dsp_hi16(a) * dsp_lo16(b);
}

// computes ((a[31:16] * b[31:16])
static inline int32_t multiply_16tx16t(uint32_t a, uint32_t b) {
	return dsp_hi16(a) * dsp_hi16(b);
}

// computes (a - b), result saturated to 32 bit integer range
static inline int32_t substract_32_saturate(uint32_t a, uint32_t b) {
	return dsp_ssat((int64_t) (int32_t) a - (int64_t) (int32_t) b, 32);
}

#endif
