/*
  Copyright (c) 2009 Florian Loitsch

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
  */
#pragma once

#include "diy_fp.h"
#include "powers.h"
#include <stdlib.h>
#include <stdbool.h>

typedef union {
    double d;
    uint64_t n;
} converter_t;

static uint64_t double_to_uint64(double d) { converter_t tmp; tmp.d = d; return tmp.n; }
static double uint64_to_double(uint64_t d64) { converter_t tmp; tmp.n = d64; return tmp.d; }

#define DP_SIGNIFICAND_SIZE 52
#define DP_EXPONENT_BIAS (0x3FF + DP_SIGNIFICAND_SIZE)
#define DP_MIN_EXPONENT (-DP_EXPONENT_BIAS)
#define DP_EXPONENT_MASK 0x7FF0000000000000
#define DP_SIGNIFICAND_MASK 0x000FFFFFFFFFFFFF
#define DP_HIDDEN_BIT    0x0010000000000000

static diy_fp_t normalize_diy_fp(diy_fp_t in) {
    diy_fp_t res = in;
    /* Normalize now */
    /* the original number could have been a denormal. */
    while (! (res.f & DP_HIDDEN_BIT))
    {
	res.f <<= 1;
	res.e--;
    }
    /* do the final shifts in one go. Don't forget the hidden bit (the '-1') */
    res.f <<= (DIY_SIGNIFICAND_SIZE - DP_SIGNIFICAND_SIZE - 1);
    res.e = res.e - (DIY_SIGNIFICAND_SIZE - DP_SIGNIFICAND_SIZE - 1);
    return res;
}

static diy_fp_t double2diy_fp(double d) {
    uint64_t d64 = double_to_uint64(d);
    int biased_e = (d64 & DP_EXPONENT_MASK) >> DP_SIGNIFICAND_SIZE;
    uint64_t significand = (d64 & DP_SIGNIFICAND_MASK);
    diy_fp_t res;
    if (biased_e != 0)
    {
	res.f = significand + DP_HIDDEN_BIT;
	res.e = biased_e - DP_EXPONENT_BIAS;
    } else {
	res.f = significand;
	res.e = DP_MIN_EXPONENT + 1;
    }
    return res;
}

static diy_fp_t normalize_boundary(diy_fp_t in) {
    diy_fp_t res = in;
    /* Normalize now */
    /* the original number could have been a denormal. */
    while (! (res.f & (DP_HIDDEN_BIT << 1)))
    {
	res.f <<= 1;
	res.e--;
    }
    /* do the final shifts in one go. Don't forget the hidden bit (the '-1') */
    res.f <<= (DIY_SIGNIFICAND_SIZE - DP_SIGNIFICAND_SIZE - 2);
    res.e = res.e - (DIY_SIGNIFICAND_SIZE - DP_SIGNIFICAND_SIZE - 2);
    return res;
}

static void normalized_boundaries(double d, diy_fp_t* out_m_minus, diy_fp_t* out_m_plus) {
    diy_fp_t v = double2diy_fp(d);
    diy_fp_t pl, mi;
    bool significand_is_zero = v.f == DP_HIDDEN_BIT;
    pl.f  = (v.f << 1) + 1; pl.e  = v.e - 1;
    pl = normalize_boundary(pl);
    if (significand_is_zero)
    {
	mi.f = (v.f << 2) - 1;
	mi.e = v.e - 2;
    } else {
	mi.f = (v.f << 1) - 1;
	mi.e = v.e - 1;
    }
    mi.f <<= mi.e - pl.e;
    mi.e = pl.e;
    *out_m_plus = pl;
    *out_m_minus = mi;
}

static double random_double() {
    uint64_t tmp = 0;
    int i;
    for (i = 0; i < 8; i++) {
	tmp <<= 8;
	tmp += rand() % 256;
    }
    return uint64_to_double(tmp);
}
