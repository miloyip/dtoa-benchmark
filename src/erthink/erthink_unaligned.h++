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

#include "erthink_arch.h"
#include "erthink_defs.h"

#include "erthink_constexpr_cstr.h++"

#ifndef ERTHINK_UNALIGNED_OK
#if defined(ENABLE_UBSAN)
#define ERTHINK_UNALIGNED_OK 0
#elif __CLANG_PREREQ(5, 0) || __GNUC_PREREQ(5, 0)
#define ERTHINK_UNALIGNED_OK 0 /* expecting optimization is well done */
#elif defined(_MSC_VER)
#define ERTHINK_UNALIGNED_OK 1 /* avoid MSVC misoptimization */
#elif (defined(__ia32__) || defined(__ARM_FEATURE_UNALIGNED)) &&               \
    !defined(__ALIGNED__)
#define ERTHINK_UNALIGNED_OK 1
#else
#define ERTHINK_UNALIGNED_OK 0
#endif
#endif /* ERTHINK_UNALIGNED_OK */

namespace erthink {

#if __GNUC_PREREQ(5, 0) || !defined(_GCC_MAX_ALIGN_T)
using max_align_t = std::max_align_t;
#else
using max_align_t = ::max_align_t;
#endif /* workaround for std::max_align_t */

template <typename T, unsigned expected_alignment = 1>
static T cxx20_constexpr peek_unaligned(const T *source) noexcept {
  constexpr auto required_alignment =
      std::min(sizeof(T), std::max(alignof(T), alignof(max_align_t)));
  if (ERTHINK_UNALIGNED_OK || expected_alignment >= required_alignment)
    return *source;
  else {
    T result;
    memcpy(&result, source, sizeof(T));
    return result;
  }
}

template <typename T, unsigned expected_alignment = 1>
static T cxx20_constexpr poke_unaligned(T *target, const T &source) noexcept {
  constexpr auto required_alignment =
      std::min(sizeof(T), std::max(alignof(T), alignof(max_align_t)));
  if (ERTHINK_UNALIGNED_OK || expected_alignment >= required_alignment)
    return *target = source;
  else {
    memcpy(target, &source, sizeof(T));
    return source;
  }
}

} // namespace erthink
