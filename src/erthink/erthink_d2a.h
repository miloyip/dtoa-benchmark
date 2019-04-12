﻿/*
 *  Copyright (c) 2010-2019 Leonid Yuriev <leo@yuriev.ru>.
 *  https://github.com/leo-yuriev/erthink
 *  ZLib License
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty. In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgement in the product documentation would be
 *     appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 */

#pragma once

#include "erthink_carryadd.h"
#include "erthink_clz.h"
#include "erthink_defs.h"
#include "erthink_misc.h"
#include "erthink_mul.h"
#include "erthink_u2a.h"

#ifdef _MSC_VER
#pragma warning(push, 1)
#endif
#include <cassert>
#include <cinttypes>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstring> // for memcpy()
#if defined(HAVE_IEEE754_H) || __has_include(<ieee754.h>)
#include <ieee754.h>
#endif
#ifdef _MSC_VER
#pragma warning(pop)
#endif

//------------------------------------------------------------------------------

namespace erthink {

#ifndef NAMESPACE_ERTHINK_D2A_DETAILS
#define NAMESPACE_ERTHINK_D2A_DETAILS /* anonymous */
#endif                                /* NAMESPACE_ERTHINK_D2A_DETAILS */

namespace NAMESPACE_ERTHINK_D2A_DETAILS {
/* low-level routines and bindings to compilers-depended intrinsics */

static cxx14_constexpr int dec_digits(uint32_t n) {
  if (n < UINT_E5) {
    if (n < UINT_E1)
      return 1;
    if (n < UINT_E2)
      return 2;
    if (n < UINT_E3)
      return 3;
    if (n < UINT_E4)
      return 4;
    return 5;
  }
  if (n < UINT_E6)
    return 6;
  if (n < UINT_E7)
    return 7;
  if (n < UINT_E8)
    return 8;
  if (n < UINT_E9)
    return 9;
  return 10;
}

static inline /* LY: 'inline' here if better for performance than 'constexpr' */
    uint64_t
    dec_power(unsigned n) {
  static const uint64_t array[] = {
      UINT_E0,  UINT_E1,  UINT_E2,  UINT_E3,  UINT_E4,  UINT_E5,  UINT_E6,
      UINT_E7,  UINT_E8,  UINT_E9,  UINT_E10, UINT_E11, UINT_E12, UINT_E13,
      UINT_E14, UINT_E15, UINT_E16, UINT_E17, UINT_E18, UINT_E19};
  static_assert(array_length(array) == 20, "WTF?");
  assert(n < array_length(array));
  return array[n];
}

} // namespace NAMESPACE_ERTHINK_D2A_DETAILS

//------------------------------------------------------------------------------

namespace grisu {

/* Based on Grisu2 algorithm by Florian Loitsch.
 * https://www.cs.tufts.edu/~nr/cs257/archive/florian-loitsch/printf.pdf */

static constexpr uint64_t IEEE754_DOUBLE_EXPONENT_MASK =
    UINT64_C(0x7FF0000000000000);
static constexpr uint64_t IEEE754_DOUBLE_MANTISSA_MASK =
    UINT64_C(0x000FFFFFFFFFFFFF);
static constexpr int64_t IEEE754_DOUBLE_IMPLICIT_LEAD =
    INT64_C(0x0010000000000000);

enum {
#ifndef IEEE754_DOUBLE_BIAS
  IEEE754_DOUBLE_BIAS = 0x3ff /* Added to exponent. */,
#endif
  IEEE754_DOUBLE_MANTISSA_SIZE = 52,
  GRISU_EXPONENT_BIAS = IEEE754_DOUBLE_BIAS + IEEE754_DOUBLE_MANTISSA_SIZE
};

union casting_union {
  double f;
  int64_t i;
  uint64_t u;
  constexpr casting_union(double v) : f(v) {}
  constexpr casting_union(uint64_t v) : u(v) {}
  constexpr casting_union(int64_t v) : i(v) {}
};

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4820 /* FOO bytes padding added                      \
                                  after data member BAR */)
#endif
struct diy_fp {
  uint64_t f;
  int e;

  explicit diy_fp(const casting_union &value) {
    const uint64_t exp_bits = (value.u & IEEE754_DOUBLE_EXPONENT_MASK);
    const uint64_t mantissa = (value.u & IEEE754_DOUBLE_MANTISSA_MASK);
    f = mantissa + (exp_bits ? IEEE754_DOUBLE_IMPLICIT_LEAD : 0u);
    e = static_cast<int>(exp_bits >> IEEE754_DOUBLE_MANTISSA_SIZE) -
        (exp_bits ? GRISU_EXPONENT_BIAS : GRISU_EXPONENT_BIAS - 1);
  }
  constexpr diy_fp(const diy_fp &rhs) : f(rhs.f), e(rhs.e) {}
  constexpr diy_fp(uint64_t f, int e) : f(f), e(e) {}
  diy_fp() {}

  static diy_fp fixedpoint(uint64_t value, int exp2) {
    assert(value > 0);
    assert(exp2 < 1032 && exp2 > -1127);
    const int gap = /* avoid underflow of (upper_bound - lower_bound) */ 3;
    const int shift = clz64(value) - gap;
    constexpr uint64_t top = UINT64_MAX >> gap;
    const uint64_t rounding = UINT64_C(1) << (1 - shift);
    value = (shift >= 0)
                ? value << shift
                : ((value < top - rounding) ? value + rounding : top) >> -shift;
    assert(top >= value && value > 0);
    return diy_fp(value, exp2 - shift);
  }

  static diy_fp middle(const diy_fp &upper, const diy_fp &lower) {
    assert(upper.e == lower.e && upper.f > lower.f);
    const int64_t diff = int64_t(upper.f - lower.f);
    assert(diff > 0);
    return diy_fp(upper.f - uint64_t(diff >> 1), upper.e);
  }

  void scale(const diy_fp &factor, bool roundup) {
    const uint64_t l = mul_64x64_128(f, factor.f, &f);
    assert(f < UINT64_MAX - INT32_MAX);
    if (roundup)
      f += l >> 63;
    e += factor.e + 64;
  }

  diy_fp operator-(const diy_fp &rhs) const {
    assert(e == rhs.e);
    assert(f >= rhs.f);
    return diy_fp(f - rhs.f, e);
  }
};
#ifdef _MSC_VER
#pragma warning(pop)
#endif

static diy_fp cached_power(const int in_exp2, int &out_exp10) {
  constexpr size_t n_items = (340 + 340) / 8 + 1 /* 10^-340 .. 0 .. 10^340 */;
  assert(in_exp2 < 1096 && in_exp2 > -1191);

  /* LY: avoid branches and IEEE754-to-integer conversion,
   * which could leads to save/restore FPU's flags/mode. */
  constexpr int64_t factor =
      static_cast<int64_t>(IEEE754_DOUBLE_IMPLICIT_LEAD /
                           3.321928094887362347870319 /* log2(10.0) */);
  const int exp2_rebased = (-61 - in_exp2);
  const int64_t exp10_unbiased_scaled =
      exp2_rebased * factor + 348 * IEEE754_DOUBLE_IMPLICIT_LEAD - 1;
  const unsigned exp10_unbiased = static_cast<unsigned>(
      exp10_unbiased_scaled >> IEEE754_DOUBLE_MANTISSA_SIZE);
  assert(static_cast<int>(exp10_unbiased) ==
         static_cast<int>(ceil((-61 - in_exp2) / log2(10.0))) + 347);

  const size_t index = exp10_unbiased >> 3;
  assert(n_items > index);
  out_exp10 = int(340 - (exp10_unbiased & ~7));

  static constexpr short power10_exp2[] = {
      -1193, -1166, -1140, -1113, -1087, -1060, -1034, -1007, -980, -954, -927,
      -901,  -874,  -847,  -821,  -794,  -768,  -741,  -715,  -688, -661, -635,
      -608,  -582,  -555,  -529,  -502,  -475,  -449,  -422,  -396, -369, -343,
      -316,  -289,  -263,  -236,  -210,  -183,  -157,  -130,  -103, -77,  -50,
      -24,   3,     30,    56,    83,    109,   136,   162,   189,  216,  242,
      269,   295,   322,   348,   375,   402,   428,   455,   481,  508,  534,
      561,   588,   614,   641,   667,   694,   720,   747,   774,  800,  827,
      853,   880,   907,   933,   960,   986,   1013,  1039,  1066};
  static_assert(array_length(power10_exp2) == n_items, "WTF?");

  static const uint64_t power10_mas[] = {
      UINT64_C(0xBAAEE17FA23EBF76), UINT64_C(0x8B16FB203055AC76),
      UINT64_C(0xCF42894A5DCE35EA), UINT64_C(0x9A6BB0AA55653B2D),
      UINT64_C(0xE61ACF033D1A45DF), UINT64_C(0xAB70FE17C79AC6CA),
      UINT64_C(0xFF77B1FCBEBCDC4F), UINT64_C(0xBE5691EF416BD60C),
      UINT64_C(0x8DD01FAD907FFC3C), UINT64_C(0xD3515C2831559A83),
      UINT64_C(0x9D71AC8FADA6C9B5), UINT64_C(0xEA9C227723EE8BCB),
      UINT64_C(0xAECC49914078536D), UINT64_C(0x823C12795DB6CE57),
      UINT64_C(0xC21094364DFB5637), UINT64_C(0x9096EA6F3848984F),
      UINT64_C(0xD77485CB25823AC7), UINT64_C(0xA086CFCD97BF97F4),
      UINT64_C(0xEF340A98172AACE5), UINT64_C(0xB23867FB2A35B28E),
      UINT64_C(0x84C8D4DFD2C63F3B), UINT64_C(0xC5DD44271AD3CDBA),
      UINT64_C(0x936B9FCEBB25C996), UINT64_C(0xDBAC6C247D62A584),
      UINT64_C(0xA3AB66580D5FDAF6), UINT64_C(0xF3E2F893DEC3F126),
      UINT64_C(0xB5B5ADA8AAFF80B8), UINT64_C(0x87625F056C7C4A8B),
      UINT64_C(0xC9BCFF6034C13053), UINT64_C(0x964E858C91BA2655),
      UINT64_C(0xDFF9772470297EBD), UINT64_C(0xA6DFBD9FB8E5B88F),
      UINT64_C(0xF8A95FCF88747D94), UINT64_C(0xB94470938FA89BCF),
      UINT64_C(0x8A08F0F8BF0F156B), UINT64_C(0xCDB02555653131B6),
      UINT64_C(0x993FE2C6D07B7FAC), UINT64_C(0xE45C10C42A2B3B06),
      UINT64_C(0xAA242499697392D3), UINT64_C(0xFD87B5F28300CA0E),
      UINT64_C(0xBCE5086492111AEB), UINT64_C(0x8CBCCC096F5088CC),
      UINT64_C(0xD1B71758E219652C), UINT64_C(0x9C40000000000000),
      UINT64_C(0xE8D4A51000000000), UINT64_C(0xAD78EBC5AC620000),
      UINT64_C(0x813F3978F8940984), UINT64_C(0xC097CE7BC90715B3),
      UINT64_C(0x8F7E32CE7BEA5C70), UINT64_C(0xD5D238A4ABE98068),
      UINT64_C(0x9F4F2726179A2245), UINT64_C(0xED63A231D4C4FB27),
      UINT64_C(0xB0DE65388CC8ADA8), UINT64_C(0x83C7088E1AAB65DB),
      UINT64_C(0xC45D1DF942711D9A), UINT64_C(0x924D692CA61BE758),
      UINT64_C(0xDA01EE641A708DEA), UINT64_C(0xA26DA3999AEF774A),
      UINT64_C(0xF209787BB47D6B85), UINT64_C(0xB454E4A179DD1877),
      UINT64_C(0x865B86925B9BC5C2), UINT64_C(0xC83553C5C8965D3D),
      UINT64_C(0x952AB45CFA97A0B3), UINT64_C(0xDE469FBD99A05FE3),
      UINT64_C(0xA59BC234DB398C25), UINT64_C(0xF6C69A72A3989F5C),
      UINT64_C(0xB7DCBF5354E9BECE), UINT64_C(0x88FCF317F22241E2),
      UINT64_C(0xCC20CE9BD35C78A5), UINT64_C(0x98165AF37B2153DF),
      UINT64_C(0xE2A0B5DC971F303A), UINT64_C(0xA8D9D1535CE3B396),
      UINT64_C(0xFB9B7CD9A4A7443C), UINT64_C(0xBB764C4CA7A44410),
      UINT64_C(0x8BAB8EEFB6409C1A), UINT64_C(0xD01FEF10A657842C),
      UINT64_C(0x9B10A4E5E9913129), UINT64_C(0xE7109BFBA19C0C9D),
      UINT64_C(0xAC2820D9623BF429), UINT64_C(0x80444B5E7AA7CF85),
      UINT64_C(0xBF21E44003ACDD2D), UINT64_C(0x8E679C2F5E44FF8F),
      UINT64_C(0xD433179D9C8CB841), UINT64_C(0x9E19DB92B4E31BA9),
      UINT64_C(0xEB96BF6EBADF77D9), UINT64_C(0xAF87023B9BF0EE6B)};
  static_assert(array_length(power10_mas) == n_items, "WTF?");

  return diy_fp(power10_mas[index], power10_exp2[index]);
}

static inline void round(char *end, uint64_t delta, uint64_t rest,
                         uint64_t ten_kappa, uint64_t upper) {
  while (rest < upper && delta - rest >= ten_kappa &&
         (rest + ten_kappa < upper || /* closer */
          upper - rest > rest + ten_kappa - upper)) {
    end[-1] -= 1;
    rest += ten_kappa;
  }
}

static inline char *make_digits(const diy_fp &v, const diy_fp &upper,
                                uint64_t delta, char *const buffer,
                                int &inout_exp10) {
  const unsigned shift = unsigned(-upper.e);
  const uint64_t mask = UINT64_MAX >> (64 - shift);
  char *ptr = buffer;
  const diy_fp gap = upper - v;

  assert((upper.f >> shift) <= UINT_E9);
  uint_fast32_t digit, body = static_cast<uint_fast32_t>(upper.f >> shift);
  uint64_t tail = upper.f & mask;
  int kappa = dec_digits(body);

  while (kappa > 0) {
    switch (kappa) {
    default:
      assert(false);
      __unreachable();
    case 10:
      digit = body / UINT_E9;
      body %= UINT_E9;
      break;
    case 9:
      digit = body / UINT_E8;
      body %= UINT_E8;
      break;
    case 8:
      digit = body / UINT_E7;
      body %= UINT_E7;
      break;
    case 7:
      digit = body / UINT_E6;
      body %= UINT_E6;
      break;
    case 6:
      digit = body / UINT_E5;
      body %= UINT_E5;
      break;
    case 5:
      digit = body / UINT_E4;
      body %= UINT_E4;
      break;
    case 4:
      digit = body / 1000u;
      body %= 1000u;
      break;
    case 3:
      digit = body / 100u;
      body %= 100u;
      break;
    case 2:
      digit = body / 10u;
      body %= 10u;
      break;
    case 1:
      digit = body;
      body = 0u;
      break;
    }
    *ptr = static_cast<char>(digit + '0');
    --kappa;
    const uint64_t left = (static_cast<uint64_t>(body) << shift) + tail;
    ptr += (digit || ptr > buffer);
    if (left < delta) {
      inout_exp10 += kappa;
      assert(kappa >= 0);
      round(ptr, delta, left, dec_power(unsigned(kappa)) << shift, gap.f);
      return ptr;
    }
  }

  if (ptr == buffer) {
    do {
      --kappa;
      tail *= 10;
      delta *= 10;
      digit = static_cast<uint_fast32_t>(tail >> shift);
      tail &= mask;
    } while (unlikely(!digit));
    *ptr++ = static_cast<char>(digit + '0');
  }
  while (tail > delta) {
    --kappa;
    tail *= 10;
    delta *= 10;
    digit = static_cast<uint_fast32_t>(tail >> shift);
    tail &= mask;
    *ptr++ = static_cast<char>(digit + '0');
  }

  inout_exp10 += kappa;
  assert(kappa >= -19 && kappa <= 0);
  const uint64_t unit = dec_power(unsigned(-kappa));
  round(ptr, delta, tail, mask + 1, gap.f * unit);
  return ptr;
}

static inline char *convert(diy_fp v, char *const buffer, int &out_exp10) {
  if (unlikely(v.f == 0)) {
    out_exp10 = 0;
    *buffer = '0';
    return buffer + 1;
  }

  const int left = clz64(v.f);
#if 0
  // LY: check to output as ordinal
  if (unlikely(v.e >= -52 && v.e <= left && (v.e >= 0 || (v.f << (64 + v.e)) == 0))) {
    uint64_t ordinal = (v.e < 0) ? v.f >> -v.e : v.f << v.e;
    assert(v.f == ((v.e < 0) ? ordinal << -v.e : ordinal >> v.e));
    out_exp10 = 0;
    return u2a(ordinal, buffer);
  }
#endif

  // LY: normalize
  assert(v.f <= UINT64_MAX / 2 && left > 1);
  v.e -= left;
  v.f <<= left;

  // LY: get boundaries
  const int mojo = v.f >= UINT64_C(0x8000000080000000) ? left - 1 : left - 2;
  const uint64_t half_epsilon = UINT64_C(1) << mojo;
  diy_fp upper(v.f + half_epsilon, v.e);
  diy_fp lower(v.f - half_epsilon, v.e);

  const diy_fp dec_factor = cached_power(upper.e, out_exp10);
  upper.scale(dec_factor, false);
  lower.scale(dec_factor, true);
  v = diy_fp::middle(upper, lower);
  --upper.f;
  assert(upper.f > lower.f);
  return make_digits(v, upper, upper.f - lower.f - 1, buffer, out_exp10);
}

} // namespace grisu

static __maybe_unused char *
d2a(const grisu::casting_union &value,
    char *const buffer /* upto 23 chars for -22250738585072014e-324 */) {
  assert(!std::isnan(value.f) && !std::isinf(value.f));
  // LY: strive for branchless (SSA-optimizer must solve this)
  *buffer = '-';
  int exponent;
  char *ptr =
      grisu::convert(grisu::diy_fp(value), buffer + (value.i < 0), exponent);
  if (exponent != 0) {
    const branchless_abs<int> pair(exponent);
    ptr[0] = 'e';
    // LY: strive for branchless
    ptr[1] = '+' + (('-' - '+') & pair.expanded_sign);
    ptr = dec3(pair.unsigned_abs, ptr + 2);
  }
  assert(ptr - buffer <= 23);
  return ptr;
}

} // namespace erthink
