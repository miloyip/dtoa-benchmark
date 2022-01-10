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

#if !defined(__cplusplus) || __cplusplus < 201103L
#error "This source code requires C++11 at least."
#endif

#include "erthink_misc.h++"

#ifdef _MSC_VER
#pragma warning(disable : 4710) /* function not inlined */
#pragma warning(disable : 4711) /* function selecte for automatic inline */
#pragma warning(push, 1)
#endif
#include <cassert>
#include <cinttypes>
#include <climits>
#include <cstddef>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

//------------------------------------------------------------------------------

namespace erthink {

#ifndef NAMESPACE_ERTHINK_U2A_DETAILS
#define NAMESPACE_ERTHINK_U2A_DETAILS /* anonymous */
#endif                                /* NAMESPACE_ERTHINK_U2A_DETAILS */

namespace NAMESPACE_ERTHINK_U2A_DETAILS {

template <typename T = uint_fast64_t>
static constexpr T power10_recursive_constexpr(const unsigned n) {
  return n ? T(10) * power10_recursive_constexpr<T>(n - 1) : 1;
}

enum : uint32_t {
  UINT_E0 = power10_recursive_constexpr(0),
  UINT_E1 = power10_recursive_constexpr(1),
  UINT_E2 = power10_recursive_constexpr(2),
  UINT_E3 = power10_recursive_constexpr(3),
  UINT_E4 = power10_recursive_constexpr(4),
  UINT_E5 = power10_recursive_constexpr(5),
  UINT_E6 = power10_recursive_constexpr(6),
  UINT_E7 = power10_recursive_constexpr(7),
  UINT_E8 = power10_recursive_constexpr(8),
  UINT_E9 = power10_recursive_constexpr(9)
};

enum : uint64_t {
  UINT_E10 = power10_recursive_constexpr(10),
  UINT_E11 = power10_recursive_constexpr(11),
  UINT_E12 = power10_recursive_constexpr(12),
  UINT_E13 = power10_recursive_constexpr(13),
  UINT_E14 = power10_recursive_constexpr(14),
  UINT_E15 = power10_recursive_constexpr(15),
  UINT_E16 = power10_recursive_constexpr(16),
  UINT_E17 = power10_recursive_constexpr(17),
  UINT_E18 = power10_recursive_constexpr(18),
  UINT_E19 = power10_recursive_constexpr(19)
};

/* LY: Using pairs of digits we asymptotically halves the number of div/mod
 * operations, but keeps this table small and friendly for CPU cache.
 * Usage of wider table is reasonable only for several extraordinary cases. */
static const char digits_00_99[200] = {
    48, 48, 48, 49, 48, 50, 48, 51, 48, 52, 48, 53, 48, 54, 48, 55, 48, 56, 48,
    57, 49, 48, 49, 49, 49, 50, 49, 51, 49, 52, 49, 53, 49, 54, 49, 55, 49, 56,
    49, 57, 50, 48, 50, 49, 50, 50, 50, 51, 50, 52, 50, 53, 50, 54, 50, 55, 50,
    56, 50, 57, 51, 48, 51, 49, 51, 50, 51, 51, 51, 52, 51, 53, 51, 54, 51, 55,
    51, 56, 51, 57, 52, 48, 52, 49, 52, 50, 52, 51, 52, 52, 52, 53, 52, 54, 52,
    55, 52, 56, 52, 57, 53, 48, 53, 49, 53, 50, 53, 51, 53, 52, 53, 53, 53, 54,
    53, 55, 53, 56, 53, 57, 54, 48, 54, 49, 54, 50, 54, 51, 54, 52, 54, 53, 54,
    54, 54, 55, 54, 56, 54, 57, 55, 48, 55, 49, 55, 50, 55, 51, 55, 52, 55, 53,
    55, 54, 55, 55, 55, 56, 55, 57, 56, 48, 56, 49, 56, 50, 56, 51, 56, 52, 56,
    53, 56, 54, 56, 55, 56, 56, 56, 57, 57, 48, 57, 49, 57, 50, 57, 51, 57, 52,
    57, 53, 57, 54, 57, 55, 57, 56, 57, 57};

static __always_inline char *dec2(uint_fast32_t v, char *ptr,
                                  std::size_t force = 0) {
  assert(v < 100u);
  // LY: strive for branchless (SSA-optimizer must solve this)
  *ptr = digits_00_99[v << 1];
  ptr += force | (v > 9);
  *ptr = digits_00_99[(v << 1) + 1];
  return ptr + 1;
}

static __always_inline __maybe_unused char *dec3(uint_fast32_t v, char *ptr,
                                                 std::size_t force = 0) {
  assert(v < 1000u);
  const uint_fast32_t hi = v / 10u;
  const uint_fast32_t lo = v % 10u;
  // LY: strive for branchless (SSA-optimizer must solve this)
  *ptr = digits_00_99[hi << 1];
  ptr += force | (v > 99);
  *ptr = digits_00_99[(hi << 1) + 1];
  ptr += force | (v > 9);
  *ptr = static_cast<char>(lo + '0');
  return ptr + 1;
}

static __always_inline char *dec4(uint_fast32_t v, char *ptr,
                                  std::size_t force = 0) {
  assert(v < 10000u);
  const uint_fast32_t hi = v / 100u;
  const uint_fast32_t lo = v % 100u;
  // LY: strive for branchless (SSA-optimizer must solve this)
  *ptr = digits_00_99[hi << 1];
  ptr += force | (v > 999);
  *ptr = digits_00_99[(hi << 1) + 1];
  ptr += force | (v > 99);
  *ptr = digits_00_99[lo << 1];
  ptr += force | (v > 9);
  *ptr = digits_00_99[(lo << 1) + 1];
  return ptr + 1;
}

} // namespace NAMESPACE_ERTHINK_U2A_DETAILS

static __maybe_unused char *
u2a(uint32_t u32, char *const buffer /* upto 10 chars for 4`294`967`295 */) {
  if (u32 < UINT_E4)
    // fast pass
    return dec4(u32, buffer);

  char *ptr = buffer;
  if (u32 >= UINT_E8) {
    /* LY: no more than 10 digits */
    ptr = dec2(u32 / UINT_E8, ptr);
    u32 %= UINT_E8;
    ptr = dec4(u32 / UINT_E4, ptr, 1);
    u32 %= UINT_E4;
  } else {
    ptr = dec4(u32 / UINT_E4, ptr);
    u32 %= UINT_E4;
  }
  ptr = dec4(u32, ptr, 1);
  assert(ptr - buffer <= 10);
  return ptr;
}

static __maybe_unused char *
u2a(uint64_t u64,
    char *const buffer /* upto 20 chars for 18`446`744`073`709`551`615 */) {
  const uint32_t u32 = static_cast<uint32_t>(u64);
  if (u64 == u32)
    // fast pass
    return u2a(u32, buffer);

  // at least 10 digits
  char *ptr = buffer;
  if (u64 >= UINT_E12) {
    std::size_t force = 0;
    if (unlikely(u64 >= UINT_E16)) {
      /* LY: no more than 20 digits */
      ptr = dec4(static_cast<unsigned>(u64 / UINT_E16), ptr, force);
      u64 %= UINT_E16;
      force = 1;
    }
    ptr = dec4(static_cast<unsigned>(u64 / UINT_E12), ptr, force);
    u64 %= UINT_E12;
    ptr = dec4(static_cast<unsigned>(u64 / UINT_E8), ptr, 1);
    u64 %= UINT_E8;
  } else {
    ptr = dec4(static_cast<unsigned>(u64 / UINT_E8), ptr);
    u64 %= UINT_E8;
  }
  ptr = dec4(static_cast<unsigned>(u64) / 10000u, ptr, 1);
  ptr = dec4(static_cast<unsigned>(u64) % 10000u, ptr, 1);
  assert(ptr - buffer <= 20);
  return ptr;
}

static inline char *
i2a(int32_t i32, char *const buffer /* upto 11 chars for -2`147`483`648 */) {
  char *ptr = buffer;
  const branchless_abs<int32_t> pair(i32);
  // LY: strive for branchless (SSA-optimizer must solve this)
  *ptr = '-';
  ptr = u2a(pair.unsigned_abs, ptr + (pair.expanded_sign & 1));
  assert(ptr - buffer <= 11);
  return ptr;
}

static inline char *
i2a(int64_t i64,
    char *const buffer /* upto 20 chars for -9`223`372`036`854`775`808 */) {
  char *ptr = buffer;
  const branchless_abs<int64_t> pair(i64);
  // LY: strive for branchless (SSA-optimizer must solve this)
  *ptr = '-';
  ptr = u2a(pair.unsigned_abs, ptr + (pair.expanded_sign & 1));
  assert(ptr - buffer <= 20);
  return ptr;
}

} // namespace erthink
