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
#include "erthink_defs.h"

//------------------------------------------------------------------------------

/* GNU ELF indirect functions usage control. For more info please see
 * https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
 * and https://sourceware.org/glibc/wiki/GNU_IFUNC */

#ifndef ERTHINK_USE_ELF_IFUNC
#if defined(__clang__) && __clang_major__ < 4
/* Clang 3.9 have a trouble with ifunc, especially when LTO is enabled. */
#define ERTHINK_USE_ELF_IFUNC 0
#elif __has_attribute(__ifunc__) &&                                            \
    defined(__ELF__) /* ifunc is broken on Darwin/OSX */
/* Use ifunc/gnu_indirect_function if corresponding attribute is available,
 * Assuming compiler will generate properly code even when
 * the -fstack-protector-all and/or the -fsanitize=address are enabled. */
#define ERTHINK_USE_ELF_IFUNC 1
#elif defined(__ELF__) && !defined(__SANITIZE_ADDRESS__) &&                    \
    !defined(__SSP_ALL__)
/* ifunc/gnu_indirect_function will be used on ELF, but only if both
 * -fstack-protector-all and -fsanitize=address are NOT enabled. */
#define ERTHINK_USE_ELF_IFUNC 1
#else
#define ERTHINK_USE_ELF_IFUNC 0
#endif
#endif /* ERTHINK_USE_ELF_IFUNC */

//------------------------------------------------------------------------------

#if ERTHINK_USE_ELF_IFUNC
#define ERTHINK_IFUNC_RESOLVER_API(API_VISIBILITY) __extern_C

#define ERTHINK_DECLARE_IFUNC(API_VISIBILITY, RESULT_TYPE, NAME,               \
                              DECLARGS_PARENTHESIZED, CALLARGS_PARENTHESIZED,  \
                              RESOLVER)                                        \
  __extern_C API_VISIBILITY RESULT_TYPE NAME DECLARGS_PARENTHESIZED;

#if __has_attribute(__ifunc__) ||                                              \
    (defined(__ELF__) && __GLIBC_PREREQ(2, 11) && __GNUC_PREREQ(4, 6))

#define ERTHINK_DEFINE_IFUNC(API_VISIBILITY, RESULT_TYPE, NAME,                \
                             DECLARGS_PARENTHESIZED, CALLARGS_PARENTHESIZED,   \
                             RESOLVER)                                         \
                                                                               \
  ERTHINK_IFUNC_RESOLVER_API(API_VISIBILITY)                                   \
  __attribute__((__used__)) RESULT_TYPE(*RESOLVER(void))                       \
      DECLARGS_PARENTHESIZED;                                                  \
                                                                               \
  __extern_C API_VISIBILITY RESULT_TYPE NAME DECLARGS_PARENTHESIZED            \
      __attribute__((__ifunc__(STRINGIFY(RESOLVER))));

#else

/* *INDENT-OFF* */
/* clang-format off */
#define ERTHINK_DEFINE_IFUNC(API_VISIBILITY, RESULT_TYPE, NAME,                \
                          DECLARGS_PARENTHESIZED,                              \
                          CALLARGS_PARENTHESIZED, RESOLVER)                    \
                                                                               \
    __extern_C RESULT_TYPE (*RESOLVER(void)) DECLARGS_PARENTHESIZED;           \
                                                                               \
    __asm__("\t.globl\t" STRINGIFY(NAME) "\n"                                  \
            "\t.type\t" STRINGIFY(NAME) ", %gnu_indirect_function\n"           \
            "\t.set\t" STRINGIFY(NAME) "," STRINGIFY(RESOLVER)                 \
      )
/* *INDENT-ON* */
/* clang-format on */

#endif /* __has_attribute(__ifunc__) */

#else /* ERTHINK_USE_ELF_IFUNC */
#define ERTHINK_IFUNC_RESOLVER_API(API) static

#ifdef __cplusplus

#define ERTHINK_DECLARE_IFUNC(API_VISIBILITY, RESULT_TYPE, NAME,               \
                              DECLARGS_PARENTHESIZED, CALLARGS_PARENTHESIZED,  \
                              RESOLVER)                                        \
                                                                               \
  __extern_C API_VISIBILITY RESULT_TYPE(*const NAME##_iFuncPtr)                \
      DECLARGS_PARENTHESIZED;                                                  \
                                                                               \
  static inline RESULT_TYPE NAME DECLARGS_PARENTHESIZED {                      \
    return NAME##_iFuncPtr CALLARGS_PARENTHESIZED;                             \
  }

#define ERTHINK_DEFINE_IFUNC(API_VISIBILITY, RESULT_TYPE, NAME,                \
                             DECLARGS_PARENTHESIZED, CALLARGS_PARENTHESIZED,   \
                             RESOLVER)                                         \
                                                                               \
  ERTHINK_IFUNC_RESOLVER_API(API_VISIBILITY)                                   \
  RESULT_TYPE(*RESOLVER(void)) DECLARGS_PARENTHESIZED;                         \
                                                                               \
  RESULT_TYPE(*const NAME##_iFuncPtr) DECLARGS_PARENTHESIZED = RESOLVER();

#else /* __cplusplus */

#define ERTHINK_DECLARE_IFUNC(API_VISIBILITY, RESULT_TYPE, NAME,               \
                              DECLARGS_PARENTHESIZED, CALLARGS_PARENTHESIZED,  \
                              RESOLVER)                                        \
                                                                               \
  __extern_C API_VISIBILITY RESULT_TYPE(*NAME##_iFuncPtr)                      \
      DECLARGS_PARENTHESIZED;                                                  \
                                                                               \
  static __inline RESULT_TYPE NAME DECLARGS_PARENTHESIZED {                    \
    return NAME##_iFuncPtr CALLARGS_PARENTHESIZED;                             \
  }

#if __GNUC_PREREQ(4, 0) || __has_attribute(__constructor__)

#define ERTHINK_DEFINE_IFUNC(API_VISIBILITY, RESULT_TYPE, NAME,                \
                             DECLARGS_PARENTHESIZED, CALLARGS_PARENTHESIZED,   \
                             RESOLVER)                                         \
                                                                               \
  RESULT_TYPE(*NAME##_iFuncPtr) DECLARGS_PARENTHESIZED;                        \
                                                                               \
  ERTHINK_IFUNC_RESOLVER_API(API_VISIBILITY)                                   \
  RESULT_TYPE(*RESOLVER(void)) DECLARGS_PARENTHESIZED;                         \
                                                                               \
  static __cold void __attribute__((__constructor__))                          \
  NAME##_iFunc_init(void) {                                                    \
    NAME##_iFuncPtr = RESOLVER();                                              \
  }

#else /* __has_attribute(__constructor__) */

#define ERTHINK_DEFINE_IFUNC(API_VISIBILITY, RESULT_TYPE, NAME,                \
                             DECLARGS_PARENTHESIZED, CALLARGS_PARENTHESIZED,   \
                             RESOLVER)                                         \
                                                                               \
  ERTHINK_IFUNC_RESOLVER_API(API_VISIBILITY)                                   \
  RESULT_TYPE(*RESOLVER(void)) DECLARGS_PARENTHESIZED;                         \
                                                                               \
  static __cold RESULT_TYPE NAME##_proxy DECLARGS_PARENTHESIZED {              \
    NAME##_iFuncPtr = RESOLVER();                                              \
    return NAME##_iFuncPtr CALLARGS_PARENTHESIZED;                             \
  }                                                                            \
                                                                               \
  RESULT_TYPE(*NAME##_iFuncPtr) DECLARGS_PARENTHESIZED = NAME##_proxy;

#endif /* __has_attribute(__constructor__) */

#endif /* __cplusplus */

#endif /* ERTHINK_USE_ELF_IFUNC */
