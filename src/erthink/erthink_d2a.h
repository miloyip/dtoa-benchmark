/*
 *  Copyright (c) 1994-2020 Leonid Yuriev <leo@yuriev.ru>.
 *  https://github.com/erthink/erthink
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

/* Double-to-string conversion based on Grisu algorithm by Florian Loitsch
 * https://www.cs.tufts.edu/~nr/cs257/archive/florian-loitsch/printf.pdf
 *
 * Seems this is the fastest Grisu-based implementation,
 * but it is not exactly Grisu3 nor Grisu2:
 *
 * 1. Generated string representation ALWAYS roundtrip convertible to
 *    the original value, i.e. any correct implementation of strtod()
 *    will always return EXACTLY the origin double value.
 *
 * 2. Generated string representation is shortest for more than 99.963% of
 *    IEEE-754 double values, i.e. one extra digit for less that 0.037% values.
 *    Moreover, for less than 0.06% of double values, the last digit differs
 *    from an ideal nearest by ±1.
 *
 * 3. Compared to Ryū algorithm (by Ulf Adams), this implementation
 *    significantly less in code size and spends less clock cycles per digit,
 *    but may slightly inferior in a whole on a 16-17 digit values.
 */

#if !defined(__cplusplus) || __cplusplus < 201103L
#error "This source code requires C++11 at least."
#endif

#include "erthink_carryadd.h"
#include "erthink_casting.h"
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
#include <ostream>
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

#ifndef ERTHINK_D2A_AVOID_MUL
#define ERTHINK_D2A_AVOID_MUL 0
#endif /* ERTHINK_D2A_AVOID_MUL */

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

static inline /* LY: 'inline' here is better for performance than 'constexpr' */
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

static cxx11_constexpr_var uint64_t IEEE754_DOUBLE_EXPONENT_MASK =
    UINT64_C(0x7FF0000000000000);
static cxx11_constexpr_var uint64_t IEEE754_DOUBLE_MANTISSA_MASK =
    UINT64_C(0x000FFFFFFFFFFFFF);
static cxx11_constexpr_var int64_t IEEE754_DOUBLE_IMPLICIT_LEAD =
    INT64_C(0x0010000000000000);

enum {
#ifndef IEEE754_DOUBLE_BIAS
  IEEE754_DOUBLE_BIAS = 0x3ff /* Added to exponent. */,
#endif
  IEEE754_DOUBLE_MANTISSA_SIZE = 52,
  GRISU_EXPONENT_BIAS = IEEE754_DOUBLE_BIAS + IEEE754_DOUBLE_MANTISSA_SIZE
};

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4820 /* FOO bytes padding added                      \
                                  after data member BAR */)
#endif
struct diy_fp {
  uint64_t f;
  int e;

  explicit diy_fp(const int64_t i64) {
    const uint64_t exp_bits = (i64 & IEEE754_DOUBLE_EXPONENT_MASK);
    const uint64_t mantissa = (i64 & IEEE754_DOUBLE_MANTISSA_MASK);
    f = mantissa + (exp_bits ? IEEE754_DOUBLE_IMPLICIT_LEAD : 0u);
    e = static_cast<int>(exp_bits >> IEEE754_DOUBLE_MANTISSA_SIZE) -
        (exp_bits ? GRISU_EXPONENT_BIAS : GRISU_EXPONENT_BIAS - 1);
  }
  cxx11_constexpr diy_fp(const diy_fp &rhs) cxx11_noexcept = default;
  cxx11_constexpr diy_fp(uint64_t f, int e) cxx11_noexcept : f(f), e(e) {}
  cxx11_constexpr diy_fp &operator=(const diy_fp &rhs) cxx11_noexcept = default;
  diy_fp() = default;

  static diy_fp fixedpoint(uint64_t value, int exp2) {
    assert(value > 0);
    assert(exp2 < 1032 && exp2 > -1127);
    const int gap = /* avoid underflow of (upper_bound - lower_bound) */ 3;
    const int shift = clz64(value) - gap;
    cxx11_constexpr_var uint64_t top = UINT64_MAX >> gap;
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

  uint_fast32_t scale(const diy_fp &factor) {
    const uint64_t l = mul_64x64_128(f, factor.f, &f);
    assert(f < UINT64_MAX - INT32_MAX);
    e += factor.e + 64;
    return static_cast<uint_fast32_t>(l >> 63);
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
  cxx11_constexpr_var std::size_t n_items =
      (340 + 340) / 8 + 1 /* 10^-340 .. 0 .. 10^340 */;
  assert(in_exp2 < 1096 && in_exp2 > -1191);

  /* LY: avoid branches and IEEE754-to-integer conversion,
   * which could leads to save/restore FPU's flags/mode. */
  cxx11_constexpr_var int64_t factor =
      static_cast<int64_t>(IEEE754_DOUBLE_IMPLICIT_LEAD /
                           3.321928094887362347870319 /* log2(10.0) */);
  const int exp2_rebased = (-61 - in_exp2);
  const int64_t exp10_unbiased_scaled =
      exp2_rebased * factor + 348 * IEEE754_DOUBLE_IMPLICIT_LEAD - 1;
  const unsigned exp10_unbiased = static_cast<unsigned>(
      exp10_unbiased_scaled >> IEEE754_DOUBLE_MANTISSA_SIZE);
  assert(static_cast<int>(exp10_unbiased) ==
         static_cast<int>(ceil((-61 - in_exp2) / log2(10.0))) + 347);

  const std::size_t index = exp10_unbiased >> 3;
  assert(n_items > index);
  out_exp10 = int(340 - (exp10_unbiased & ~7));

  static cxx11_constexpr_var short power10_exp2[] = {
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

static __always_inline void round(char *&end, uint64_t delta, uint64_t rest,
                                  uint64_t ten_kappa, uint64_t upper,
                                  int &inout_exp10) {
  while (delta >= ten_kappa + rest &&
         (rest + ten_kappa < upper ||
          (rest < upper &&
           /* closer */ upper - rest >= rest + ten_kappa - upper))) {
    end[-1] -= 1;
    if (unlikely(end[-1] < '1')) {
      inout_exp10 += 1;
      end -= 1;
      return;
    }
    rest += ten_kappa;
  }
}

static inline char *make_digits(const bool accurate, const uint64_t top,
                                uint64_t delta, char *const buffer,
                                int &inout_exp10, const uint64_t value,
                                unsigned shift) {
  uint64_t mask = UINT64_MAX >> (64 - shift);
  char *ptr = buffer;
  const uint64_t gap = top - value;

  assert((top >> shift) <= UINT_E9);
  uint_fast32_t digit, body = static_cast<uint_fast32_t>(top >> shift);
  uint64_t tail = top & mask;
  int kappa = dec_digits(body);
  assert(kappa > 0);

  do {
    switch (--kappa) {
    default:
      assert(false);
      __unreachable();
    case 9:
      digit = body / UINT_E9;
      body %= UINT_E9;
      break;
    case 8:
      digit = body / UINT_E8;
      body %= UINT_E8;
      break;
    case 7:
      digit = body / UINT_E7;
      body %= UINT_E7;
      break;
    case 6:
      digit = body / UINT_E6;
      body %= UINT_E6;
      break;
    case 5:
      digit = body / UINT_E5;
      body %= UINT_E5;
      break;
    case 4:
      digit = body / UINT_E4;
      body %= UINT_E4;
      break;
    case 3:
      digit = body / 1000u;
      body %= 1000u;
      break;
    case 2:
      digit = body / 100u;
      body %= 100u;
      break;
    case 1:
      digit = body / 10u;
      body %= 10u;
      break;
    case 0:
      digit = body;
      if (unlikely(tail < delta)) {
      early_last:
        *ptr++ = static_cast<char>(digit + '0');
      early_skip:
        inout_exp10 += kappa;
        assert(kappa >= 0);
        if (accurate)
          round(ptr, delta, tail, dec_power(unsigned(kappa)) << shift, gap,
                inout_exp10);
        return ptr;
      }

      while (true) {
        if (likely(digit))
          goto done;
        --kappa;
#if ERTHINK_D2A_AVOID_MUL
        tail += tail << 2;   // *= 5
        delta += delta << 2; // *= 5
        digit = static_cast<uint_fast32_t>(tail >> --shift);
        tail &= (mask >>= 1);
#else
        tail *= 10;
        delta *= 10;
        digit = static_cast<uint_fast32_t>(tail >> shift);
        tail &= mask;
#endif /* ERTHINK_D2A_AVOID_MUL */
      }
    }
  } while (unlikely(digit == 0));

  while (true) {
    *ptr++ = static_cast<char>(digit + '0');
    switch (--kappa) {
    default:
      assert(false);
      __unreachable();
    case 9:
      digit = body / UINT_E9;
      body %= UINT_E9;
      break;
    case 8:
      digit = body / UINT_E8;
      body %= UINT_E8;
      break;
    case 7:
      digit = body / UINT_E7;
      body %= UINT_E7;
      break;
    case 6:
      digit = body / UINT_E6;
      body %= UINT_E6;
      break;
    case 5:
      digit = body / UINT_E5;
      body %= UINT_E5;
      break;
    case 4:
      digit = body / UINT_E4;
      body %= UINT_E4;
      break;
    case 3:
      digit = body / 1000u;
      body %= 1000u;
      break;
    case 2:
      digit = body / 100u;
      body %= 100u;
      break;
    case 1:
      digit = body / 10u;
      body %= 10u;
      break;
    case 0:
      digit = body;
      goto done;
    }

    const uint64_t left = (static_cast<uint64_t>(body) << shift) + tail;
    if (unlikely(left < delta)) {
      if (likely(digit))
        goto early_last;
      ++kappa;
      goto early_skip;
    }
  }

done:
  *ptr++ = static_cast<char>(digit + '0');
  while (likely(tail > delta)) {
    --kappa;
#if ERTHINK_D2A_AVOID_MUL
    tail += tail << 2;   // *= 5
    delta += delta << 2; // *= 5
    digit = static_cast<uint_fast32_t>(tail >> --shift);
    tail &= (mask >>= 1);
#else
    tail *= 10;
    delta *= 10;
    digit = static_cast<uint_fast32_t>(tail >> shift);
    tail &= mask;
#endif /* ERTHINK_D2A_AVOID_MUL */
    *ptr++ = static_cast<char>(digit + '0');
  }

  inout_exp10 += kappa;
  assert(kappa >= -19 && kappa <= 0);
  if (accurate) {
    const uint64_t unit = dec_power(unsigned(-kappa));
    round(ptr, delta, tail, mask + 1, gap * unit, inout_exp10);
  }
  return ptr;
}

static inline char *convert(const bool accurate, diy_fp v, char *const buffer,
                            int &out_exp10) {
  if (unlikely(v.f == 0)) {
    out_exp10 = 0;
    *buffer = '0';
    return buffer + 1;
  }

  const int lead_zeros = clz64(v.f);
  /* Check to output as ordinal.
   * Given the remaining optimizations, on average it does not have a positive
   * effect (although a little faster in a simplest cases).
   * However, it reduces the number of inaccuracies and non-shortest strings. */
  if (!accurate && unlikely(v.e >= -52 && v.e <= lead_zeros) &&
      (v.e >= 0 || (v.f << (64 + v.e)) == 0)) {
    uint64_t ordinal = (v.e < 0) ? v.f >> -v.e : v.f << v.e;
    assert(v.f == ((v.e < 0) ? ordinal << -v.e : ordinal >> v.e));
    out_exp10 = 0;
    return u2a(ordinal, buffer);
  }

  // LY: normalize
  assert(v.f <= UINT64_MAX / 2 && lead_zeros > 1);
  v.e -= lead_zeros;
  v.f <<= lead_zeros;
  const diy_fp dec_factor = cached_power(v.e, out_exp10);

  // LY: get boundaries
  const int mojo = v.f > UINT64_C(0x80000000000007ff) ? 64 : 65;
  const uint64_t delta = dec_factor.f >> (mojo - lead_zeros);
  assert(delta >= 2);
  const uint_fast32_t lsb = v.scale(dec_factor);
  if (accurate)
    // -1 -2 1 0 1: non-shortest 9522 for 25M probes, ratio 0.038088%
    //              shortest errors: +5727 -9156
    //              non-shortest errors: +3 -5
    return make_digits(accurate, v.f + ((delta + lsb - 1) >> 1), delta - 2,
                       buffer, out_exp10, v.f + lsb, -v.e);
  else
    // -1 -2 1 0 0: non-shortest 9522 for 25M probes, ratio 0.038088%
    return make_digits(accurate, v.f + ((delta + lsb - 1) >> 1), delta - 2,
                       buffer, out_exp10, v.f, -v.e);
}

double inline cast(int64_t i64) { return bit_cast<double>(i64); }

double inline cast(uint64_t u64) { return bit_cast<double>(u64); }

int64_t inline cast(double f64) { return bit_cast<int64_t>(f64); }

} // namespace grisu

enum { d2a_max_chars = 23 };

template <bool accurate>
/* The "accurate" controls the trade-off between conversion speed and accuracy:
 *
 *  - True: accurately conversion to impeccable string representation,
 *    which will be a nearest to the actual binary value.
 *
 *  - False: conversion will be slightly faster and the result will also
 *    be correct (inverse conversion via stdtod() will give the original value).
 *    However, the string representation will be slightly larger than the ideal
 *    nearest value. */
char *
d2a(const double &value,
    char *const
        buffer /* upto erthink::d2a_max_chars for -22250738585072014e-324 */) {
  assert(!std::isnan(value) && !std::isinf(value));
  const int64_t i64 = grisu::cast(value);
  // LY: strive for branchless (SSA-optimizer must solve this)
  *buffer = '-';
  int exponent;
  char *ptr = grisu::convert(accurate, grisu::diy_fp(i64), buffer + (i64 < 0),
                             exponent);
  if (exponent != 0) {
    const branchless_abs<int> pair(exponent);
    ptr[0] = 'e';
    // LY: strive for branchless
    ptr[1] = '+' + (('-' - '+') & pair.expanded_sign);
    ptr = dec3(pair.unsigned_abs, ptr + 2);
  }
  assert(ptr - buffer <= d2a_max_chars);
  return ptr;
}

static inline __maybe_unused char *d2a_accurate(
    const double &value,
    char *const buffer /* upto d2a_max_chars for -22250738585072014e-324 */) {
  return d2a<true>(value, buffer);
}

static inline __maybe_unused char *d2a_fast(
    const double &value,
    char *const buffer /* upto d2a_max_chars for -22250738585072014e-324 */) {
  return d2a<false>(value, buffer);
}

template <bool accurate = true> struct output_double {
  const double value;
  cxx11_constexpr output_double(const output_double &) = default;
  cxx11_constexpr output_double(const double value) : value(value) {}
};

} // namespace erthink

namespace std {

inline ostream &operator<<(ostream &out,
                           const erthink::output_double<false> &it) {
  char buf[erthink::d2a_max_chars];
  char *end = erthink::d2a_fast(it.value, buf);
  return out.write(buf, end - buf);
}

inline ostream &operator<<(ostream &out,
                           const erthink::output_double<true> &it) {
  char buf[erthink::d2a_max_chars];
  char *end = erthink::d2a_accurate(it.value, buf);
  return out.write(buf, end - buf);
}

} // namespace std
