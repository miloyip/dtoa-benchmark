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

#ifdef _MSC_VER
#if defined(_MSC_VER)
#define _STL_WARNING_LEVEL 3
#endif
#pragma warning(push, 1)
#pragma warning(disable : 4548) /* expression before comma has no effect;      \
                                   expected expression with side - effect */
#pragma warning(disable : 4530) /* C++ exception handler used, but unwind      \
                                   semantics are not enabled. Specify /EHsc */
#pragma warning(disable : 4577) /* 'noexcept' used with no exception handling  \
                                   mode specified; termination on exception    \
                                   is not guaranteed. Specify /EHsc */
#endif

#if defined(__KERNEL__) || !defined(__cplusplus) || __cplusplus < 201103L
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#else
#include <cassert>
#include <cstddef>
#include <cstdint>
#endif

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) ||     \
    defined(__BSD__) || defined(__NETBSD__) || defined(__bsdi__) ||            \
    defined(__DragonFly__)
#include </usr/include/sys/cdefs.h>
#endif /* BSD */

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifndef __GNUC_PREREQ
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#define __GNUC_PREREQ(maj, min)                                                \
  ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
#define __GNUC_PREREQ(maj, min) (0)
#endif
#endif /* __GNUC_PREREQ */

#ifndef __CLANG_PREREQ
#ifdef __clang__
#define __CLANG_PREREQ(maj, min)                                               \
  ((__clang_major__ << 16) + __clang_minor__ >= ((maj) << 16) + (min))
#else
#define __CLANG_PREREQ(maj, min) (0)
#endif
#endif /* __CLANG_PREREQ */

#ifndef __GLIBC_PREREQ
#if defined(__GLIBC__) && defined(__GLIBC_MINOR__)
#define __GLIBC_PREREQ(maj, min)                                               \
  ((__GLIBC__ << 16) + __GLIBC_MINOR__ >= ((maj) << 16) + (min))
#else
#define __GLIBC_PREREQ(maj, min) (0)
#endif
#endif /* __GLIBC_PREREQ */

#ifndef __has_attribute
#define __has_attribute(x) (0)
#endif

#ifndef __has_feature
#define __has_feature(x) (0)
#endif

#ifndef __has_extension
#define __has_extension(x) (0)
#endif

#ifndef __has_builtin
#define __has_builtin(x) (0)
#endif

#ifndef __has_warning
#define __has_warning(x) (0)
#endif

#ifndef __has_include
#define __has_include(x) (0)
#endif

#ifndef __has_cpp_attribute
#define __has_cpp_attribute(x) (0)
#endif

#if __has_feature(thread_sanitizer)
#define __SANITIZE_THREAD__ 1
#endif

#if __has_feature(address_sanitizer)
#define __SANITIZE_ADDRESS__ 1
#endif

#if !defined(__cplusplus) && (HAVE_STDALIGN_H || __has_include(<stdalign.h>))
#include <stdalign.h>
#endif

//------------------------------------------------------------------------------

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901
#if __GNUC_PREREQ(2, 0) || defined(__clang__) || defined(_MSC_VER)
#define __func__ __FUNCTION__
#else
#define __func__ "__func__"
#endif
#endif /* __func__ */

#ifndef __extern_C
#ifdef __cplusplus
#define __extern_C extern "C"
#else
#define __extern_C
#endif
#endif /* __extern_C */

#ifndef __cplusplus
#ifndef bool
#define bool _Bool
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#endif

#ifndef __fallthrough
#if __has_cpp_attribute(fallthrough)
#define __fallthrough [[fallthrough]]
#elif __GNUC_PREREQ(8, 0) && defined(__cplusplus) && __cplusplus >= 201103L
#define __fallthrough [[fallthrough]]
#elif __GNUC_PREREQ(7, 0)
#define __fallthrough __attribute__((__fallthrough__))
#elif defined(__clang__) && defined(__cplusplus) && __cplusplus >= 201103L &&  \
    __has_feature(cxx_attributes) && __has_warning("-Wimplicit-fallthrough")
#define __fallthrough [[clang::fallthrough]]
#else
#define __fallthrough
#endif
#endif /* __fallthrough */

#if !defined(nullptr) && (!defined(__cplusplus) || __cplusplus < 201103L)
#define nullptr NULL
#endif /* nullptr */

#if !defined(noexcept) && (!defined(__cplusplus) || __cplusplus < 201103L)
#define noexcept
#endif /* noexcept */

#if !defined(constexpr) && (!defined(__cplusplus) || __cplusplus < 201103L)
#define constexpr
#endif /* constexpr */

#if !defined(cxx14_constexpr)
#if defined(__cplusplus) && __cplusplus >= 201402L &&                          \
    (!defined(_MSC_VER) || _MSC_VER >= 1910) &&                                \
    (!defined(__GNUC__) || defined(__clang__) || __GNUC__ >= 6)
#define cxx14_constexpr constexpr
#else
#define cxx14_constexpr
#endif
#endif /* cxx14_constexpr */

#if !defined(cxx17_constexpr)
#if defined(__cplusplus) && __cplusplus >= 201703L &&                          \
    (!defined(_MSC_VER) || _MSC_VER >= 1915) &&                                \
    (!defined(__GNUC__) || defined(__clang__) || __GNUC__ >= 7)
#define cxx17_constexpr constexpr
#define cxx17_noexcept noexcept
#define if_constexpr if constexpr
#else
#define cxx17_constexpr
#define cxx17_noexcept
#define if_constexpr if
#endif
#endif /* cxx17_constexpr */

#if !defined(constexpr_assert) && defined(__cplusplus)
#if defined(HAS_RELAXED_CONSTEXPR) ||                                          \
    (__cplusplus >= 201408L && (!defined(_MSC_VER) || _MSC_VER >= 1915) &&     \
     (!defined(__GNUC__) || defined(__clang__) || __GNUC__ >= 6))
#define constexpr_assert(cond) assert(cond)
#else
#define constexpr_assert(foo)
#endif
#endif /* constexpr_assert */

#ifndef NDEBUG_CONSTEXPR
#ifdef NDEBUG
#define NDEBUG_CONSTEXPR constexpr
#else
#define NDEBUG_CONSTEXPR
#endif
#endif /* NDEBUG_CONSTEXPR */

/* Crutch for case when OLD GLIBC++ (without std::max_align_t)
 * is coupled with MODERN C++ COMPILER (with __cpp_aligned_new) */
#ifndef ERTHINK_PROVIDE_ALIGNED_NEW
#if defined(__cpp_aligned_new) &&                                              \
    (!defined(__GLIBCXX__) || defined(_GLIBCXX_HAVE_ALIGNED_ALLOC))
#define ERTHINK_PROVIDE_ALIGNED_NEW 1
#else
#define ERTHINK_PROVIDE_ALIGNED_NEW 0
#endif
#endif /* ERTHINK_PROVIDE_ALIGNED_NEW */

#ifndef ERTHINK_NAME_PREFIX
#ifdef __cplusplus
#define ERTHINK_NAME_PREFIX(NAME) NAME
#else
#define ERTHINK_NAME_PREFIX(NAME) erthink_##NAME
#endif
#endif /* ERTHINK_NAME_PREFIX */

#ifndef constexpr_intrin
#ifdef __GNUC__
#define constexpr_intrin constexpr
#else
#define constexpr_intrin
#endif
#endif /* constexpr_intrin */

//------------------------------------------------------------------------------

#if defined(__GNUC__) || __has_attribute(__format__)
#define __printf_args(format_index, first_arg)                                 \
  __attribute__((__format__(__printf__, format_index, first_arg)))
#else
#define __printf_args(format_index, first_arg)
#endif

#if !defined(__thread) && (defined(_MSC_VER) || defined(__DMC__))
#define __thread __declspec(thread)
#endif /* __thread */

#ifndef __always_inline
#if defined(__GNUC__) || __has_attribute(__always_inline__)
#define __always_inline __inline __attribute__((__always_inline__))
#elif defined(_MSC_VER)
#define __always_inline __forceinline
#else
#define __always_inline
#endif
#endif /* __always_inline */

#ifndef __must_check_result
#if defined(__GNUC__) || __has_attribute(__warn_unused_result__)
#define __must_check_result __attribute__((__warn_unused_result__))
#else
#define __must_check_result
#endif
#endif /* __must_check_result */

#ifndef __deprecated
#if defined(__GNUC__) || __has_attribute(__deprecated__)
#define __deprecated __attribute__((__deprecated__))
#elif defined(_MSC_VER)
#define __deprecated __declspec(deprecated)
#else
#define __deprecated
#endif
#endif /* __deprecated */

#ifndef __noreturn
#if defined(__GNUC__) || __has_attribute(__noreturn__)
#define __noreturn __attribute__((__noreturn__))
#elif defined(_MSC_VER)
#define __noreturn __declspec(noreturn)
#else
#define __noreturn
#endif
#endif /* __noreturn */

#ifndef __nothrow
#if defined(__cplusplus)
#if __cplusplus < 201703L
#define __nothrow throw()
#else
#define __nothrow noexcept(true)
#endif /* __cplusplus */
#elif defined(__GNUC__) || __has_attribute(__nothrow__)
#define __nothrow __attribute__((__nothrow__))
#elif defined(_MSC_VER) && defined(__cplusplus)
#define __nothrow __declspec(nothrow)
#else
#define __nothrow
#endif
#endif /* __nothrow */

#ifndef __pure_function
/* Many functions have no effects except the return value and their
 * return value depends only on the parameters and/or global variables.
 * Such a function can be subject to common subexpression elimination
 * and loop optimization just as an arithmetic operator would be.
 * These functions should be declared with the attribute pure. */
#if defined(__GNUC__) || __has_attribute(__pure__)
#define __pure_function __attribute__((__pure__))
#else
#define __pure_function
#endif
#endif /* __pure_function */

#ifndef __const_function
/* Many functions do not examine any values except their arguments,
 * and have no effects except the return value. Basically this is just
 * slightly more strict class than the PURE attribute, since function
 * is not allowed to read global memory.
 *
 * Note that a function that has pointer arguments and examines the
 * data pointed to must not be declared const. Likewise, a function
 * that calls a non-const function usually must not be const.
 * It does not make sense for a const function to return void. */
#if defined(__GNUC__) || __has_attribute(__const__)
#define __const_function __attribute__((__const__))
#else
#define __const_function
#endif
#endif /* __const_function */

#ifndef __optimize
#if defined(__OPTIMIZE__)
#if (defined(__GNUC__) && !defined(__clang__)) || __has_attribute(__optimize__)
#define __optimize(ops) __attribute__((__optimize__(ops)))
#else
#define __optimize(ops)
#endif
#else
#define __optimize(ops)
#endif
#endif /* __optimize */

#ifndef __hot
#if defined(__OPTIMIZE__)
#if defined(__e2k__)
#define __hot __attribute__((__hot__)) __optimize(3)
#elif defined(__clang__) && !__has_attribute(__hot__) &&                       \
    __has_attribute(__section__) &&                                            \
    (defined(__linux__) || defined(__gnu_linux__))
/* just put frequently used functions in separate section */
#define __hot __attribute__((__section__("text.hot"))) __optimize("O3")
#elif defined(__GNUC__) || __has_attribute(__hot__)
#define __hot __attribute__((__hot__)) __optimize("O3")
#else
#define __hot __optimize("O3")
#endif
#else
#define __hot
#endif
#endif /* __hot */

#ifndef __cold
#if defined(__OPTIMIZE__)
#if defined(__e2k__)
#define __cold __optimize(1) __attribute__((__cold__))
#elif defined(__clang__) && !__has_attribute(__cold__) &&                      \
    __has_attribute(__section__) &&                                            \
    (defined(__linux__) || defined(__gnu_linux__))
/* just put infrequently used functions in separate section */
#define __cold __attribute__((__section__("text.unlikely"))) __optimize("Os")
#elif defined(__GNUC__) || __has_attribute(__cold__)
#define __cold __attribute__((__cold__)) __optimize("Os")
#else
#define __cold __optimize("Os")
#endif
#else
#define __cold
#endif
#endif /* __cold */

#ifndef __flatten
#if defined(__OPTIMIZE__) && (defined(__GNUC__) || __has_attribute(__flatten__))
#define __flatten __attribute__((__flatten__))
#else
#define __flatten
#endif
#endif /* __flatten */

#ifndef __noinline
#if defined(__GNUC__) || __has_attribute(__noinline__)
#define __noinline __attribute__((__noinline__))
#elif defined(_MSC_VER)
#define __noinline __declspec(noinline)
#else
#define __noinline
#endif
#endif /* __noinline */

#ifndef __maybe_unused
#if defined(__GNUC__) || __has_attribute(__unused__)
#define __maybe_unused __attribute__((__unused__))
#else
#define __maybe_unused
#endif
#endif /* __maybe_unused */

//------------------------------------------------------------------------------

#ifndef __hidden
#if defined(__GNUC__) || __has_attribute(__visibility__)
#define __hidden __attribute__((__visibility__("hidden")))
#else
#define __hidden
#endif
#endif /* __hidden */

#ifndef __dll_export
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#if defined(__GNUC__) || __has_attribute(__dllexport__)
#define __dll_export __attribute__((__dllexport__))
#else
#define __dll_export __declspec(dllexport)
#endif
#elif defined(__GNUC__) || __has_attribute(__visibility__)
#define __dll_export __attribute__((__visibility__("default")))
#else
#define __dll_export
#endif
#endif /* __dll_export */

#ifndef __dll_import
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#if defined(__GNUC__) || __has_attribute(__dllimport__)
#define __dll_import __attribute__((__dllimport__))
#else
#define __dll_import __declspec(dllimport)
#endif
#elif defined(__GNUC__) || __has_attribute(__visibility__)
#define __dll_import __attribute__((__visibility__("default")))
#else
#define __dll_import
#endif
#endif /* __dll_import */

#ifndef __dll_visibility_default
#if defined(__GNUC__) || __has_attribute(__visibility__)
#define __dll_visibility_default __attribute__((__visibility__("default")))
#else
#define __dll_visibility_default
#endif
#endif /* __dll_visibility_default */

//----------------------------------------------------------------------------

#if !defined(__noop) && !defined(_MSC_VER)
#ifdef __cplusplus
static inline void __noop_consume_args() {}
template <typename First, typename... Rest>
static inline void __noop_consume_args(const First &first,
                                       const Rest &... rest) {
  (void)first;
  __noop_consume_args(rest...);
}
#define __noop(...) __noop_consume_args(__VA_ARGS__)
#elif defined(__GNUC__) && (!defined(__STRICT_ANSI__) || !__STRICT_ANSI__)
static __inline void __noop_consume_args(void *anchor, ...) { (void)anchor; }
#define __noop(...) __noop_consume_args(0, ##__VA_ARGS__)
#else
#define __noop(...)                                                            \
  do {                                                                         \
  } while (0)
#endif
#endif /* __noop */

#ifdef _MSC_VER
#define ERTHINK_PACKED_STRUCT(name)                                            \
  __pragma(pack(push, 1)) struct name __pragma(pack(pop))
#elif defined(__GNUC__) || __has_attribute(__packed__)
#define ERTHINK_PACKED_STRUCT(name) struct __attribute__((__packed__)) name
#else
#error Unsupported C/C++ compiler
#endif /* FPT_PACKED_STRUCT */

#ifndef __unreachable
#if __GNUC_PREREQ(4, 5)
#define __unreachable() __builtin_unreachable()
#elif defined(_MSC_VER)
#define __unreachable() __assume(0)
#else
#define __unreachable() __noop()
#endif
#endif /* __unreachable */

#ifndef likely
#if (defined(__GNUC__) || defined(__clang__)) && !defined(__COVERITY__)
#define likely(cond) __builtin_expect(!!(cond), 1)
#else
#define likely(x) (x)
#endif
#endif /* likely */

#ifndef unlikely
#if (defined(__GNUC__) || defined(__clang__)) && !defined(__COVERITY__)
#define unlikely(cond) __builtin_expect(!!(cond), 0)
#else
#define unlikely(x) (x)
#endif
#endif /* unlikely */

#if !defined(alignas) && (!defined(__cplusplus) || __cplusplus < 201103L)
#if defined(__GNUC__) || defined(__clang__) || __has_attribute(__aligned__)
#define alignas(N) __attribute__((__aligned__(N)))
#elif defined(_MSC_VER)
#define alignas(N) __declspec(align(N))
#else
#error "C++11 or C11 compiler is required"
#endif
#endif /* alignas */

//------------------------------------------------------------------------------

#if !defined(__typeof)
#ifdef _MSC_VER
#define __typeof(exp) decltype(exp)
#else
#define __typeof(exp) __typeof__(exp)
#endif
#endif /* __typeof */

#ifndef offsetof
#define offsetof(type, member) __builtin_offsetof(type, member)
#endif /* offsetof */

#ifndef container_of
#define container_of(ptr, type, member)                                        \
  ({                                                                           \
    const __typeof(((type *)nullptr)->member) *__ptr = (ptr);                  \
    (type *)((char *)__ptr - offsetof(type, member));                          \
  })
#endif /* container_of */

#ifndef STRINGIFY
#define __MAKE_STR(x) #x
#define STRINGIFY(x) __MAKE_STR(x)
#endif /* STRINGIFY */

//------------------------------------------------------------------------------

#ifndef DEFINE_ENUM_FLAG_OPERATORS
#if defined(__cplusplus)
// Define operator overloads to enable bit operations on enum values that are
// used to define flags (based on Microsoft's DEFINE_ENUM_FLAG_OPERATORS).
#define DEFINE_ENUM_FLAG_OPERATORS(ENUM)                                       \
  extern "C++" {                                                               \
  constexpr inline ENUM operator|(ENUM a, ENUM b) {                            \
    return ENUM(std::size_t(a) | std::size_t(b));                              \
  }                                                                            \
  cxx14_constexpr inline ENUM &operator|=(ENUM &a, ENUM b) {                   \
    return a = a | b;                                                          \
  }                                                                            \
  constexpr inline ENUM operator&(ENUM a, ENUM b) {                            \
    return ENUM(std::size_t(a) & std::size_t(b));                              \
  }                                                                            \
  cxx14_constexpr inline ENUM &operator&=(ENUM &a, ENUM b) {                   \
    return a = a & b;                                                          \
  }                                                                            \
  constexpr inline ENUM operator~(ENUM a) { return ENUM(~std::size_t(a)); }    \
  constexpr inline ENUM operator^(ENUM a, ENUM b) {                            \
    return ENUM(std::size_t(a) ^ std::size_t(b));                              \
  }                                                                            \
  cxx14_constexpr inline ENUM &operator^=(ENUM &a, ENUM b) {                   \
    return a = a ^ b;                                                          \
  }                                                                            \
  }
#else                                    /* __cplusplus */
#define DEFINE_ENUM_FLAG_OPERATORS(ENUM) /* nope, C allows these operators */
#endif                                   /* !__cplusplus */
#endif                                   /* DEFINE_ENUM_FLAG_OPERATORS */
