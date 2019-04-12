/*
 *  Copyright (c) 1994-2019 Leonid Yuriev <leo@yuriev.ru>.
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
