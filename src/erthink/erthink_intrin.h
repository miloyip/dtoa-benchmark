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

#elif defined(_MSC_VER)

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
