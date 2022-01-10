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

#include "erthink_defs.h"
#include "erthink_dynamic_constexpr.h++"

#include <cstring> // for std::strlen, str:memcmp

#if defined(__cpp_lib_string_view) && __cpp_lib_string_view >= 201606L
#include <string_view>
#endif

namespace erthink {

static cxx14_constexpr size_t strlen_constexpr(const char *c_str) noexcept {
  for (size_t i = 0; c_str; ++i)
    if (!c_str[i])
      return i;
  return 0;
}

static inline size_t strlen_dynamic(const char *c_str) noexcept {
  return c_str ? ::std::strlen(c_str) : 0;
}

ERTHINK_DYNAMIC_CONSTEXPR(size_t, strlen, (const char *c_str), (c_str), c_str)

static cxx20_constexpr void *memcpy(void *dest, const void *src,
                                    size_t bytes) noexcept {
#if defined(__cpp_lib_is_constant_evaluated) &&                                \
    __cpp_lib_is_constant_evaluated >= 201811L
  if (::std::is_constant_evaluated()) {
    for (size_t i = 0; i < bytes; ++i)
      static_cast<char *>(dest)[i] = static_cast<const char *>(src)[i];
    return dest;
  } else
#endif /* __cpp_lib_is_constant_evaluated >= 201811 */
    return ::std::memcpy(dest, src, bytes);
}

static cxx20_constexpr int memcmp(const void *a, const void *b,
                                  size_t bytes) noexcept {
#if defined(__cpp_lib_is_constant_evaluated) &&                                \
    __cpp_lib_is_constant_evaluated >= 201811L
  if (::std::is_constant_evaluated()) {
    for (size_t i = 0; i < bytes; ++i) {
      const int diff = static_cast<const unsigned char *>(a)[i] -
                       static_cast<const unsigned char *>(b)[i];
      if (diff)
        return diff;
    }
    return 0;
  } else
#endif /* __cpp_lib_is_constant_evaluated >= 201811 */
    return ::std::memcmp(a, b, bytes);
}

} // namespace erthink
