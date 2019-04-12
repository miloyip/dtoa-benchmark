/*
 *  Copyright (c) 1994-2019 Leonid Yuriev <leo@yuriev.ru>.
 *  https://github.com/leo-yuriev/erthink
 *  ZLib License
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty. In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgement in the product documentation would be
 *     appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
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
#include <stddef.h>
#else
#include <cstddef>
#endif
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

#if __has_feature(thread_sanitizer)
#define __SANITIZE_THREAD__ 1
#endif

#if __has_feature(address_sanitizer)
#define __SANITIZE_ADDRESS__ 1
#endif

//----------------------------------------------------------------------------

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901
#if (defined(__GNUC__) && __GNUC__ >= 2) || defined(__clang__) ||              \
    defined(_MSC_VER)
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

#if !defined(nullptr) && !defined(__cplusplus) ||                              \
    (__cplusplus < 201103L && !defined(_MSC_VER))
#define nullptr NULL
#endif

#if !defined(constexpr) && !defined(__cplusplus) ||                            \
    (__cplusplus < 201103L && !defined(_MSC_VER))
#define constexpr
#endif

#if !defined(cxx14_constexpr)
#if defined(__cplusplus) && __cplusplus >= 201402L &&                          \
    (!defined(_MSC_VER) || _MSC_VER >= 1910) &&                                \
    (!defined(__GNUC__) || __GNUC__ >= 6)
#define cxx14_constexpr constexpr
#else
#define cxx14_constexpr
#endif
#endif /* cxx14_constexpr */

#if !defined(cxx17_constexpr)
#if defined(__cplusplus) && __cplusplus >= 201703L &&                          \
    (!defined(_MSC_VER) || _MSC_VER >= 1915) &&                                \
    (!defined(__GNUC__) || __GNUC__ >= 7)
#define cxx17_constexpr constexpr
#else
#define cxx17_constexpr
#endif
#endif /* cxx17_constexpr */

#if __cplusplus >= 201402L
#define constexpr_assert(foo) assert(foo)
#else
#define constexpr_assert(foo) __noop(foo)
#endif /* constexpr_assert for C++14 */

//----------------------------------------------------------------------------

#if defined(__GNUC__) || __has_attribute(format)
#define __printf_args(format_index, first_arg)                                 \
  __attribute__((format(printf, format_index, first_arg)))
#else
#define __printf_args(format_index, first_arg)
#endif

#if !defined(__thread) && (defined(_MSC_VER) || defined(__DMC__))
#define __thread __declspec(thread)
#endif /* __thread */

#ifndef __always_inline
#if defined(__GNUC__) || __has_attribute(always_inline)
#define __always_inline __inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#define __always_inline __forceinline
#else
#define __always_inline
#endif
#endif /* __always_inline */

#ifndef __must_check_result
#if defined(__GNUC__) || __has_attribute(warn_unused_result)
#define __must_check_result __attribute__((warn_unused_result))
#else
#define __must_check_result
#endif
#endif /* __must_check_result */

#ifndef __deprecated
#if defined(__GNUC__) || __has_attribute(deprecated)
#define __deprecated __attribute__((deprecated))
#elif defined(_MSC_VER)
#define __deprecated __declspec(deprecated)
#else
#define __deprecated
#endif
#endif /* __deprecated */

#ifndef __noreturn
#if defined(__GNUC__) || __has_attribute(noreturn)
#define __noreturn __attribute__((noreturn))
#elif defined(_MSC_VER)
#define __noreturn __declspec(noreturn)
#else
#define __noreturn
#endif
#endif /* __noreturn */

#ifndef __nothrow
#if defined(__GNUC__) || __has_attribute(nothrow)
#define __nothrow __attribute__((nothrow))
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
#if defined(__GNUC__) || __has_attribute(pure)
#define __pure_function __attribute__((pure))
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
#if defined(__GNUC__) || __has_attribute(const)
#define __const_function __attribute__((const))
#else
#define __const_function
#endif
#endif /* __const_function */

#ifndef __optimize
#if defined(__OPTIMIZE__)
#if defined(__clang__) && !__has_attribute(optimize)
#define __optimize(ops)
#elif defined(__GNUC__) || __has_attribute(optimize)
#define __optimize(ops) __attribute__((optimize(ops)))
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
#define __hot __attribute__((hot)) __optimize(3)
#elif defined(__clang__) && !__has_attribute(hot)
/* just put frequently used functions in separate section */
#define __hot __attribute__((section("text.hot"))) __optimize("O3")
#elif defined(__GNUC__) || __has_attribute(hot)
#define __hot __attribute__((hot)) __optimize("O3")
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
#define __cold __optimize(1) __attribute__((cold))
#elif defined(__clang__) && !__has_attribute(cold)
/* just put infrequently used functions in separate section */
#define __cold __attribute__((section("text.unlikely"))) __optimize("Os")
#elif defined(__GNUC__) || __has_attribute(cold)
#define __cold __attribute__((cold)) __optimize("Os")
#else
#define __cold __optimize("Os")
#endif
#else
#define __cold
#endif
#endif /* __cold */

#ifndef __flatten
#if defined(__OPTIMIZE__) && (defined(__GNUC__) || __has_attribute(flatten))
#define __flatten __attribute__((flatten))
#else
#define __flatten
#endif
#endif /* __flatten */

#ifndef __noinline
#if defined(__GNUC__) || __has_attribute(noinline)
#define __noinline __attribute__((noinline))
#elif defined(_MSC_VER)
#define __noinline __declspec(noinline)
#elif defined(__SUNPRO_C) || defined(__sun) || defined(sun)
#define __noinline inline
#elif !defined(__INTEL_COMPILER)
#define __noinline /* FIXME ? */
#endif
#endif /* __noinline */

#ifndef __maybe_unused
#if defined(__GNUC__) || __has_attribute(unused)
#define __maybe_unused __attribute__((unused))
#else
#define __maybe_unused
#endif
#endif /* __maybe_unused */

//----------------------------------------------------------------------------

#ifndef __dll_hidden
#if defined(__GNUC__) || __has_attribute(visibility)
#define __hidden __attribute__((visibility("hidden")))
#else
#define __hidden
#endif
#endif /* __dll_hidden */

#ifndef __dll_export
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#if defined(__GNUC__) || __has_attribute(dllexport)
#define __dll_export __attribute__((dllexport))
#else
#define __dll_export __declspec(dllexport)
#endif
#elif defined(__GNUC__) || __has_attribute(visibility)
#define __dll_export __attribute__((visibility("default")))
#else
#define __dll_export
#endif
#endif /* __dll_export */

#ifndef __dll_import
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#if defined(__GNUC__) || __has_attribute(dllimport)
#define __dll_import __attribute__((dllimport))
#else
#define __dll_import __declspec(dllimport)
#endif
#elif defined(__GNUC__) || __has_attribute(visibility)
#define __dll_import __attribute__((visibility("default")))
#else
#define __dll_import
#endif
#endif /* __dll_import */

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
#elif defined(__GNUC__) || __has_attribute(packed)
#define ERTHINK_PACKED_STRUCT(name) struct __attribute__((packed)) name
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
#if defined(__GNUC__) || defined(__clang__)
#define likely(cond) __builtin_expect(!!(cond), 1)
#else
#define likely(x) (x)
#endif
#endif /* likely */

#ifndef unlikely
#if defined(__GNUC__) || defined(__clang__)
#define unlikely(cond) __builtin_expect(!!(cond), 0)
#else
#define unlikely(x) (x)
#endif
#endif /* unlikely */

#ifndef __aligned
#if defined(__GNUC__) || defined(__clang__)
#define __aligned(N) __attribute__((aligned(N)))
#elif defined(_MSC_VER)
#define __aligned(N) __declspec(align(N))
#else
#define __aligned(N)
#endif
#endif /* __align */

//----------------------------------------------------------------------------

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
