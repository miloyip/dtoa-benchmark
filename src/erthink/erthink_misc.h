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

#ifdef _MSC_VER
#pragma warning(push, 1)
#endif
#include <cinttypes>
#include <climits>
#include <cstddef>
#include <type_traits>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace erthink {

template <typename T> struct branchless_abs {
  typedef typename std::make_signed<T>::type signed_type;
  typedef typename std::make_unsigned<T>::type unsigned_type;
  const signed_type expanded_sign;
  const unsigned_type unsigned_abs;
  constexpr branchless_abs(const T value)
      : expanded_sign(signed_type(value) >>
                      (sizeof(signed_type) * CHAR_BIT - 1)),
        unsigned_abs(unsigned_type((signed_type(value) + expanded_sign) ^
                                   expanded_sign)) {
    static_assert(((INT32_MIN >> 5) >> 29) == -1,
                  "requires arithmetic shift with sign expansion");
  }

  /* MSVC compiler is crazy... */
  branchless_abs(const branchless_abs &) = delete;
  branchless_abs &operator=(const branchless_abs &) = delete;
};

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100) /* unreferenced formal parameter */
#pragma warning(disable : 4514) /* unreferenced inline function                \
                                   has been removed */
#endif
template <typename TYPE, std::size_t LENGTH>
constexpr std::size_t array_length(const TYPE __maybe_unused (&array)[LENGTH]) {
  return LENGTH;
}

template <typename TYPE, std::size_t LENGTH>
constexpr const TYPE *array_end(const TYPE (&array)[LENGTH]) {
  return array + LENGTH;
}

static inline constexpr bool msb(const uint64_t value) {
  static_assert(static_cast<int64_t>(UINT64_C(1) << 63) < 0,
                "2-complement representation required");
  return static_cast<int64_t>(value) < 0;
}

static inline constexpr bool msb(const uint32_t value) {
  static_assert(static_cast<int32_t>(UINT32_C(1) << 31) < 0,
                "2-complement representation required");
  return static_cast<int32_t>(value) < 0;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace erthink
