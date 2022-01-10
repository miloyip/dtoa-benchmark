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

#if !(defined(__cplusplus) && __cplusplus >= 201103L)
#error "This source code requires C++11 at least."
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4505) /* unreferenced local function... */
#endif

#include "erthink.h"
#include "erthink_casting.h++"
#include "erthink_clz.h++"
#include "erthink_constexpr_cstr.h++"
#include "erthink_d2a.h++"
#include "erthink_dynamic_constexpr.h++"
#include "erthink_misc.h++"
#include "erthink_short_alloc.h++"
#include "erthink_u2a.h++"
#include "erthink_unaligned.h++"

#ifdef _MSC_VER
#pragma warning(pop)
#endif
