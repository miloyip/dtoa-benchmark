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

/* TODO: refactoring defines to C++ functions and templateds */

namespace erthink {

#if defined(__e2k__) && __iset__ >= 5

static __maybe_unused __always_inline unsigned
e2k_add64carry_first(uint64_t base, uint64_t addend, uint64_t *sum) {
  *sum = base + addend;
  return (unsigned)__builtin_e2k_addcd_c(base, addend, 0);
}
#define add64carry_first(base, addend, sum)                                    \
  e2k_add64carry_first(base, addend, sum)

static __maybe_unused __always_inline unsigned
e2k_add64carry_next(unsigned carry, uint64_t base, uint64_t addend,
                    uint64_t *sum) {
  *sum = __builtin_e2k_addcd(base, addend, carry);
  return (unsigned)__builtin_e2k_addcd_c(base, addend, carry);
}
#define add64carry_next(carry, base, addend, sum)                              \
  e2k_add64carry_next(carry, base, addend, sum)

static __maybe_unused __always_inline void e2k_add64carry_last(unsigned carry,
                                                               uint64_t base,
                                                               uint64_t addend,
                                                               uint64_t *sum) {
  *sum = __builtin_e2k_addcd(base, addend, carry);
}
#define add64carry_last(carry, base, addend, sum)                              \
  e2k_add64carry_last(carry, base, addend, sum)

#endif /* __e2k__ Elbrus &&  __iset__ >= 5 */

//------------------------------------------------------------------------------

#if defined(_M_X64) || defined(_M_IA64) || defined(_M_AMD64)

#pragma intrinsic(_addcarry_u64)
#define add64carry_first(base, addend, sum) _addcarry_u64(0, base, addend, sum)
#define add64carry_next(carry, base, addend, sum)                              \
  _addcarry_u64(carry, base, addend, sum)
#define add64carry_last(carry, base, addend, sum)                              \
  (void)_addcarry_u64(carry, base, addend, sum)

#elif defined(_M_IX86) &&                                                      \
    _MSC_VER >= 1915 /* LY: workaround for SSA-optimizer bug */

#pragma intrinsic(_addcarry_u32)
#define add32carry_first(base, addend, sum) _addcarry_u32(0, base, addend, sum)
#define add32carry_next(carry, base, addend, sum)                              \
  _addcarry_u32(carry, base, addend, sum)
#define add32carry_last(carry, base, addend, sum)                              \
  (void)_addcarry_u32(carry, base, addend, sum)

static __always_inline unsigned char
msvc32_add64carry_first(uint64_t base, uint64_t addend, uint64_t *sum) {
  uint32_t *const sum32 = (uint32_t *)sum;
  const uint32_t base_32l = (uint32_t)base;
  const uint32_t base_32h = (uint32_t)(base >> 32);
  const uint32_t addend_32l = (uint32_t)addend;
  const uint32_t addend_32h = (uint32_t)(addend >> 32);
  return add32carry_next(add32carry_first(base_32l, addend_32l, sum32),
                         base_32h, addend_32h, sum32 + 1);
}
#define add64carry_first(base, addend, sum)                                    \
  msvc32_add64carry_first(base, addend, sum)

static __always_inline unsigned char msvc32_add64carry_next(unsigned char carry,
                                                            uint64_t base,
                                                            uint64_t addend,
                                                            uint64_t *sum) {
  uint32_t *const sum32 = (uint32_t *)sum;
  const uint32_t base_32l = (uint32_t)base;
  const uint32_t base_32h = (uint32_t)(base >> 32);
  const uint32_t addend_32l = (uint32_t)addend;
  const uint32_t addend_32h = (uint32_t)(addend >> 32);
  return add32carry_next(add32carry_next(carry, base_32l, addend_32l, sum32),
                         base_32h, addend_32h, sum32 + 1);
}
#define add64carry_next(carry, base, addend, sum)                              \
  msvc32_add64carry_next(carry, base, addend, sum)

static __always_inline void msvc32_add64carry_last(unsigned char carry,
                                                   uint64_t base,
                                                   uint64_t addend,
                                                   uint64_t *sum) {
  uint32_t *const sum32 = (uint32_t *)sum;
  const uint32_t base_32l = (uint32_t)base;
  const uint32_t base_32h = (uint32_t)(base >> 32);
  const uint32_t addend_32l = (uint32_t)addend;
  const uint32_t addend_32h = (uint32_t)(addend >> 32);
  add32carry_last(add32carry_next(carry, base_32l, addend_32l, sum32), base_32h,
                  addend_32h, sum32 + 1);
}
#define add64carry_last(carry, base, addend, sum)                              \
  msvc32_add64carry_last(carry, base, addend, sum)

#endif /* _M_IX86 && _MSC_VER >= 1915 */

//------------------------------------------------------------------------------

#ifndef add64carry_first
static __maybe_unused __always_inline unsigned
add64carry_first(uint64_t base, uint64_t addend, uint64_t *sum) {
#if __GNUC_PREREQ(5, 0) || __has_builtin(__builtin_add_overflow)
  return __builtin_add_overflow(base, addend, sum);
#elif __has_builtin(__builtin_addcll)
  unsigned long long carryout;
  *sum = __builtin_addcll(base, addend, 0, &carryout);
  return (unsigned)carryout;
#else
  *sum = base + addend;
  return *sum < addend;
#endif /* __has_builtin(__builtin_addcll) */
}
#endif /* add64carry_fist */

#ifndef add64carry_next
static __maybe_unused __always_inline unsigned
add64carry_next(unsigned carry, uint64_t base, uint64_t addend, uint64_t *sum) {
#if __has_builtin(__builtin_addcll)
  unsigned long long carryout;
  *sum = __builtin_addcll(base, addend, carry, &carryout);
  return (unsigned)carryout;
#else
  *sum = base + addend + carry;
  return *sum < addend || (carry && *sum == addend);
#endif /* __has_builtin(__builtin_addcll) */
}
#endif /* add64carry_next */

#ifndef add64carry_last
static __maybe_unused __always_inline void
add64carry_last(unsigned carry, uint64_t base, uint64_t addend, uint64_t *sum) {
#if __has_builtin(__builtin_addcll)
  unsigned long long carryout;
  *sum = __builtin_addcll(base, addend, carry, &carryout);
  (void)carryout;
#else
  *sum = base + addend + carry;
#endif /* __has_builtin(__builtin_addcll) */
}
#endif /* add64carry_last */

} // namespace erthink
