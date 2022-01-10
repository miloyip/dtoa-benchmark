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

#include "erthink_arch.h"
#include "erthink_bswap.h"
#include "erthink_byteorder.h"
#include "erthink_carryadd.h"
#include "erthink_clz.h++"
#include "erthink_defs.h"
#include "erthink_intrin.h"
#include "erthink_mul.h"
#include "erthink_rot.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201) /* nonstandard extension used :                \
                                   nameless struct / union */
#endif                          /* _MSC_VER (warnings) */

#ifdef __cplusplus
#include "erthink_casting.h++" // for erthink::enable_if_t stub
#include "erthink_constexpr_cstr.h++"
#include "erthink_dynamic_constexpr.h++"
#include <algorithm> // for std::reverse
#include <array>
#include <limits>
#include <ostream>
#include <string>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <utility>

#if (defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L) ||          \
    (__cplusplus >= 201703L && defined(__GLIBCXX__) && __GLIBCXX__ > 20200304)
#include <charconv>
#define ERTHINK_HAVE_std_to_chars 1
#else
#define ERTHINK_HAVE_std_to_chars 0
#endif /* __cpp_lib_to_chars */

#if defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L
#include <string_view>
#define ERTHINK_HAVE_std_string_view 1
#else
#define ERTHINK_HAVE_std_string_view 0
#endif /* __cpp_lib_string_view */

namespace erthink {
#endif /* __cplusplus */

#ifndef ERTHINK_USE_NATIVE_128
#if defined(__SIZEOF_INT128__) ||                                              \
    (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128)
#define ERTHINK_USE_NATIVE_128 1
#else
#define ERTHINK_USE_NATIVE_128 0
#endif
#endif /* ERTHINK_USE_NATIVE_128 */

#if ERTHINK_USE_NATIVE_128
#define erthink_u128_constexpr11 cxx01_constexpr
#define erthink_u128_constexpr14 cxx14_constexpr
#else
#define erthink_u128_constexpr11 erthink_dynamic_constexpr
#define erthink_u128_constexpr14 erthink_dynamic_constexpr
#endif /* ERTHINK_USE_NATIVE_128 */

union uint128_t {
  struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint64_t l, h;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    uint64_t h, l;
#else
#error "FIXME: Unsupported byte order"
#endif /* __BYTE_ORDER__ */
  };
#ifdef ERTHINK_NATIVE_U128_TYPE
  ERTHINK_NATIVE_U128_TYPE u128;
#endif /* ERTHINK_NATIVE_U128_TYPE */
#ifdef ERTHINK_NATIVE_I128_TYPE
  ERTHINK_NATIVE_I128_TYPE i128;
#endif /* ERTHINK_NATIVE_U128_TYPE */
  uint32_t u64[2];
  uint32_t u32[4];
  uint16_t u16[8];
  uint8_t u8[16];

#ifdef __cplusplus
  uint128_t() = default;
  constexpr uint128_t(const uint128_t &) = default;
  cxx14_constexpr uint128_t &operator=(const uint128_t &) noexcept = default;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  constexpr uint128_t(uint64_t h, uint64_t l) noexcept : l(l), h(h) {}
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  constexpr uint128_t(uint64_t h, uint64_t l) noexcept : h(h), l(l) {}
#else
#error "FIXME: Unsupported byte order"
#endif /* __BYTE_ORDER__ */

  template <typename T, typename = typename erthink::enable_if_t<
                            std::is_convertible<T, uint64_t>::value>>
  constexpr uint128_t(const T &v) noexcept
      : uint128_t(std::is_signed<T>::value ? static_cast<int64_t>(v) >> 63 : 0,
                  static_cast<uint64_t>(v)) {}

  template <typename T, typename = typename erthink::enable_if_t<
                            std::is_convertible<uint64_t, T>::value>>
  constexpr explicit operator T() const noexcept {
    return T(l);
  }

  template <typename T, typename = typename erthink::enable_if_t<
                            std::is_convertible<T, uint64_t>::value>>
  cxx14_constexpr uint128_t &operator=(const T &v) noexcept {
    return operator=(uint128_t(v));
  }

#ifdef ERTHINK_NATIVE_U128_TYPE
  constexpr uint128_t(const ERTHINK_NATIVE_U128_TYPE &v) noexcept : u128(v) {}
  constexpr operator ERTHINK_NATIVE_U128_TYPE() const noexcept { return u128; }
  cxx14_constexpr uint128_t &
  operator=(const ERTHINK_NATIVE_U128_TYPE v) noexcept {
    u128 = v;
    return *this;
  }
#endif /* ERTHINK_NATIVE_U128_TYPE */

#ifdef ERTHINK_NATIVE_I128_TYPE
  constexpr uint128_t(const ERTHINK_NATIVE_I128_TYPE &v) noexcept : i128(v) {}
  constexpr operator ERTHINK_NATIVE_I128_TYPE() const noexcept { return i128; }
  cxx14_constexpr uint128_t &
  operator=(const ERTHINK_NATIVE_I128_TYPE v) noexcept {
    i128 = v;
    return *this;
  }
#endif /* ERTHINK_NATIVE_I128_TYPE */

  constexpr operator bool() const noexcept {
#if ERTHINK_USE_NATIVE_128
    return u128 != 0;
#else
    return (l | h) != 0;
#endif /* ERTHINK_USE_NATIVE_128 */
  }

  constexpr uint32_t most_significant_word() const noexcept {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return u32[3];
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return u32[0];
#else
#error "FIXME: Unsupported byte order"
#endif /* __BYTE_ORDER__ */
  }

  constexpr bool most_significant_bit() const noexcept {
#ifdef ERTHINK_ARCH64
    return static_cast<int64_t>(h) < 0;
#else
    return static_cast<int32_t>(most_significant_word()) < 0;
#endif /* ERTHINK_ARCH64 */
  }

  __nothrow_pure_function static erthink_u128_constexpr11
      std::pair<uint128_t, uint128_t>
      divmod(const uint128_t &dividend, const uint128_t &divisor) noexcept;

  erthink_u128_constexpr14 uint128_t
  divmod_quotient(const uint128_t &divisor) noexcept {
    const auto quotient_remainder(divmod(*this, divisor));
    *this = quotient_remainder.second;
    return quotient_remainder.first;
  }

  erthink_u128_constexpr14 uint128_t
  divmod_remainder(const uint128_t &divisor) noexcept {
    const auto quotient_remainder(divmod(*this, divisor));
    *this = quotient_remainder.first;
    return quotient_remainder.second;
  }

  erthink_u128_constexpr14 std::pair<char *, std::errc>
  to_chars(char *first, char *last, unsigned base = 10) const noexcept;
  inline std::string to_string(unsigned base = 10,
                               const std::string::allocator_type &alloc =
                                   std::string::allocator_type()) const;
  std::string to_hex() const { return to_string(16); }

  static erthink_u128_constexpr14 std::tuple<const char *, uint128_t, std::errc>
  from_chars(const char *first, const char *last, unsigned base = 0) noexcept;
  static erthink_u128_constexpr14 uint128_t from_string(const char *begin,
                                                        const char *end,
                                                        unsigned base = 0);

#if ERTHINK_HAVE_std_string_view
  static erthink_u128_constexpr14 std::tuple<const char *, uint128_t, std::errc>
  from_chars(const std::string_view &sv, unsigned base = 10) noexcept {
    return from_chars(sv.data(), sv.data() + sv.length(), base);
  }

  static erthink_u128_constexpr14 uint128_t
  from_string(const std::string_view &sv, unsigned base = 0) {
    return from_string(sv.data(), sv.data() + sv.length(), base);
  }
#endif /* ERTHINK_HAVE_std_string_view */

  static erthink_u128_constexpr14 uint128_t from_string(const char *cstr,
                                                        unsigned base = 0) {
    return from_string(cstr, cstr ? cstr + strlen(cstr) : cstr, base);
  }

#endif /* __cplusplus */
};

#ifndef __cplusplus
typedef union uint128_t uint128_t;
#else

//------------------------------------------------------------------------------
} // namespace erthink
namespace std {

template <>
struct numeric_limits<erthink::uint128_t> : public numeric_limits<unsigned> {
  using type = erthink::uint128_t;
  static constexpr int radix = 2;
  static constexpr int digits = 128;
  static constexpr int digits10 = /* 39 */ 1 + 128 * 643l / 2136;
  static constexpr type epsilon() noexcept { return 0; }
  static constexpr type lowest() noexcept { return 0; }
  static constexpr type min() noexcept { return 0; }
  static constexpr type max() noexcept {
    /* 340282366920938463463374607431768211455 */
    return type(~uint64_t(0), ~uint64_t(0));
  }
};

inline std::string to_string(const erthink::uint128_t &v, unsigned base = 10) {
  return v.to_string(base);
}

#if ERTHINK_HAVE_std_to_chars

erthink_u128_constexpr11 std::to_chars_result
to_chars(char *first, char *last, const erthink::uint128_t &value,
         int base = 10) noexcept {
  const auto pair = value.to_chars(first, last, base);
  return {pair.first, pair.second};
}

erthink_u128_constexpr11 std::from_chars_result
from_chars(const char *first, const char *last, erthink::uint128_t &value,
           int base = 10) noexcept {
  const auto triplet = erthink::uint128_t::from_chars(first, last, base);
  if (std::get<0>(triplet) != first && std::get<2>(triplet) == std::errc())
    value = std::get<1>(triplet);
  return {std::get<0>(triplet), std::get<2>(triplet)};
}

#endif /* ERTHINK_HAVE_std_to_chars */

} // namespace std
namespace erthink {
//------------------------------------------------------------------------------

constexpr bool operator==(const uint128_t &x, const uint128_t &y) noexcept {
#if ERTHINK_USE_NATIVE_128
  return x.u128 == y.u128;
#elif defined(ERTHINK_ARCH64)
  return (x.l == y.l) & (x.h == y.h);
#else
  return (x.l == y.l) && (x.h == y.h);
#endif /* ERTHINK_USE_NATIVE_128 */
}

constexpr bool operator!=(const uint128_t &x, const uint128_t &y) noexcept {
#if ERTHINK_USE_NATIVE_128
  return x.u128 != y.u128;
#elif defined(ERTHINK_ARCH64)
  return (x.l != y.l) | (x.h != y.h);
#else
  return (x.l != y.l) || (x.h != y.h);
#endif /* ERTHINK_USE_NATIVE_128 */
}

#if !ERTHINK_USE_NATIVE_128
namespace details {

cxx14_constexpr __nothrow_pure_function uint128_t
add128_constexpr(const uint128_t &x, const uint128_t &y) cxx11_noexcept {
  const uint64_t l = x.l + y.l;
  const bool c = l < x.l;
  return uint128_t(x.h + y.h + c, l);
}

static __nothrow_pure_function uint128_t
add128_dynamic(const uint128_t &x, const uint128_t &y) cxx11_noexcept {
  uint128_t r;
  add64carry_last(add64carry_first(x.l, y.l, &r.l), x.h, y.h, &r.h);
  assert(r == add128_constexpr(x, y));
  return r;
}

ERTHINK_DYNAMIC_CONSTEXPR(uint128_t, add128,
                          (const uint128_t &x, const uint128_t &y), (x, y),
                          (x.h + x.l + y.h + y.l))

// ----------------------------------------------------------------------------

cxx14_constexpr __nothrow_pure_function uint128_t
sub128_constexpr(const uint128_t &x, const uint128_t &y) cxx11_noexcept {
  const uint64_t l = x.l - y.l;
  const bool c = l > x.l;
  return uint128_t(x.h - y.h - c, l);
}

static __nothrow_pure_function uint128_t
sub128_dynamic(const uint128_t &x, const uint128_t &y) cxx11_noexcept {
  uint128_t r;
  sub64borrow_last(sub64borrow_first(x.l, y.l, &r.l), x.h, y.h, &r.h);
  assert(r == sub128_constexpr(x, y));
  return r;
}

ERTHINK_DYNAMIC_CONSTEXPR(uint128_t, sub128,
                          (const uint128_t &x, const uint128_t &y), (x, y),
                          (x.h + x.l + y.h + y.l))

// ----------------------------------------------------------------------------

constexpr __nothrow_pure_function bool
gt128_constexpr(const uint128_t &x, const uint128_t &y) cxx11_noexcept {
#ifdef ERTHINK_ARCH64
  return (x.h > y.h) | ((x.h == y.h) & (x.l > y.l));
#else
  return x.h > y.h || (x.h == y.h && x.l > y.l);
#endif /* ERTHINK_ARCH64 */
}

static __nothrow_pure_function bool
gt128_dynamic(const uint128_t &x, const uint128_t &y) cxx11_noexcept {
#if defined(sub64borrow_next) || __has_builtin(__builtin_sub_overflow) ||      \
    __has_builtin(__builtin_subcll)
  uint128_t unused;
  const bool r = sub64borrow_next(sub64borrow_first(y.l, x.l, &unused.l), y.h,
                                  x.h, &unused.h);
#else
  const bool r = gt128_constexpr(x, y);
#endif /* sub64borrow_next || __builtin_sub_overflow || __builtin_subcll */
  assert(r == gt128_constexpr(x, y));
  return r;
}

ERTHINK_DYNAMIC_CONSTEXPR(bool, gt128, (const uint128_t &x, const uint128_t &y),
                          (x, y), (x.h + x.l + y.h + y.l))

// ----------------------------------------------------------------------------

constexpr __nothrow_pure_function bool
lt128_constexpr(const uint128_t &x, const uint128_t &y) cxx11_noexcept {
#ifdef ERTHINK_ARCH64
  return (x.h < y.h) | ((x.h == y.h) & (x.l < y.l));
#else
  return x.h < y.h || (x.h == y.h && x.l < y.l);
#endif /* ERTHINK_ARCH64 */
}

static __nothrow_pure_function bool
lt128_dynamic(const uint128_t &x, const uint128_t &y) cxx11_noexcept {
#if defined(sub64borrow_next) || __has_builtin(__builtin_sub_overflow) ||      \
    __has_builtin(__builtin_subcll)
  uint128_t unused;
  const bool r = sub64borrow_next(sub64borrow_first(x.l, y.l, &unused.l), x.h,
                                  y.h, &unused.h);
#else
  const bool r = lt128_constexpr(x, y);
#endif /* sub64borrow_next || __builtin_sub_overflow || __builtin_subcll */
  assert(r == lt128_constexpr(x, y));
  return r;
}

ERTHINK_DYNAMIC_CONSTEXPR(bool, lt128, (const uint128_t &x, const uint128_t &y),
                          (x, y), (x.h + x.l + y.h + y.l))

// ----------------------------------------------------------------------------

cxx14_constexpr __nothrow_pure_function uint128_t
umul128_constexpr(uint64_t x, uint64_t y) cxx11_noexcept {
  const uint64_t xl = x & UINT32_C(0xFFFFffff);
  const uint64_t xh = x >> 32;
  const uint64_t yl = y & UINT32_C(0xFFFFffff);
  const uint64_t yh = y >> 32;

  const uint64_t ll = xl * yl;
  const uint64_t hh = xh * yh;
  const uint64_t hl = xh * yl + (ll >> 32);
  const uint64_t lh = xl * yh + (hl & UINT32_C(0xFFFFffff));

  return uint128_t(hh + (hl >> 32) + (lh >> 32),
                   (lh << 32) | (ll & UINT32_C(0xFFFFffff)));
}

static __nothrow_pure_function uint128_t umul128_dynamic(uint64_t x, uint64_t y)
    cxx11_noexcept {
  uint64_t h, l = mul_64x64_128(x, y, &h);
  assert(uint128_t(h, l) == umul128_constexpr(x, y));
  return uint128_t(h, l);
}

ERTHINK_DYNAMIC_CONSTEXPR(uint128_t, umul128,
                          (const uint64_t x, const uint64_t y), (x, y), (x + y))

} // namespace details
#endif /* ERTHINK_USE_NATIVE_128 */

//------------------------------------------------------------------------------

constexpr uint128_t operator~(const uint128_t &v) noexcept {
#if ERTHINK_USE_NATIVE_128
  return uint128_t(~v.u128);
#else
  return uint128_t(~v.h, ~v.l);
#endif /* ERTHINK_USE_NATIVE_128 */
}

constexpr uint128_t operator^(const uint128_t &x, const uint128_t &y) noexcept {
#if ERTHINK_USE_NATIVE_128
  return uint128_t(x.u128 ^ y.u128);
#else
  return uint128_t(x.h ^ y.h, x.l ^ y.l);
#endif /* ERTHINK_USE_NATIVE_128 */
}

constexpr uint128_t operator&(const uint128_t &x, const uint128_t &y) noexcept {
#if ERTHINK_USE_NATIVE_128
  return uint128_t(x.u128 & y.u128);
#else
  return uint128_t(x.h & y.h, x.l & y.l);
#endif /* ERTHINK_USE_NATIVE_128 */
}

constexpr uint128_t operator|(const uint128_t &x, const uint128_t &y) noexcept {
#if ERTHINK_USE_NATIVE_128
  return uint128_t(x.u128 | y.u128);
#else
  return uint128_t(x.h | y.h, x.l | y.l);
#endif /* ERTHINK_USE_NATIVE_128 */
}

erthink_u128_constexpr11 uint128_t operator+(const uint128_t &x,
                                             const uint128_t &y) noexcept {
#if ERTHINK_USE_NATIVE_128
  return uint128_t(x.u128 + y.u128);
#else
  return details::add128(x, y);
#endif /* ERTHINK_USE_NATIVE_128 */
}

erthink_u128_constexpr11 uint128_t operator-(const uint128_t &x,
                                             const uint128_t &y) noexcept {
#if ERTHINK_USE_NATIVE_128
  return uint128_t(x.u128 - y.u128);
#else
  return details::sub128(x, y);
#endif /* ERTHINK_USE_NATIVE_128 */
}

constexpr uint128_t operator<<(const uint128_t &v, unsigned s) noexcept {

#if ERTHINK_USE_NATIVE_128
  return CONSTEXPR_ASSERT(s < 128), uint128_t(v.u128 << s);
#else
  return CONSTEXPR_ASSERT(s < 128),
         uint128_t((s < 64) ? (v.h << s) | (s ? v.l >> (64 - s) : 0)
                            : v.l << (s - 64),
                   (s < 64) ? v.l << s : 0);
#endif /* ERTHINK_USE_NATIVE_128 */
}

constexpr uint128_t operator<<(const uint128_t &v, int s) noexcept {
  return operator<<(v, unsigned(s));
}

constexpr uint128_t operator>>(const uint128_t &v, unsigned s) noexcept {
#if ERTHINK_USE_NATIVE_128
  return CONSTEXPR_ASSERT(s < 128), uint128_t(v.u128 >> s);
#else
  return CONSTEXPR_ASSERT(s < 128),
         uint128_t((s < 64) ? v.h >> s : 0,
                   (s < 64) ? (s ? v.h << (64 - s) : 0) | (v.l >> s)
                            : v.h >> (s - 64));
#endif /* ERTHINK_USE_NATIVE_128 */
}

constexpr uint128_t operator>>(const uint128_t &v, int s) noexcept {
  return operator>>(v, unsigned(s));
}

constexpr uint128_t operator+(const uint128_t &v) noexcept { return v; }

erthink_u128_constexpr11 uint128_t operator-(const uint128_t &v) noexcept {
#if ERTHINK_USE_NATIVE_128
  return uint128_t(-v.i128);
#else
  return uint128_t(0) - v;
#endif /* ERTHINK_USE_NATIVE_128 */
}

erthink_u128_constexpr14 uint128_t &operator++(uint128_t &v) noexcept {
#if ERTHINK_USE_NATIVE_128
  ++v.u128;
  return v;
#else
  return v = v + uint128_t(1);
#endif /* ERTHINK_USE_NATIVE_128 */
}

erthink_u128_constexpr14 uint128_t &operator--(uint128_t &v) noexcept {
#if ERTHINK_USE_NATIVE_128
  --v.u128;
  return v;
#else
  return v = v - uint128_t(1);
#endif /* ERTHINK_USE_NATIVE_128 */
}

erthink_u128_constexpr14 uint128_t operator++(uint128_t &v, int) noexcept {
  const auto t = v;
  ++v;
  return t;
}

erthink_u128_constexpr14 uint128_t operator--(uint128_t &v, int) noexcept {
  const auto t = v;
  --v;
  return t;
}

erthink_u128_constexpr14 uint128_t &operator+=(uint128_t &x,
                                               const uint128_t &y) noexcept {
  return x = x + y;
}

erthink_u128_constexpr14 uint128_t &operator-=(uint128_t &x,
                                               const uint128_t &y) noexcept {
  return x = x - y;
}

cxx14_constexpr uint128_t &operator|=(uint128_t &x,
                                      const uint128_t &y) noexcept {
  return x = x | y;
}

cxx14_constexpr uint128_t &operator&=(uint128_t &x,
                                      const uint128_t &y) noexcept {
  return x = x & y;
}

cxx14_constexpr uint128_t &operator^=(uint128_t &x,
                                      const uint128_t &y) noexcept {
  return x = x ^ y;
}

cxx14_constexpr uint128_t &operator<<=(uint128_t &x, unsigned s) noexcept {
  return x = x << s;
}

cxx14_constexpr uint128_t &operator>>=(uint128_t &x, unsigned s) noexcept {
  return x = x >> s;
}

erthink_u128_constexpr11 bool operator>(const uint128_t &x,
                                        const uint128_t &y) noexcept {
#if ERTHINK_USE_NATIVE_128
  return x.u128 > y.u128;
#else
  return details::gt128(x, y);
#endif /* ERTHINK_USE_NATIVE_128 */
}

erthink_u128_constexpr11 bool operator<(const uint128_t &x,
                                        const uint128_t &y) noexcept {
#if ERTHINK_USE_NATIVE_128
  return x.u128 < y.u128;
#else
  return details::lt128(x, y);
#endif /* ERTHINK_USE_NATIVE_128 */
}

erthink_u128_constexpr11 bool operator>=(const uint128_t &x,
                                         const uint128_t &y) noexcept {
  return !(x < y);
}

erthink_u128_constexpr11 bool operator<=(const uint128_t &x,
                                         const uint128_t &y) noexcept {
  return !(x > y);
}

//------------------------------------------------------------------------------

namespace details {

constexpr unsigned char2digit(char c) noexcept {
  return (c <= '9') ? unsigned(c) - '0' /* intentional unsigned overflow during
                                           subtraction in case: c < '0' */
         : (c |= 32, c >= 'a') ? c - 'a' + 10
                               : ~0u;
}

constexpr char digit2char(unsigned char d,
                          unsigned char alphabase = 'a') noexcept {
  return (d < 10) ? d + '0' : d + alphabase - 10;
}

#if !ERTHINK_USE_NATIVE_128

/* Based on "Improved division by invariant integers"
 * https://gmplib.org/~tege/division-paper.pdf */

static constexpr uint16_t reciprocal_v0_constexpr(const size_t d9) noexcept {
  return CONSTEXPR_ASSERT(d9 >= 256 && d9 <= 511),
         uint16_t(UINT32_C(0x7fd00) / d9);
}

__nothrow_pure_function static inline uint16_t
reciprocal_v0_dynamic(const size_t d9) noexcept {
  static const uint16_t table[] = {
      2045, 2037, 2029, 2021, 2013, 2005, 1998, 1990, 1983, 1975, 1968, 1960,
      1953, 1946, 1938, 1931, 1924, 1917, 1910, 1903, 1896, 1889, 1883, 1876,
      1869, 1863, 1856, 1849, 1843, 1836, 1830, 1824, 1817, 1811, 1805, 1799,
      1792, 1786, 1780, 1774, 1768, 1762, 1756, 1750, 1745, 1739, 1733, 1727,
      1722, 1716, 1710, 1705, 1699, 1694, 1688, 1683, 1677, 1672, 1667, 1661,
      1656, 1651, 1646, 1641, 1636, 1630, 1625, 1620, 1615, 1610, 1605, 1600,
      1596, 1591, 1586, 1581, 1576, 1572, 1567, 1562, 1558, 1553, 1548, 1544,
      1539, 1535, 1530, 1526, 1521, 1517, 1513, 1508, 1504, 1500, 1495, 1491,
      1487, 1483, 1478, 1474, 1470, 1466, 1462, 1458, 1454, 1450, 1446, 1442,
      1438, 1434, 1430, 1426, 1422, 1418, 1414, 1411, 1407, 1403, 1399, 1396,
      1392, 1388, 1384, 1381, 1377, 1374, 1370, 1366, 1363, 1359, 1356, 1352,
      1349, 1345, 1342, 1338, 1335, 1332, 1328, 1325, 1322, 1318, 1315, 1312,
      1308, 1305, 1302, 1299, 1295, 1292, 1289, 1286, 1283, 1280, 1276, 1273,
      1270, 1267, 1264, 1261, 1258, 1255, 1252, 1249, 1246, 1243, 1240, 1237,
      1234, 1231, 1228, 1226, 1223, 1220, 1217, 1214, 1211, 1209, 1206, 1203,
      1200, 1197, 1195, 1192, 1189, 1187, 1184, 1181, 1179, 1176, 1173, 1171,
      1168, 1165, 1163, 1160, 1158, 1155, 1153, 1150, 1148, 1145, 1143, 1140,
      1138, 1135, 1133, 1130, 1128, 1125, 1123, 1121, 1118, 1116, 1113, 1111,
      1109, 1106, 1104, 1102, 1099, 1097, 1095, 1092, 1090, 1088, 1086, 1083,
      1081, 1079, 1077, 1074, 1072, 1070, 1068, 1066, 1064, 1061, 1059, 1057,
      1055, 1053, 1051, 1049, 1047, 1044, 1042, 1040, 1038, 1036, 1034, 1032,
      1030, 1028, 1026, 1024};
  assert(d9 >= 256 && d9 <= 511);
  assert(reciprocal_v0_constexpr(d9) == table[d9 - 256]);
  return table[d9 - 256];
}

ERTHINK_DYNAMIC_CONSTEXPR(uint16_t, reciprocal_v0, (const size_t d9), (d9), d9)

erthink_u128_constexpr14 uint64_t reciprocal_2by1(const uint64_t d) noexcept {
  const uint_fast32_t v0 = reciprocal_v0(size_t(d >> 55));
  const uint64_t d40 = (d >> 24) + 1;
  const uint_fast32_t v1 = (v0 << 11) - uint_fast32_t(v0 * v0 * d40 >> 40) - 1;
  const uint64_t v2 = (uint64_t(v1) << 13) +
                      (v1 * (UINT64_C(0x1000000000000000) - v1 * d40) >> 47);
  const uint64_t d63 = (d + 1) >> 1;
  const uint64_t e = ((d & 1) ? v2 >> 1 : 0) - v2 * d63;
  const uint64_t v3 = (v2 << 31) + (umul128(v2, e).h >> 1);
  const uint64_t v4 = v3 - (umul128(v3, d) + uint128_t(d)).h - d;
  return v4;
}

erthink_u128_constexpr14 uint64_t reciprocal_3by2(const uint128_t &d) noexcept {
  auto v = reciprocal_2by1(d.h);
  auto p = d.h * v + d.l;
  if (p < d.l) {
    v -= (p < d.h) ? 1 : 2;
    p -= (p < d.h) ? d.h : d.h + d.h;
  }
  const auto t = umul128(v, d.l);
  p += t.h;
  if (p < t.h)
    v -= (uint128_t(p, t.l) < d) ? 1 : 2;
  return v;
}

erthink_u128_constexpr14 std::pair<uint64_t, uint64_t>
divmod_2by1(const uint128_t &u, const uint64_t d, const uint64_t v) noexcept {
  auto q = umul128(v, u.h) + u;
  auto r = u.l - ++q.h * d;
  if (r > q.l) {
    --q.h;
    r += d;
  }
  if (unlikely(r >= d)) {
    ++q.h;
    r -= d;
  }
  return {q.h, r};
}

erthink_u128_constexpr14 std::pair<uint64_t, uint128_t>
divmod_3by2(const uint64_t u2, const uint64_t u1, const uint64_t u0,
            const uint128_t &d, const uint64_t v) noexcept {
  auto q = umul128(v, u2) + uint128_t(u2, u1);
  auto r = uint128_t(u1 - q.h * d.h, u0) - umul128(d.l, q.h) - d;
  if (r.h >= q.l) {
    --q.h;
    r += d;
  }
  if (unlikely(r >= d)) {
    ++q.h;
    r -= d;
  }
  return {q.h + 1, r};
}

erthink_u128_constexpr14 __nothrow_pure_function std::pair<uint128_t, uint128_t>
divmod_u128(uint128_t x, uint128_t y) noexcept {
  if (y.h == 0) {
    const unsigned s = clz64(y.l);
    uint64_t o = 0;
    if (s) {
      o = x.h >> (64 - s);
      x <<= s;
      y.l <<= s;
    }
    const auto v = reciprocal_2by1(y.l);
    const auto h = divmod_2by1(uint128_t(o, x.h), y.l, v);
    const auto l = divmod_2by1(uint128_t(h.second, x.l), y.l, v);
    return {uint128_t(h.first, l.first), l.second >> s};
  }

  if (y.h > x.h)
    return {0, x};

  const unsigned s = clz64(y.h);
  if (s == 0) {
    const unsigned q = (y.h < x.h) | (y.l <= x.l);
    return {q, q ? x - y : x};
  }

  const uint64_t o = x.h >> (64 - s);
  x <<= s;
  y <<= s;
  const auto v = reciprocal_3by2(y);
  const auto r = divmod_3by2(o, x.h, x.l, y, v);
  return {r.first, r.second >> s};
}

erthink_u128_constexpr14 __nothrow_pure_function std::pair<uint128_t, uint128_t>
divmod_s128(const uint128_t &x, const uint128_t &y) noexcept {
  const auto sign = static_cast<int32_t>(x.most_significant_word() ^
                                         y.most_significant_word());
  auto pair = divmod_u128(x.most_significant_bit() ? -x : x,
                          y.most_significant_bit() ? -y : y);
  if (sign < 0) {
    pair.first = -pair.first;
    pair.second = -pair.second;
  }
  return pair;
}

#endif /* ERTHINK_USE_NATIVE_128 */

} // namespace details

erthink_u128_constexpr11 uint128_t operator*(const uint128_t &x,
                                             const uint128_t &y) noexcept {
#if ERTHINK_USE_NATIVE_128
  return uint128_t(x.u128 * y.u128);
#else
  uint128_t r = details::umul128(x.l, y.l);
  r.h += x.l * y.h + y.l * x.h;
  return r;
#endif /* ERTHINK_USE_NATIVE_128 */
}

erthink_u128_constexpr11 uint128_t operator/(const uint128_t &x,
                                             const uint128_t &y) noexcept {
#if ERTHINK_USE_NATIVE_128
  return uint128_t(x.u128 / y.u128);
#else
  return details::divmod_u128(x, y).first;
#endif /* ERTHINK_USE_NATIVE_128 */
}

erthink_u128_constexpr11 uint128_t operator%(const uint128_t &x,
                                             const uint128_t &y) noexcept {
#if ERTHINK_USE_NATIVE_128
  return uint128_t(x.u128 % y.u128);
#else
  return details::divmod_u128(x, y).second;
#endif /* ERTHINK_USE_NATIVE_128 */
}

erthink_u128_constexpr11 std::pair<uint128_t, uint128_t>
uint128_t::divmod(const uint128_t &dividend,
                  const uint128_t &divisor) noexcept {
#if ERTHINK_USE_NATIVE_128
  return {dividend.u128 / divisor.u128, dividend.u128 % divisor.u128};
#else
  return details::divmod_u128(dividend, divisor);
#endif /* ERTHINK_USE_NATIVE_128 */
}

//------------------------------------------------------------------------------

erthink_u128_constexpr14 uint128_t &operator*=(uint128_t &x,
                                               uint128_t y) noexcept {
  return x = x * y;
}

erthink_u128_constexpr14 uint128_t &operator/=(uint128_t &x,
                                               uint128_t y) noexcept {
  return x = x / y;
}

erthink_u128_constexpr14 uint128_t &operator%=(uint128_t &x,
                                               uint128_t y) noexcept {
  return x = x % y;
}

//------------------------------------------------------------------------------

static cxx14_constexpr uint128_t ror128(uint128_t v, unsigned s) noexcept {
  return (s &= 127) ? (v << (128 - s)) | (v >> s) : v;
}

static cxx14_constexpr uint128_t rol128(uint128_t v, unsigned s) noexcept {
  return (s &= 127) ? (v >> (128 - s)) | (v << s) : v;
}

static cxx14_constexpr int clz128(uint128_t v) noexcept {
  return v.h ? clz64(v.h) : 64 + clz64(v.l);
}

static constexpr_intrin uint128_t bswap128(uint128_t v) noexcept {
  return uint128_t(bswap(v.l), bswap(v.h));
}

template <>
cxx14_constexpr uint128_t ror<uint128_t>(uint128_t v, unsigned s) noexcept {
  return ror128(v, s);
}

template <>
cxx14_constexpr uint128_t rol<uint128_t>(uint128_t v, unsigned s) noexcept {
  return rol128(v, s);
}

template <> cxx14_constexpr int clz<uint128_t>(uint128_t v) noexcept {
  return clz128(v);
}

template <> constexpr_intrin uint128_t bswap<uint128_t>(uint128_t v) {
  return bswap128(v);
}

//------------------------------------------------------------------------------

erthink_u128_constexpr14 std::pair<char *, std::errc>
uint128_t::to_chars(char *first, char *last, unsigned base) const noexcept {
  if (unlikely(base < 2 || base > 36))
    return {nullptr, std::errc::invalid_argument};

  if (*this < uint128_t(base)) {
    *first = details::digit2char(char(*this));
    return {first + 1, std::errc()};
  }

  auto v = *this;
  auto p = last;
  do {
    const auto d = char(v.divmod_remainder(base));
    if (unlikely(p <= first))
      return {last, std::errc::value_too_large};
    *--p = details::digit2char(d);
  } while (v);

  const auto len = last - p;
  if (p > first)
    std::memmove(first, p, len);
  return {first + len, std::errc()};
}

inline std::string
uint128_t::to_string(unsigned base,
                     const std::string::allocator_type &alloc) const {
  if (unlikely(base < 2 || base > 36))
    throw std::invalid_argument("invalid base");

  if (*this < uint128_t(base))
    return std::string(1, details::digit2char(char(*this)), alloc);

  std::string str(alloc);
  auto v = *this;
  do {
    const auto d = char(v.divmod_remainder(base));
    str.push_back(details::digit2char(d));
  } while (v);

  std::reverse(str.begin(), str.end());
  return str;
}

inline std::ostream &operator<<(std::ostream &out, uint128_t v) {
  const auto flags = out.flags();
  assert(flags & std::ios_base::basefield);

  // producing digits in reverse order (from least to most significant)
  std::array<char, /* enough for octal representation */ 128 / 3 + 1> buffer;
  auto digits = buffer.end();
  do {
    char d;
    if (flags & std::ios_base::dec) {
      d = char(v.divmod_remainder(10)) + '0';
    } else {
      d = char(v) & ((flags & std::ios_base::hex) ? 15 : 7);
      v >>= (flags & std::ios_base::hex) ? 4 : 3;
      d += d < 10 ? '0'
           : (flags & std::ios_base::uppercase) ? 'A' - 10
                                                : 'a' - 10;
    }
    assert(digits > buffer.begin() && digits <= buffer.end());
    *--digits = d;
  } while (v);

  // compute the result width and padding for adjustment
  const auto prefix_len =
      ((flags & std::ios_base::showpos) ? /* the '+' sign */ 1 : 0) +
      ((std::ios_base::showbase !=
        (flags & (std::ios_base::showbase | std::ios_base::dec)))
           ? /* no prefix for decimal */ 0
       : (flags & std::ios_base::hex) ? /* '0x' for hex */ 2
                                      : /* '0' for octal */ 1);
  auto padding = out.width() - (buffer.end() - digits + prefix_len);

  // padding at the left up to target width in case of right adjustment
  if (!(flags & (std::ios_base::internal | std::ios_base::left)))
    while (padding-- > 0 && !out.bad())
      out.put(out.fill());

  // put sign and base prefix if required
  if (prefix_len) {
    static const char pattern[] = {'+', '0', 'x', '+', '0', 'X'};
    const char *prefix =
        (flags & std::ios_base::uppercase) ? pattern + 4 : pattern + 1;
    out.write((flags & std::ios_base::showpos) ? prefix - 1 : prefix,
              prefix_len);
  }

  // padding up to target width in case of internal adjustment
  if (flags & std::ios_base::internal)
    while (padding-- > 0 && !out.bad())
      out.put(out.fill());

  // output digits
  out.write(&*digits, buffer.end() - digits);

  // padding at the right up to target width in case of left adjustment
  if (flags & std::ios_base::left)
    while (padding-- > 0 && !out.bad())
      out.put(out.fill());

  return out;
}

//------------------------------------------------------------------------------

erthink_u128_constexpr14 std::tuple<const char *, uint128_t, std::errc>
uint128_t::from_chars(const char *first, const char *last,
                      unsigned base) noexcept {
  auto scan = first;
  if (!base) {
    base = 10;
    if (scan + 1 < last && scan[0] == '0') {
      const unsigned is_hex = (scan[1] | 32) == 'x';
      scan += is_hex + 1;
      base = 8 << is_hex;
    }
  }

  uint128_t result(0);
  if (unlikely(base < 2 || base > 36))
    return std::tuple<const char *, uint128_t, std::errc>(
        nullptr, result, std::errc::invalid_argument);

  int_fast8_t state = 0 /* no pattern match case */;
  while (likely(scan < last)) {
    const auto digit = details::char2digit(*scan);
    if (unlikely(digit >= base))
      break;
    const auto next = result * uint128_t(base) + uint128_t(digit);
    state |= likely(next >= result) ? 1 /* pattern match */
                                    : -1 /* overflow case */;
    result = next;
    ++scan;
  }

  const std::errc rc = likely(state > 0)
                           ? std::errc()
                           : (state ? std::errc::result_out_of_range
                                    : std::errc::invalid_argument);
  return std::tuple<const char *, uint128_t, std::errc>(scan, result, rc);
}

erthink_u128_constexpr14 uint128_t uint128_t::from_string(const char *begin,
                                                          const char *end,
                                                          unsigned base) {
  CONSTEXPR_ASSERT(end >= begin);
  const auto tuple = from_chars(begin, end, base);
  return (std::get<2>(tuple) == std::errc() && std::get<0>(tuple) == end)
             ? std::get<1>(tuple)
             : (((std::get<2>(tuple) == std::errc::result_out_of_range)
                     ? throw std::out_of_range(begin)
                 : std::get<0>(tuple)
                     ? throw std::invalid_argument(begin)
                     : throw std::invalid_argument("invalid base"),
                 std::get<1>(tuple)));
}

erthink_u128_constexpr14 uint128_t operator"" _u128(const char *str) {
  return uint128_t::from_string(str);
}

} // namespace erthink

#endif /* __cplusplus */

#ifdef _MSC_VER
#pragma warning(pop)
#endif
