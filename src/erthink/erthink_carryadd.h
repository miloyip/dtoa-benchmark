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
#include "erthink_defs.h"
#include "erthink_intrin.h"

/* TODO: refactoring defines to C++ functions and templateds */

#ifdef __cplusplus
namespace erthink {
#endif

#if defined(__e2k__) && __iset__ >= 5

static __maybe_unused __always_inline unsigned
e2k_add64carry_first(uint64_t base, uint64_t addend, uint64_t *sum) {
  *sum = base + addend;
  return __builtin_e2k_addcd_c(base, addend, 0);
}
#define add64carry_first(base, addend, sum)                                    \
  e2k_add64carry_first(base, addend, sum)

static __maybe_unused __always_inline unsigned
e2k_add64carry_next(unsigned carry, uint64_t base, uint64_t addend,
                    uint64_t *sum) {
  *sum = __builtin_e2k_addcd(base, addend, carry);
  return __builtin_e2k_addcd_c(base, addend, carry);
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

// ----------------------------------------------------------------------------

static __maybe_unused __always_inline unsigned
e2k_sub64borrow_first(uint64_t base, uint64_t subtrahend, uint64_t *diff) {
  *diff = base - subtrahend;
  return __builtin_e2k_subcd_c(base, subtrahend, 0);
}
#define sub64borrow_first(base, subtrahend, diff)                              \
  e2k_sub64borrow_first(base, subtrahend, diff)

static __maybe_unused __always_inline unsigned
e2k_sub64borrow_next(unsigned borrow, uint64_t base, uint64_t subtrahend,
                     uint64_t *diff) {
  *diff = __builtin_e2k_subcd(base, subtrahend, borrow);
  return __builtin_e2k_subcd_c(base, subtrahend, borrow);
}
#define sub64borrow_next(borrow, base, subtrahend, diff)                       \
  e2k_sub64borrow_next(borrow, base, subtrahend, diff)

static __maybe_unused __always_inline void
e2k_sub64borrow_last(unsigned borrow, uint64_t base, uint64_t subtrahend,
                     uint64_t *diff) {
  *diff = __builtin_e2k_subcd(base, subtrahend, borrow);
}
#define sub64borrow_last(borrow, base, subtrahend, diff)                       \
  e2k_sub64borrow_last(borrow, base, subtrahend, diff)

#endif /* __e2k__ &&  __iset__ >= 5 */

//------------------------------------------------------------------------------

#if !defined(__clang__) &&                                                     \
    (defined(_M_X64) || defined(_M_IA64) || defined(_M_AMD64))

#pragma intrinsic(_addcarry_u64)
#define add64carry_first(base, addend, sum) _addcarry_u64(0, base, addend, sum)
#define add64carry_next(carry, base, addend, sum)                              \
  _addcarry_u64(carry, base, addend, sum)
#define add64carry_last(carry, base, addend, sum)                              \
  (void)_addcarry_u64(carry, base, addend, sum)

#pragma intrinsic(_subborrow_u64)
#define sub64borrow_first(base, subtrahend, diff)                              \
  _subborrow_u64(0, base, subtrahend, diff)
#define sub64borrow_next(borrow, base, subtrahend, diff)                       \
  _subborrow_u64(borrow, base, subtrahend, diff)
#define sub64borrow_last(borrow, base, subtrahend, diff)                       \
  (void)_subborrow_u64(borrow, base, subtrahend, diff)

#elif !defined(__clang__) && defined(_M_IX86) &&                               \
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

#pragma intrinsic(_subborrow_u32)
#define sub32borrow_first(base, subtrahend, diff)                              \
  _subborrow_u32(0, base, subtrahend, diff)
#define sub32borrow_next(borrow, base, subtrahend, diff)                       \
  _subborrow_u32(borrow, base, subtrahend, diff)
#define sub32borrow_last(borrow, base, subtrahend, diff)                       \
  (void)_subborrow_u32(borrow, base, subtrahend, diff)

static __always_inline unsigned char
msvc32_sub64borrow_first(uint64_t base, uint64_t subtrahend, uint64_t *diff) {
  uint32_t *const diff32 = (uint32_t *)diff;
  const uint32_t base_32l = (uint32_t)base;
  const uint32_t base_32h = (uint32_t)(base >> 32);
  const uint32_t subtrahend_32l = (uint32_t)subtrahend;
  const uint32_t subtrahend_32h = (uint32_t)(subtrahend >> 32);
  return sub32borrow_next(sub32borrow_first(base_32l, subtrahend_32l, diff32),
                          base_32h, subtrahend_32h, diff32 + 1);
}
#define sub64borrow_first(base, subtrahend, diff)                              \
  msvc32_sub64borrow_first(base, subtrahend, diff)

static __always_inline unsigned char
msvc32_sub64borrow_next(unsigned char borrow, uint64_t base,
                        uint64_t subtrahend, uint64_t *diff) {
  uint32_t *const diff32 = (uint32_t *)diff;
  const uint32_t base_32l = (uint32_t)base;
  const uint32_t base_32h = (uint32_t)(base >> 32);
  const uint32_t subtrahend_32l = (uint32_t)subtrahend;
  const uint32_t subtrahend_32h = (uint32_t)(subtrahend >> 32);
  return sub32borrow_next(
      sub32borrow_next(borrow, base_32l, subtrahend_32l, diff32), base_32h,
      subtrahend_32h, diff32 + 1);
}
#define sub64borrow_next(borrow, base, subtrahend, diff)                       \
  msvc32_sub64borrow_next(borrow, base, subtrahend, diff)

static __always_inline void msvc32_sub64borrow_last(unsigned char borrow,
                                                    uint64_t base,
                                                    uint64_t subtrahend,
                                                    uint64_t *diff) {
  uint32_t *const diff32 = (uint32_t *)diff;
  const uint32_t base_32l = (uint32_t)base;
  const uint32_t base_32h = (uint32_t)(base >> 32);
  const uint32_t subtrahend_32l = (uint32_t)subtrahend;
  const uint32_t subtrahend_32h = (uint32_t)(subtrahend >> 32);
  sub32borrow_last(sub32borrow_next(borrow, base_32l, subtrahend_32l, diff32),
                   base_32h, subtrahend_32h, diff32 + 1);
}
#define sub64borrow_last(borrow, base, subtrahend, diff)                       \
  msvc32_sub64borrow_last(borrow, base, subtrahend, diff)

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
  addend += carry;
  *sum = base + addend;
  return addend < carry || *sum < base;
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

//------------------------------------------------------------------------------

#ifndef sub64borrow_first
static __maybe_unused __always_inline unsigned
sub64borrow_first(uint64_t base, uint64_t subtrahend, uint64_t *diff) {
#if __GNUC_PREREQ(5, 0) || __has_builtin(__builtin_sub_overflow)
  return __builtin_sub_overflow(base, subtrahend, diff);
#elif __has_builtin(__builtin_subcll)
  unsigned long long borrowout;
  *diff = __builtin_subcll(base, subtrahend, 0, &borrowout);
  return (unsigned)borrowout;
#else
  *diff = base - subtrahend;
  return base < subtrahend;
#endif /* __has_builtin(__builtin_subcll) */
}
#endif /* sub64borrow_fist */

#ifndef sub64borrow_next
static __maybe_unused __always_inline unsigned
sub64borrow_next(unsigned borrow, uint64_t base, uint64_t subtrahend,
                 uint64_t *diff) {
#if __has_builtin(__builtin_subcll)
  unsigned long long borrowout;
  *diff = __builtin_subcll(base, subtrahend, borrow, &borrowout);
  return (unsigned)borrowout;
#else
  subtrahend += borrow;
  *diff = base - subtrahend;
  return subtrahend < borrow || base < subtrahend;
#endif /* __has_builtin(__builtin_subcll) */
}
#endif /* sub64borrow_next */

#ifndef sub64borrow_last
static __maybe_unused __always_inline void sub64borrow_last(unsigned borrow,
                                                            uint64_t base,
                                                            uint64_t subtrahend,
                                                            uint64_t *diff) {
#if __has_builtin(__builtin_subcll)
  unsigned long long borrowout;
  *diff = __builtin_subcll(base, subtrahend, borrow, &borrowout);
  (void)borrowout;
#else
  *diff = base - subtrahend - borrow;
#endif /* __has_builtin(__builtin_subcll) */
}
#endif /* sub64borrow_last */

//------------------------------------------------------------------------------

#ifdef __cplusplus
} // namespace erthink
#endif
