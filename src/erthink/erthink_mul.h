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
#include "erthink_carryadd.h"
#include "erthink_defs.h"
#include "erthink_intrin.h"

/* TODO: refactoring defines to C++ functions and templateds */

namespace erthink {

#if defined(__e2k__) && __iset__ >= 3
#define mul_64x64_high(a, b) __builtin_e2k_umulhd(a, b)
#endif /* __e2k__ Elbrus && __iset__ >= 3 */

#if defined(_M_X64) || defined(_M_IA64) || defined(_M_AMD64)
#pragma intrinsic(_umul128)
#define mul_64x64_128(a, b, ph) _umul128(a, b, ph)
#endif

#if defined(_M_ARM64) || defined(_M_X64) || defined(_M_IA64)
#pragma intrinsic(__umulh)
#define mul_64x64_high(a, b) __umulh(a, b)
#endif

#if defined(_M_IX86)
#pragma intrinsic(__emulu)
#define mul_32x32_64(a, b) __emulu(a, b)

#elif defined(_M_ARM)
#define mul_32x32_64(a, b) _arm_umull(a, b)
#endif

#ifndef mul_32x32_64
static constexpr __always_inline uint64_t mul_32x32_64(uint32_t a, uint32_t b) {
  return a * (uint64_t)b;
}
#endif /* mul_32x32_64 */

#ifndef mul_64x64_128
static __maybe_unused __always_inline uint64_t mul_64x64_128(uint64_t a,
                                                             uint64_t b,
                                                             uint64_t *high) {
#if defined(__SIZEOF_INT128__) ||                                              \
    (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128)
  __uint128_t r = (__uint128_t)a * (__uint128_t)b;
  /* modern GCC could nicely optimize this */
  *high = (uint64_t)(r >> 64);
  return (uint64_t)r;
#elif defined(mul_64x64_high)
  *high = mul_64x64_high(a, b);
  return a * b;
#else
  /* performs 64x64 to 128 bit multiplication */
  const uint64_t ll = mul_32x32_64((uint32_t)a, (uint32_t)b);
  const uint64_t lh = mul_32x32_64(a >> 32, (uint32_t)b);
  const uint64_t hl = mul_32x32_64((uint32_t)a, b >> 32);
  const uint64_t hh = mul_32x32_64(a >> 32, b >> 32);

  uint64_t low;
  add64carry_last(add64carry_first(ll, lh << 32, &low), hh, lh >> 32, high);
  add64carry_last(add64carry_first(low, hl << 32, &low), *high, hl >> 32, high);
  return low;
#endif
}
#endif /* mul_64x64_128() */

} // namespace erthink
