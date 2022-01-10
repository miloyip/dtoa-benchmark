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

#include "erthink_byteorder.h"
#include "erthink_defs.h"
#include "erthink_intrin.h"

#ifdef __cplusplus
namespace erthink {
#if __cplusplus < 201103L
#error "This source code requires C++11 at least."
#endif
#endif /* __cplusplus */

static cxx14_constexpr uint32_t ror32(uint32_t v, unsigned s) noexcept {
#if defined(_MSC_VER) && !defined(__cplusplus)
  return _rotr(v, s);
#else
  return s &= 31, (v >> s) | (v << (32 - s));
#endif
}

static cxx14_constexpr uint32_t rol32(uint32_t v, unsigned s) noexcept {
#if defined(_MSC_VER) && !defined(__cplusplus)
  return _rotl(v, s);
#else
  return s &= 31, (v << s) | (v >> (32 - s));
#endif
}

static cxx14_constexpr uint64_t ror64(uint64_t v, unsigned s) noexcept {
#if defined(_MSC_VER) && !defined(__cplusplus)
  return _rotr64(v, s);
#else
  return s &= 63, (v >> s) | (v << (64 - s));
#endif
}

static cxx14_constexpr uint64_t rol64(uint64_t v, unsigned s) noexcept {
#if defined(_MSC_VER) && !defined(__cplusplus)
  return _rotl64(v, s);
#else
  return s &= 63, (v << s) | (v >> (64 - s));
#endif
}

#ifdef ERTHINK_NATIVE_U128_TYPE

static cxx14_constexpr ERTHINK_NATIVE_U128_TYPE
ror128(ERTHINK_NATIVE_U128_TYPE v, unsigned s) noexcept {
  return (s &= 127) ? v >> s | v << (128 - s) : v;
}

static cxx14_constexpr ERTHINK_NATIVE_U128_TYPE
rol128(ERTHINK_NATIVE_U128_TYPE v, unsigned s) noexcept {
  return (s &= 127) ? v << s | v >> (128 - s) : v;
}

#endif /* ERTHINK_NATIVE_U128_TYPE */

//------------------------------------------------------------------------------

#ifdef __cplusplus

template <typename T> cxx14_constexpr T ror(T v, unsigned s) noexcept;
template <typename T> cxx14_constexpr T rol(T v, unsigned s) noexcept;

template <>
cxx14_constexpr uint32_t ror<uint32_t>(uint32_t v, unsigned s) noexcept {
  return ror32(v, s);
}

template <>
cxx14_constexpr uint32_t rol<uint32_t>(uint32_t v, unsigned s) noexcept {
  return rol32(v, s);
}

template <>
cxx14_constexpr uint64_t ror<uint64_t>(uint64_t v, unsigned s) noexcept {
  return ror64(v, s);
}

template <>
cxx14_constexpr uint64_t rol<uint64_t>(uint64_t v, unsigned s) noexcept {
  return rol64(v, s);
}

#ifdef ERTHINK_NATIVE_U128_TYPE

template <>
cxx14_constexpr ERTHINK_NATIVE_U128_TYPE
ror<ERTHINK_NATIVE_U128_TYPE>(ERTHINK_NATIVE_U128_TYPE v, unsigned s) noexcept {
  return ror128(v, s);
}

template <>
cxx14_constexpr ERTHINK_NATIVE_U128_TYPE
rol<ERTHINK_NATIVE_U128_TYPE>(ERTHINK_NATIVE_U128_TYPE v, unsigned s) noexcept {
  return rol128(v, s);
}

#endif /* ERTHINK_NATIVE_U128_TYPE */

} // namespace erthink
#endif /* __cplusplus */
