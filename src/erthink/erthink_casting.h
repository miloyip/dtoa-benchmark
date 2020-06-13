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

#include "erthink_defs.h"

#include <type_traits>
#if defined(__cpp_lib_bit_cast) && __cpp_lib_bit_cast >= 201806L
#include <bit>
#define HAVE_std_bit_cast 1
#else
#define HAVE_std_bit_cast 0
#include <cstring>
#endif

#if defined(__cpp_lib_concepts) && __cpp_lib_concepts >= 202002L
#include <concepts>
#endif

namespace erthink {

#if __cplusplus < 201402L
template <bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;
#else
using std::enable_if_t;
#endif /* C++14 */

template <typename TO, typename FROM,
          typename erthink::enable_if_t<std::is_pointer<TO>::value, int> = 0,
          typename erthink::enable_if_t<std::is_pointer<FROM>::value, int> = 0,
          typename erthink::enable_if_t<
              std::is_const<typename std::remove_pointer<FROM>::type>::value,
              int> = 0>
cxx11_constexpr TO constexpr_pointer_cast(FROM from) {
  return static_cast<TO>(static_cast<const void *>(from));
}

template <typename TO, typename FROM,
          typename erthink::enable_if_t<std::is_pointer<TO>::value, int> = 0,
          typename erthink::enable_if_t<std::is_pointer<FROM>::value, int> = 0,
          typename erthink::enable_if_t<
              !std::is_const<typename std::remove_pointer<FROM>::type>::value,
              int> = 0>
cxx11_constexpr TO constexpr_pointer_cast(FROM from) {
  return static_cast<TO>(static_cast<void *>(from));
}

//------------------------------------------------------------------------------

#if HAVE_std_bit_cast

template <class TO, class FROM>
cxx11_constexpr TO bit_cast(const FROM &src) cxx11_noexcept {
  return std::bit_cast<TO, FROM>(src);
}

#else

#if defined(__cpp_concepts) && __cpp_concepts >= 201507L
template <typename TO, typename FROM>
    requires(sizeof(TO) == sizeof(FROM)) &&
    std::is_trivially_copyable<FROM>::value
        &&std::is_trivially_copyable<TO>::value
#else
template <typename TO, typename FROM,
          typename erthink::enable_if_t<sizeof(TO) == sizeof(FROM), int> = 0,
          typename erthink::enable_if_t<std::is_trivially_copyable<FROM>::value,
                                        int> = 0,
          typename erthink::enable_if_t<std::is_trivially_copyable<TO>::value,
                                        int> = 0>
#endif
    cxx11_constexpr TO bit_cast(const FROM &src) cxx11_noexcept {
  static_assert(sizeof(TO) == sizeof(FROM),
                "bit_cast requires source and destination to be the same size");
  static_assert(std::is_trivially_copyable<FROM>::value,
                "bit_cast requires the source type to be trivially copyable");
  static_assert(
      std::is_trivially_copyable<TO>::value,
      "bit_cast requires the destination type to be trivially copyable");
#if __has_builtin(__builtin_bit_cast)
  return __builtin_bit_cast(TO, src);
#else
  typename std::aligned_storage<sizeof(TO), alignof(TO)>::type tmp = {};
#if __has_builtin(__builtin_memcpy)
  __builtin_memcpy(&tmp, &src, sizeof(TO));
#else
  std::memcpy(&tmp, &src, sizeof(TO));
#endif
  return reinterpret_cast<TO &>(tmp);
#endif
}

#endif /* HAVE_std_bit_cast */

} // namespace erthink
