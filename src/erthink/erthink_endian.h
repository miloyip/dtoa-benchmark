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

#include "erthink_arch.h"
#include "erthink_bswap.h"
#include "erthink_byteorder.h"
#include "erthink_defs.h"

#if !defined(__BYTE_ORDER__) || !defined(__ORDER_LITTLE_ENDIAN__) ||           \
    !defined(__ORDER_BIG_ENDIAN__)
#error __BYTE_ORDER__ should be defined.
#endif

#pragma push_macro("htobe16")
#pragma push_macro("htole16")
#pragma push_macro("be16toh")
#pragma push_macro("le16toh")

#pragma push_macro("htobe32")
#pragma push_macro("htole32")
#pragma push_macro("be32toh")
#pragma push_macro("le32toh")

#pragma push_macro("htobe64")
#pragma push_macro("htole64")
#pragma push_macro("be64toh")
#pragma push_macro("le64toh")

#undef htobe16
#undef htole16
#undef be16toh
#undef le16toh

#undef htobe32
#undef htole32
#undef be32toh
#undef le32toh

#undef htobe64
#undef htole64
#undef be64toh
#undef le64toh

//------------------------------------------------------------------------------

#ifdef __cplusplus
namespace erthink {
enum class endian {
  little = __ORDER_LITTLE_ENDIAN__,
  big = __ORDER_BIG_ENDIAN__,
  native = __BYTE_ORDER__
};
#endif /* __cplusplus */

/* *INDENT-OFF* */
/* clang-format off */

#if __BYTE_ORDER == __LITTLE_ENDIAN

static constexpr_intrin uint16_t ERTHINK_NAME_PREFIX(htobe16)(uint16_t x) { return ERTHINK_NAME_PREFIX(bswap16)(x); }
static constexpr_intrin uint16_t ERTHINK_NAME_PREFIX(htole16)(uint16_t x) { return x; }
static constexpr_intrin uint16_t ERTHINK_NAME_PREFIX(be16toh)(uint16_t x) { return ERTHINK_NAME_PREFIX(bswap16)(x); }
static constexpr_intrin uint16_t ERTHINK_NAME_PREFIX(le16toh)(uint16_t x) { return x; }

static constexpr_intrin uint32_t ERTHINK_NAME_PREFIX(htobe32)(uint32_t x) { return ERTHINK_NAME_PREFIX(bswap32)(x); }
static constexpr_intrin uint32_t ERTHINK_NAME_PREFIX(htole32)(uint32_t x) { return x; }
static constexpr_intrin uint32_t ERTHINK_NAME_PREFIX(be32toh)(uint32_t x) { return ERTHINK_NAME_PREFIX(bswap32)(x); }
static constexpr_intrin uint32_t ERTHINK_NAME_PREFIX(le32toh)(uint32_t x) { return x; }

static constexpr_intrin uint64_t ERTHINK_NAME_PREFIX(htobe64)(uint64_t x) { return ERTHINK_NAME_PREFIX(bswap64)(x); }
static constexpr_intrin uint64_t ERTHINK_NAME_PREFIX(htole64)(uint64_t x) { return x; }
static constexpr_intrin uint64_t ERTHINK_NAME_PREFIX(be64toh)(uint64_t x) { return ERTHINK_NAME_PREFIX(bswap64)(x); }
static constexpr_intrin uint64_t ERTHINK_NAME_PREFIX(le64toh)(uint64_t x) { return x; }

#else

static constexpr_intrin uint16_t ERTHINK_NAME_PREFIX(htobe16)(uint16_t x) { return x; }
static constexpr_intrin uint16_t ERTHINK_NAME_PREFIX(htole16)(uint16_t x) { return ERTHINK_NAME_PREFIX(bswap16)(x); }
static constexpr_intrin uint16_t ERTHINK_NAME_PREFIX(be16toh)(uint16_t x) { return x; }
static constexpr_intrin uint16_t ERTHINK_NAME_PREFIX(le16toh)(uint16_t x) { return ERTHINK_NAME_PREFIX(bswap16)(x); }

static constexpr_intrin uint32_t ERTHINK_NAME_PREFIX(htobe32)(uint32_t x) { return x; }
static constexpr_intrin uint32_t ERTHINK_NAME_PREFIX(htole32)(uint32_t x) { return ERTHINK_NAME_PREFIX(bswap32)(x); }
static constexpr_intrin uint32_t ERTHINK_NAME_PREFIX(be32toh)(uint32_t x) { return x; }
static constexpr_intrin uint32_t ERTHINK_NAME_PREFIX(le32toh)(uint32_t x) { return ERTHINK_NAME_PREFIX(bswap32)(x); }

static constexpr_intrin uint64_t ERTHINK_NAME_PREFIX(htobe64)(uint64_t x) { return x; }
static constexpr_intrin uint64_t ERTHINK_NAME_PREFIX(htole64)(uint64_t x) { return ERTHINK_NAME_PREFIX(bswap64)(x); }
static constexpr_intrin uint64_t ERTHINK_NAME_PREFIX(be64toh)(uint64_t x) { return x; }
static constexpr_intrin uint64_t ERTHINK_NAME_PREFIX(le64toh)(uint64_t x) { return ERTHINK_NAME_PREFIX(bswap64)(x); }

#endif

/* *INDENT-ON* */
/* clang-format on */

#ifdef __cplusplus

template <typename T> constexpr_intrin T h2le(T v);
template <typename T> constexpr_intrin T h2be(T v);
template <typename T> constexpr_intrin T le2h(T v);
template <typename T> constexpr_intrin T be2h(T v);

#define HERE_MAKE(FN)                                                          \
  template <> constexpr_intrin uint8_t FN<uint8_t>(uint8_t v) { return v; }    \
  template <> constexpr_intrin int8_t FN<int8_t>(int8_t v) { return v; }
HERE_MAKE(h2le)
HERE_MAKE(h2be)
HERE_MAKE(le2h)
HERE_MAKE(be2h)
#undef HERE_MAKE

#define HERE_MAKE(CASE, WIDTH)                                                 \
  template <>                                                                  \
  constexpr_intrin uint##WIDTH##_t h2##CASE<uint##WIDTH##_t>(                  \
      uint##WIDTH##_t v) {                                                     \
    return hto##CASE##WIDTH(v);                                                \
  }                                                                            \
  template <>                                                                  \
  constexpr_intrin int##WIDTH##_t h2##CASE<int##WIDTH##_t>(int##WIDTH##_t v) { \
    return hto##CASE##WIDTH(v);                                                \
  }                                                                            \
  template <>                                                                  \
      constexpr_intrin uint##WIDTH##_t CASE##2h < uint##WIDTH##_t >            \
      (uint##WIDTH##_t v) {                                                    \
    return CASE##WIDTH##toh(v);                                                \
  }                                                                            \
  template <>                                                                  \
      constexpr_intrin int##WIDTH##_t CASE##2h < int##WIDTH##_t >              \
      (int##WIDTH##_t v) {                                                     \
    return CASE##WIDTH##toh(v);                                                \
  }

HERE_MAKE(le, 16)
HERE_MAKE(le, 32)
HERE_MAKE(le, 64)
HERE_MAKE(be, 16)
HERE_MAKE(be, 32)
HERE_MAKE(be, 64)
#undef HERE_MAKE

} // namespace erthink
#endif /* __cplusplus */

#pragma pop_macro("htobe16")
#pragma pop_macro("htole16")
#pragma pop_macro("be16toh")
#pragma pop_macro("le16toh")

#pragma pop_macro("htobe32")
#pragma pop_macro("htole32")
#pragma pop_macro("be32toh")
#pragma pop_macro("le32toh")

#pragma pop_macro("htobe64")
#pragma pop_macro("htole64")
#pragma pop_macro("be64toh")
#pragma pop_macro("le64toh")
