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

#if !defined(__cplusplus) || __cplusplus < 201103L
#error "This source code requires C++11 at least."
#endif

#include "erthink_arch.h"
#include "erthink_defs.h"
#include "erthink_intrin.h"

#ifdef _MSC_VER
#pragma warning(push, 1)
#endif
#include <cassert>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace erthink {

template <typename T> cxx11_constexpr int clz(T v);

static __maybe_unused inline int fallback_clz32(uint32_t v) {
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  static const int8_t deBruijn_clz32[32] = {
      31, 22, 30, 21, 18, 10, 29, 2,  20, 17, 15, 13, 9, 6,  28, 1,
      23, 19, 11, 3,  16, 14, 7,  24, 12, 4,  8,  25, 5, 26, 27, 0};
  return deBruijn_clz32[v * UINT32_C(0x07C4ACDD) >> 27];
}

static __maybe_unused inline int fallback_clz64(uint64_t v) {
#ifdef ERTHINK_ARCH32
  const uint32_t hi = static_cast<uint32_t>(v >> 32);
  return (hi ? 0 : 32) + fallback_clz32(hi ? hi : static_cast<uint32_t>(v));
#else
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v |= v >> 32;
  static const uint8_t deBruijn_clz64[64] = {
      63, 16, 62, 7,  15, 36, 61, 3,  6,  14, 22, 26, 35, 47, 60, 2,
      9,  5,  28, 11, 13, 21, 42, 19, 25, 31, 34, 40, 46, 52, 59, 1,
      17, 8,  37, 4,  23, 27, 48, 10, 29, 12, 43, 20, 32, 41, 53, 18,
      38, 24, 49, 30, 44, 33, 54, 39, 50, 45, 55, 51, 56, 57, 58, 0};
  return deBruijn_clz64[v * UINT64_C(0x03F79D71B4CB0A89) >> 58];
#endif
}

#if defined(__GNUC__) || defined(__clang__)

template <> cxx11_constexpr int clz<unsigned>(unsigned v) {
  return __builtin_clz(v);
}
template <> cxx11_constexpr int clz<unsigned long>(unsigned long v) {
  return __builtin_clzl(v);
}
template <> cxx11_constexpr int clz<unsigned long long>(unsigned long long v) {
  return __builtin_clzll(v);
}

#elif defined(_MSC_VER) && !defined(__clang__)

#pragma intrinsic(_BitScanReverse)
template <> inline int clz<uint32_t>(uint32_t v) {
  unsigned long index;
  assert(v > 0);
  _BitScanReverse(&index, v);
  return 31 - (int)index;
}

#ifdef ERTHINK_ARCH64
#pragma intrinsic(_BitScanReverse64)
template <> inline int clz<uint64_t>(uint64_t v) {
  unsigned long index;
  assert(v > 0);
  _BitScanReverse64(&index, v);
  return 63 - (int)index;
}
#else
template <> inline int clz<uint64_t>(uint64_t v) {
  return (v > UINT32_MAX) ? clz(static_cast<uint32_t>(v >> 32))
                          : 32 + clz(static_cast<uint32_t>(v));
}
#endif /* ARCH 32/64 */

#else /* fallback */

template <> inline int clz<uint32_t>(uint32_t v) { return fallback_clz32(v); }
template <> inline int clz<uint64_t>(uint64_t v) { return fallback_clz64(v); }

#endif /* compiler */

static __maybe_unused __always_inline int clz64(uint64_t v) { return clz(v); }

static __maybe_unused __always_inline int clz32(uint32_t v) { return clz(v); }

} // namespace erthink
