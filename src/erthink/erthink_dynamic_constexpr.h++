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

#include <type_traits>

namespace erthink {

cxx11_constexpr bool is_constant_evaluated() cxx11_noexcept {
#if defined(__cpp_lib_is_constant_evaluated)
  return std::is_constant_evaluated();
#elif __GNUC_PREREQ(9, 0) ||                                                   \
    (__has_builtin(__builtin_is_constant_evaluated) &&                         \
     (!defined(__LCC__) || __LCC__ > 125))
  return ::__builtin_is_constant_evaluated();
#else
  return false;
#endif
}

#if defined(__cpp_lib_is_constant_evaluated) &&                                \
    __cpp_lib_is_constant_evaluated >= 201811L

#define erthink_dynamic_constexpr cxx14_constexpr
#define ERTHINK_DYNAMIC_CONSTEXPR(RESULT_TYPE, NAME, DECLARGS_PARENTHESIZED,   \
                                  CALLARGS_PARENTHESIZED, PROBE_ARG)           \
  static cxx14_constexpr RESULT_TYPE NAME DECLARGS_PARENTHESIZED               \
      cxx11_noexcept {                                                         \
    return ::std::is_constant_evaluated()                                      \
               ? NAME##_constexpr CALLARGS_PARENTHESIZED                       \
               : NAME##_dynamic CALLARGS_PARENTHESIZED;                        \
  }

#elif __GNUC_PREREQ(9, 0) ||                                                   \
    (__has_builtin(__builtin_is_constant_evaluated) &&                         \
     (!defined(__LCC__) || __LCC__ > 125))

#define erthink_dynamic_constexpr cxx14_constexpr
#define ERTHINK_DYNAMIC_CONSTEXPR(RESULT_TYPE, NAME, DECLARGS_PARENTHESIZED,   \
                                  CALLARGS_PARENTHESIZED, PROBE_ARG)           \
  static cxx14_constexpr RESULT_TYPE NAME DECLARGS_PARENTHESIZED               \
      cxx11_noexcept {                                                         \
    return __builtin_is_constant_evaluated()                                   \
               ? NAME##_constexpr CALLARGS_PARENTHESIZED                       \
               : NAME##_dynamic CALLARGS_PARENTHESIZED;                        \
  }

#elif __GNUC_PREREQ(5, 4) || __has_builtin(__builtin_constant_p)

#define erthink_dynamic_constexpr cxx14_constexpr
#define ERTHINK_DYNAMIC_CONSTEXPR(RESULT_TYPE, NAME, DECLARGS_PARENTHESIZED,   \
                                  CALLARGS_PARENTHESIZED, PROBE_ARG)           \
  static cxx14_constexpr RESULT_TYPE NAME DECLARGS_PARENTHESIZED               \
      cxx11_noexcept {                                                         \
    return __builtin_constant_p(PROBE_ARG)                                     \
               ? NAME##_constexpr CALLARGS_PARENTHESIZED                       \
               : NAME##_dynamic CALLARGS_PARENTHESIZED;                        \
  }

#else

#define erthink_dynamic_constexpr inline
#define ERTHINK_DYNAMIC_CONSTEXPR(RESULT_TYPE, NAME, DECLARGS_PARENTHESIZED,   \
                                  CALLARGS_PARENTHESIZED, PROBE_ARG)           \
  static inline RESULT_TYPE NAME DECLARGS_PARENTHESIZED cxx11_noexcept {       \
    return NAME##_dynamic CALLARGS_PARENTHESIZED;                              \
  }

#endif

} // namespace erthink
