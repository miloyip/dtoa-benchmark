/*
 *  Copyright (c) 1994-2019 Leonid Yuriev <leo@yuriev.ru>.
 *  https://github.com/leo-yuriev/erthink
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

template <typename T> inline constexpr int clz(T v);

static inline int fallback_clz8(uint8_t v) {
  static const int8_t lut[256] = {
      8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  return lut[v];
}

static inline int fallback_clz32(uint32_t v) {
  // LY: strive for branchless (SSA-optimizer must solve this)
  int r = 24, s = (v > 0xFFFF) << 4;
  v >>= s;
  r -= s;

  s = (v > 0xFF) << 3;
  v >>= s;
  r -= s;

  return r + fallback_clz8(static_cast<uint8_t>(v));
}

static inline int fallback_clz64(uint64_t v) {
#ifdef ERTHINK_ARCH32
  const uint32_t hi = static_cast<uint32_t>(v >> 32);
  return (hi ? 0 : 32) + fallback_clz32(hi ? hi : static_cast<uint32_t>(v));
#else
  // LY: strive for branchless (SSA-optimizer must solve this)
  const int s = (v > UINT32_C(0xFFFFffff)) << 5;
  return 32 - s + fallback_clz32(static_cast<uint32_t>(v >> s));
#endif
}

#ifdef __GNUC__

template <> inline constexpr int clz<unsigned>(unsigned v) {
  return __builtin_clz(v);
}
template <> inline constexpr int clz<unsigned long>(unsigned long v) {
  return __builtin_clzl(v);
}
template <> inline constexpr int clz<unsigned long long>(unsigned long long v) {
  return __builtin_clzll(v);
}

#elif defined(_MSC_VER)

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

static __always_inline int clz64(uint64_t v) { return clz(v); }

static __always_inline int clz32(uint32_t v) { return clz(v); }

} // namespace erthink
