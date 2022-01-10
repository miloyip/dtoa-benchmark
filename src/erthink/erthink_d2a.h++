/*
 *  Copyright (c) 1994-2021 Leonid Yuriev <leo@yuriev.ru>.
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
#include "erthink_casting.h++"
#include "erthink_clz.h++"
#include "erthink_defs.h"
#include "erthink_misc.h++"
#include "erthink_mul.h"
#include "erthink_u2a.h++"

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
#include <utility> // for std::pair
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

double inline cast(int64_t i64) { return bit_cast<double>(i64); }

double inline cast(uint64_t u64) { return bit_cast<double>(u64); }

int64_t inline cast(double f64) { return bit_cast<int64_t>(f64); }

static cxx11_constexpr_var uint64_t IEEE754_DOUBLE_EXPONENT_MASK =
    UINT64_C(0x7FF0000000000000);
static cxx11_constexpr_var uint64_t IEEE754_DOUBLE_MANTISSA_MASK =
    UINT64_C(0x000FFFFFFFFFFFFF);
static cxx11_constexpr_var int64_t IEEE754_DOUBLE_IMPLICIT_LEAD =
    INT64_C(0x0010000000000000);

enum {
#ifndef IEEE754_DOUBLE_BIAS /* maybe defined in the ieee754.h */
  IEEE754_DOUBLE_BIAS = 0x3ff,
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

  explicit diy_fp(const int64_t i64) cxx11_noexcept {
    const uint64_t exp_bits = (i64 & IEEE754_DOUBLE_EXPONENT_MASK);
    const uint64_t mantissa = (i64 & IEEE754_DOUBLE_MANTISSA_MASK);
    f = mantissa + (exp_bits ? /* normalized */ IEEE754_DOUBLE_IMPLICIT_LEAD
                             : /* de-normalized */ 0u);
    e = static_cast<int>(exp_bits >> IEEE754_DOUBLE_MANTISSA_SIZE) -
        (exp_bits ? /* normalized */ GRISU_EXPONENT_BIAS
                  : /* de-normalized */ GRISU_EXPONENT_BIAS - 1);
  }

  cxx11_constexpr diy_fp(const diy_fp &rhs) cxx11_noexcept = default;
  cxx11_constexpr diy_fp(uint64_t f, int e) cxx11_noexcept : f(f), e(e) {}
  cxx14_constexpr diy_fp &operator=(const diy_fp &rhs) cxx11_noexcept = default;
  diy_fp() = default;

  static diy_fp fixedpoint(uint64_t value, int exp2) {
    assert(exp2 < 1032 && exp2 > -1127);
    if (unlikely(value == 0))
      return diy_fp(0);
    else {
      const int gap = /* avoid underflow of (upper_bound - lower_bound) */ 3;
      const int shift = clz64(value) - gap;
      cxx11_constexpr_var uint64_t top = UINT64_MAX >> gap;
      if (shift >= 0)
        value = value << shift;
      else {
        const uint64_t rounding = UINT64_C(1) << (1 - shift);
        value = ((value < top - rounding) ? value + rounding : top) >> -shift;
      }
      assert(top >= value && value > 0);
      return diy_fp(value, exp2 - shift);
    }
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

static diy_fp cached_power(const int in_exp2, int &out_exp10) cxx11_noexcept {
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

template <typename PRINTER>
inline void adjust(PRINTER &printer, uint64_t delta, uint64_t rest,
                   uint64_t ten_kappa, uint64_t upper,
                   int &inout_exp10) cxx11_noexcept {
  if (printer.is_accurate()) {
    while (delta >= ten_kappa + rest &&
           (rest + ten_kappa < upper ||
            (rest < upper &&
             /* closer */ upper - rest >= rest + ten_kappa - upper))) {
      if (!printer.adjust_last_digit(-1)) {
        ++inout_exp10;
        break;
      }
      rest += ten_kappa;
    }
  }
}

template <typename PRINTER>
inline void make_digits(PRINTER &printer, const uint64_t top, uint64_t delta,
                        int &inout_exp10, const uint64_t value,
                        unsigned shift) {
  uint64_t mask = UINT64_MAX >> (64 - shift);
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
        printer.mantissa_digit(static_cast<char>(digit) + '0');
      early_skip:
        inout_exp10 += kappa;
        assert(kappa >= 0);
        return adjust(printer, delta, tail, dec_power(unsigned(kappa)) << shift,
                      gap, inout_exp10);
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
    if (unlikely(!printer.mantissa_digit(static_cast<char>(digit) + '0')))
      goto early_skip;
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
  while (likely(printer.mantissa_digit(static_cast<char>(digit) + '0') &&
                tail > delta)) {
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

  inout_exp10 += kappa;
  assert(kappa >= -19 && kappa <= 0);
  return adjust(printer, delta, tail, mask + 1,
                gap * dec_power(unsigned(-kappa)), inout_exp10);
}

template <typename PRINTER>
inline void convert(PRINTER &printer, diy_fp diy) cxx11_noexcept {
  if (unlikely(diy.e == 0x7ff - grisu::GRISU_EXPONENT_BIAS))
    return (diy.f - grisu::IEEE754_DOUBLE_IMPLICIT_LEAD) ? printer.nan()
                                                         : printer.inf();
  if (unlikely(diy.f == 0))
    return printer.zero();

  const int lead_zeros = clz64(diy.f);
  /* Check to output as ordinal.
   * Given the remaining optimizations, on average it does not have a positive
   * effect (although a little faster in a simplest cases).
   * However, it reduces the number of inaccuracies and non-shortest strings. */
  if (!printer.is_accurate() && unlikely(diy.e >= -52 && diy.e <= lead_zeros) &&
      (diy.e >= 0 || (diy.f << (64 + diy.e)) == 0)) {
    uint64_t ordinal = (diy.e < 0) ? diy.f >> -diy.e : diy.f << diy.e;
    assert(diy.f == ((diy.e < 0) ? ordinal << -diy.e : ordinal >> diy.e));
    if (printer.integer(ordinal))
      return;
  }

  // normalize
  assert(diy.f <= UINT64_MAX / 2 && lead_zeros > 1);
  diy.e -= lead_zeros;
  diy.f <<= lead_zeros;
  int exp10;
  const diy_fp dec_factor = grisu::cached_power(diy.e, exp10);

  // get boundaries
  const int mojo = diy.f > UINT64_C(0x80000000000007ff) ? 64 : 65;
  const uint64_t delta = dec_factor.f >> (mojo - lead_zeros);
  assert(delta >= 2);
  const uint_fast32_t lsb = diy.scale(dec_factor);
  if (printer.is_accurate())
    // -1 -2 1 0 1: non-shortest 9522 for 25M probes, ratio 0.038088%
    //              shortest errors: +5727 -9156
    //              non-shortest errors: +3 -5
    make_digits(printer, diy.f + ((delta + lsb - 1) >> 1), delta - 2, exp10,
                diy.f + lsb, -diy.e);
  else
    // -1 -2 1 0 0: non-shortest 9522 for 25M probes, ratio 0.038088%
    make_digits(printer, diy.f + ((delta + lsb - 1) >> 1), delta - 2, exp10,
                diy.f, -diy.e);
  printer.exponenta(exp10);
}

template <typename PRINTER>
inline void convert(PRINTER &printer, const double &value) cxx11_noexcept {
  const int64_t i64 = grisu::cast(value);
  printer.sign(i64 < 0);
  return convert(printer, diy_fp(i64));
}

template <bool accurate, unsigned DERIVED_PRINTERS__MAX_CHARS = 23>
struct ieee754_default_printer {
  enum { max_chars = DERIVED_PRINTERS__MAX_CHARS };
  char *end;
  char *begin;

  ieee754_default_printer(char *buffer_begin, char *buffer_end) cxx11_noexcept
      : end(buffer_begin),
        begin(buffer_begin) {
    assert(buffer_end - buffer_begin >= max_chars);
#ifndef NDEBUG
    std::memset(buffer_begin, '_', buffer_end - buffer_begin);
#else
    (void)buffer_end;
#endif
  }

  void sign(bool negative) cxx11_noexcept {
    // strive for branchless
    *end = '-';
    end += negative;
  }

  void nan() cxx11_noexcept {
    // assumes compiler optimize-out memcpy() with small fixed length
    std::memcpy(end, "nan", 4);
    end += 3;
  }

  void inf() cxx11_noexcept {
    // assumes compiler optimize-out memcpy() with small fixed length
    std::memcpy(end, "inf", 4);
    end += 3;
  }

  bool is_accurate() const cxx11_noexcept { return accurate; }

  void zero() cxx11_noexcept { *end++ = '0'; }

  bool integer(uint64_t value) cxx11_noexcept {
    end = u2a(value, end);
    return true;
  }

  bool mantissa_digit(char digit) cxx11_noexcept {
    *end++ = digit;
    return true;
  }

  bool adjust_last_digit(int8_t diff) cxx11_noexcept {
    assert(diff == -1);
    end[-1] += diff;
    if (unlikely(end[-1] < '1')) {
      --end;
      return false;
    }
    return true;
  }

  void exponenta(int value) cxx11_noexcept {
    if (value) {
      const branchless_abs<int> pair(value);
      end[0] = 'e';
      // strive for branchless
      end[1] = '+' + (('-' - '+') & pair.expanded_sign);
      end = dec3(pair.unsigned_abs, end + 2);
    }
  }

  std::pair<char *, char *> finalize_and_get() cxx11_noexcept {
    assert(end > begin && begin + max_chars >= end);
    return std::make_pair(begin, end);
  }
};

// roundtrip-convertible with auto choose between decimal and exponential form
template <bool accurate = false, int min_exp4dec = -4, int max_exp4dec = 10,
          bool force_sign = false>
struct shodan_printer : public ieee754_default_printer<accurate> {
  using inherited = ieee754_default_printer<accurate>;

  static constexpr int gap = sizeof(uint64_t) * 2;
  static constexpr size_t buffer_size = inherited::max_chars + gap * 2;

  shodan_printer(char *buffer_begin, char *buffer_end) cxx11_noexcept
      : inherited(buffer_begin + gap, buffer_end - gap) {
    static_assert(-min_exp4dec < gap, "Oops, min_exp4dec is too less");
    static_assert(max_exp4dec < gap, "Oops, max_exp4dec is too large");
  }

  bool is_negative = false;
  void sign(bool negative) cxx11_noexcept { is_negative = negative; }

  void zero() cxx11_noexcept {
    // assumes compiler optimize-out memcpy() with small fixed length
    std::memcpy(inherited::end, "0.0", 4);
    inherited::end += 3;
  }

  bool integer(uint64_t value) cxx11_noexcept {
    (void)value;
    return false;
  }

  std::pair<char *, char *> finalize_and_get() cxx11_noexcept {
    inherited::begin[-1] = is_negative ? '-' : '+';
    inherited::begin -= (is_negative || force_sign);
    return std::make_pair(inherited::begin, inherited::end);
  }

  void exponenta(int exp) cxx11_noexcept {
    static constexpr char zeros_with_dot[] = {
        '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
        '0', '0', '0', '0', '0', '.', '0', 0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0};
    static_assert(gap * 2 == erthink::array_length(zeros_with_dot), "WTF?");
    const ptrdiff_t ndigits = inherited::end - inherited::begin;
    const int canon_exp = int(ndigits) + exp - 1;

    if (canon_exp >= min_exp4dec && canon_exp <= max_exp4dec) {
      // decimal
      if (exp < 0) {
        if (canon_exp >= 0) {
          // wanna "1.23", have "123", insert dot
          char *ptr = inherited::begin + ndigits + exp;
          // assumes compiler optimize-out memmove() with small fixed length
          std::memmove(ptr + 1, ptr, gap);
          ptr[0] = '.';
          inherited::end += 1;
        } else {
          // wanna "0.000123", have "123", write ahead "0.000"
          // assumes compiler optimize-out memcpy() with small fixed length
          std::memcpy(inherited::begin - gap, zeros_with_dot, gap);
          inherited::begin += canon_exp - 1;
          inherited::begin[1] = '.';
        }
      } else {
        // wanna "123000.0", have "123", should append "000.0"
        // assumes compiler optimize-out memcpy() with small fixed length
        std::memcpy(inherited::end, zeros_with_dot + gap - exp, gap);
        inherited::end += exp + 2;
      }
    } else {
      // exponential: wanna "1.23+e456", have "123"
      inherited::begin -= 1;
      inherited::begin[0] = inherited::begin[1];
      inherited::begin[1] = '.';
      // strive branchless
      inherited::end[0] = '0';
      inherited::end += (inherited::end == inherited::begin);
      inherited::exponenta(canon_exp);
    }
  }
};

// designed to printing fractional part of a fixed-point value
struct fractional_printer : public ieee754_default_printer<true, 32> {
  using inherited = ieee754_default_printer;

  fractional_printer(char *buffer_begin, char *buffer_end) cxx11_noexcept
      : inherited(buffer_begin, buffer_end) {
    assert(buffer_end - buffer_begin >= max_chars);
    *end++ = '.';
  }

  void sign(bool negative) cxx11_noexcept {
    assert(!negative);
    (void)negative;
  }

  void exponenta(int exp) cxx11_noexcept {
    char *const first = begin + 1;
    assert(end > first && end <= begin + max_chars);
    assert(-exp >= end - first);
    const ptrdiff_t zero_needed = -exp - (end - first);
    assert(zero_needed >= 0 && zero_needed < max_chars - 1 - (end - first));
    if (zero_needed > 0) {
      memmove(first + zero_needed, first, size_t(end - first));
      // coverity[bad_memset : FALSE]
      memset(first, '0', size_t(zero_needed));
      end += zero_needed;
    } else {
      while (end[-1] == '0')
        --end;
    }
  }
};

template <bool accurate = false>
struct json5_printer : public ieee754_default_printer<accurate> {
  using inherited = ieee754_default_printer<accurate>;

  json5_printer(char *buffer_begin, char *buffer_end) cxx11_noexcept
      : inherited(buffer_begin, buffer_end) {}

  void nan() cxx11_noexcept {
    // assumes compiler optimize-out memcpy() with small fixed length
    std::memcpy(inherited::end, "NaN", 4);
    inherited::end += 3;
  }

  void inf() cxx11_noexcept {
    // assumes compiler optimize-out memcpy() with small fixed length
    std::memcpy(inherited::end, "Infinity", 8);
    inherited::end += 8;
  }
};

} // namespace grisu

enum { d2a_max_chars = grisu::ieee754_default_printer<false>::max_chars };

template <class PRINTER = grisu::ieee754_default_printer<false>>
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
  PRINTER printer(buffer, buffer + PRINTER::max_chars);
  grisu::convert(printer, value);
  return printer.finalize_and_get().second;
}

static inline __maybe_unused char *d2a_accurate(
    const double &value,
    char *const buffer /* upto d2a_max_chars for -22250738585072014e-324 */) {
  return d2a<grisu::ieee754_default_printer<true>>(value, buffer);
}

static inline __maybe_unused char *d2a_fast(
    const double &value,
    char *const buffer /* upto d2a_max_chars for -22250738585072014e-324 */) {
  return d2a<grisu::ieee754_default_printer<false>>(value, buffer);
}

template <bool accurate = true> struct output_double {
  const double value;
  cxx11_constexpr output_double(const output_double &) = default;
  cxx11_constexpr output_double(const double value) : value(value) {}
};

inline std::ostream &operator<<(std::ostream &out,
                                const erthink::output_double<false> &it) {
  char buf[erthink::grisu::ieee754_default_printer<false>::max_chars];
  char *end = erthink::d2a_fast(it.value, buf);
  return out.write(buf, end - buf);
}

inline std::ostream &operator<<(std::ostream &out,
                                const erthink::output_double<true> &it) {
  char buf[erthink::grisu::ieee754_default_printer<true>::max_chars];
  char *end = erthink::d2a_accurate(it.value, buf);
  return out.write(buf, end - buf);
}

template <typename T> class fpclassify {
  const int std_fpclassify;
  const bool negative;

  constexpr fpclassify(const uint64_t) noexcept = delete;
  constexpr fpclassify(const int64_t) noexcept = delete;
  constexpr fpclassify(const uint32_t) noexcept = delete;
  constexpr fpclassify(const int32_t) noexcept = delete;

public:
  constexpr fpclassify(const fpclassify &) noexcept = default;
  explicit fpclassify(const T &value) noexcept
      : std_fpclassify(::std::fpclassify(value)), negative(value < T(0)) {}

  constexpr bool is_negative() const noexcept { return negative; }
  constexpr bool is_zero() const noexcept { return std_fpclassify == FP_ZERO; }
  constexpr bool is_finite() const noexcept {
    return std_fpclassify != FP_INFINITE;
  }
  constexpr bool is_nan() const noexcept { return std_fpclassify == FP_NAN; }
  constexpr bool is_infinity() const noexcept {
    return std_fpclassify == FP_INFINITE;
  }
  constexpr bool is_normal() const noexcept {
    return std_fpclassify == FP_NORMAL;
  }
  constexpr bool is_subnormal() const noexcept {
    return std_fpclassify == FP_SUBNORMAL;
  }
  constexpr operator int() const noexcept { return std_fpclassify; }
};

template <> class fpclassify<float> {
  using type = uint32_t;
  const type value;

  constexpr fpclassify(const uint64_t) noexcept = delete;
  constexpr fpclassify(const int64_t) noexcept = delete;
  constexpr fpclassify(const type value) noexcept : value(value) {}
  constexpr fpclassify(const int32_t) noexcept = delete;

public:
  constexpr fpclassify(const fpclassify &) noexcept = default;
  explicit fpclassify(const float src) noexcept : value(bit_cast<type>(src)) {}
  friend constexpr fpclassify fpclassify_from_uint(const type value) noexcept;
  constexpr bool is_negative() const noexcept {
    return value > UINT32_C(0x7fffFFFF);
  }
  constexpr bool is_zero() const noexcept {
    return (value & UINT32_C(0x7fffFFFF)) == 0;
  }
  constexpr bool is_finite() const noexcept {
    return (value & UINT32_C(0x7fffFFFF)) < UINT32_C(0x7f800000);
  }
  constexpr bool is_nan() const noexcept {
    return (value & UINT32_C(0x7fffFFFF)) > UINT32_C(0x7f800000);
  }
  constexpr bool is_infinity() const noexcept {
    return (value & UINT32_C(0x7fffFFFF)) == UINT32_C(0x7f800000);
  }
  constexpr bool is_normal() const noexcept {
    return is_finite() && (value & UINT32_C(0x7fffFFFF)) > UINT32_C(0x007fFFFF);
  }
  constexpr bool is_subnormal() const noexcept {
    return is_finite() && (value & UINT32_C(0x7fffFFFF)) < UINT32_C(0x00800000);
  }
  constexpr operator int() const noexcept {
    return likely(is_finite())
               ? (likely(is_normal()) ? FP_NORMAL
                                      : (is_zero() ? FP_ZERO : FP_SUBNORMAL))
           : is_infinity() ? FP_INFINITE
                           : FP_NAN;
  }
};

constexpr fpclassify<float>
fpclassify_from_uint(const uint32_t value) noexcept {
  return fpclassify<float>(value);
}

template <> class fpclassify<double> {
  using type = uint64_t;
  const type value;

  constexpr fpclassify(const type value) noexcept : value(value) {}
  constexpr fpclassify(const int64_t) noexcept = delete;
  constexpr fpclassify(const uint32_t) noexcept = delete;
  constexpr fpclassify(const int32_t) noexcept = delete;

public:
  constexpr fpclassify(const fpclassify &) noexcept = default;
  explicit fpclassify(const double src) noexcept : value(bit_cast<type>(src)) {}
  friend constexpr fpclassify fpclassify_from_uint(const type value) noexcept;
  constexpr bool is_negative() const noexcept {
    return value > UINT64_C(0x7fffFFFFffffFFFF);
  }
  constexpr bool is_zero() const noexcept {
    return (value & UINT64_C(0x7fffFFFFffffFFFF)) == 0;
  }
  constexpr bool is_finite() const noexcept {
    return (value & UINT64_C(0x7fffFFFFffffFFFF)) <
           UINT64_C(0x7ff0000000000000);
  }
  constexpr bool is_nan() const noexcept {
    return (value & UINT64_C(0x7fffFFFFffffFFFF)) >
           UINT64_C(0x7ff0000000000000);
  }
  constexpr bool is_infinity() const noexcept {
    return (value & UINT64_C(0x7fffFFFFffffFFFF)) ==
           UINT64_C(0x7ff0000000000000);
  }
  constexpr bool is_normal() const noexcept {
    return is_finite() && (value & UINT64_C(0x7fffFFFFffffFFFF)) >
                              UINT64_C(0x000fFFFFffffFFFF);
  }
  constexpr bool is_subnormal() const noexcept {
    return is_finite() && (value & UINT64_C(0x7fffFFFFffffFFFF)) <
                              UINT64_C(0x0010000000000000);
  }
  constexpr operator int() const noexcept {
    return likely(is_finite())
               ? (likely(is_normal()) ? FP_NORMAL
                                      : (is_zero() ? FP_ZERO : FP_SUBNORMAL))
           : is_infinity() ? FP_INFINITE
                           : FP_NAN;
  }
};

constexpr fpclassify<double>
fpclassify_from_uint(const uint64_t value) noexcept {
  return fpclassify<double>(value);
}

} // namespace erthink
