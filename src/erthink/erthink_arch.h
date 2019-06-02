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
#include "erthink_defs.h"

#ifdef _MSC_VER
#pragma warning(push, 1)
#endif
#if defined(__KERNEL__) || !defined(__cplusplus) || __cplusplus < 201103L
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#else
#include <climits>
#include <cstddef>
#include <cstdint>
#endif
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#if (UINTPTR_MAX > 0xffffFFFFul || ULONG_MAX > 0xffffFFFFul)
#define ERTHINK_ARCH64
#else
#define ERTHINK_ARCH32
#endif /* FPT_ARCH64/32 */

#if defined(i386) || defined(__386) || defined(__i386) || defined(__i386__) || \
    defined(i486) || defined(__i486) || defined(__i486__) ||                   \
    defined(i586) | defined(__i586) || defined(__i586__) || defined(i686) ||   \
    defined(__i686) || defined(__i686__) || defined(_M_IX86) ||                \
    defined(_X86_) || defined(__THW_INTEL__) || defined(__I86__) ||            \
    defined(__INTEL__) || defined(__x86_64) || defined(__x86_64__) ||          \
    defined(__amd64__) || defined(__amd64) || defined(_M_X64) ||               \
    defined(_M_AMD64) || defined(__IA32__) || defined(__INTEL__)
#ifndef __ia32__
/* LY: define neutral __ia32__ for x86 and x86-64 archs */
#define __ia32__ 1
#endif /* __ia32__ */

/* LY: define neutral __amd64__ for x86-64 */
#if !defined(__amd64__) && (defined(__x86_64) || defined(__x86_64__) ||        \
                            defined(__amd64) || defined(_M_X64))
/* LY: define trusty __amd64__ for all AMD64/x86-64 arch */
#define __amd64__ 1
#endif /* __amd64__ */
#endif /* all x86 */
