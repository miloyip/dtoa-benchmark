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
#include "erthink_defs.h"

#if __GNUC_PREREQ(4, 4) || defined(__clang__)

#if defined(__ia32__) || defined(__e2k__)
#include <x86intrin.h>
#endif

#if defined(__ia32__) && !defined(__cpuid_count)
#include <cpuid.h>
#endif

#if defined(__e2k__)
#include <e2kbuiltin.h>
#endif

#elif defined(_MSC_VER) && !defined(__clang__)

#if _MSC_FULL_VER < 191526730
#pragma message(                                                               \
    "It is recommended to use \"Microsoft C/C++ Compiler\" version 19.15.26730 (Visual Studio 2017 15.8) or newer.")
#endif
#if _MSC_FULL_VER < 190024234
#pragma message(                                                               \
    "At least \"Microsoft C/C++ Compiler\" version 19.00.24234 (Visual Studio 2015 Update 3) is required.")
#endif

#pragma warning(push, 1)
#include <intrin.h>
#pragma warning(pop)
#pragma warning(disable : 4514) /* 'xyz': unreferenced inline function         \
                                   has been removed */
#pragma warning(disable : 4710) /* 'xyz': function not inlined */
#pragma warning(disable : 4711) /* function 'xyz' selected for                 \
                                   automatic inline expansion */
#pragma warning(disable : 4127) /* conditional expression is constant */
#pragma warning(disable : 4702) /* unreachable code */

#else
#warning "Unsupported C/C++ compiler"
#endif /* _MSC_VER */
