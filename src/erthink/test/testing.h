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
#define _USE_MATH_DEFINES
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#define _STL_WARNING_LEVEL 3
#pragma warning(disable : 4710) /* function not inlined */
#pragma warning(disable : 4711) /* function selecte for automatic inline */
#pragma warning(disable : 4571) /* catch(...) semantics changed since          \
                                   Visual C++ 7... */
#pragma warning(push, 1)
#pragma warning(disable : 4530) /* C++ exception handler used, but             \
                                   unwind semantics are not enabled.           \
                                   Specify /EHsc */
#pragma warning(disable : 4577) /* 'noexcept' used with no exception           \
                                   handling mode specified; termination on     \
                                   exception is not guaranteed.                \
                                   Specify /EHsc */
#pragma warning(disable : 4738) /* storing 32-bit float result in memory,      \
                                   possible loss of performance */
#if _MSC_VER > 1800
#pragma warning(disable : 4464) /* relative include path contains '..' */
#endif
#if _MSC_VER < 1900
/* LY: workaround for dead code:
       microsoft visual studio 12.0\vc\include\xtree(1826) */
#pragma warning(disable : 4702) /* unreachable code */
#endif
#endif /* _MSC_VER (warnings) */

#include <gtest/gtest.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

/* LY: reduce test runtime (significantly on Elbrus) */
#if defined(__LCC__) && defined(NDEBUG) && defined(__OPTIMIZE__) &&            \
    !defined(ENABLE_GPROF)
#undef SCOPED_TRACE
#define SCOPED_TRACE(message) __noop()
#endif /* __LCC__ */

//----------------------------------------------------------------------------

/* Ограничитель по времени выполнения.
 * Нужен для предотвращения таумаута тестов в CI. Предполагается, что он
 * используется вместе с установкой GTEST_SHUFFLE=1, что в сумме дает
 * выполнение части тестов в случайном порядке, пока не будет превышен лимит
 * заданный через переменную среды окружения GTEST_RUNTIME_LIMIT. */
class runtime_limiter {
  const time_t edge;

  static time_t fetch() {
    const char *GTEST_RUNTIME_LIMIT = getenv("GTEST_RUNTIME_LIMIT");
    if (GTEST_RUNTIME_LIMIT) {
      long limit = atol(GTEST_RUNTIME_LIMIT);
      if (limit > 0)
        return time(nullptr) + limit;
    }
    return 0;
  }

public:
  runtime_limiter() : edge(fetch()) {}

  bool is_timeout() {
    if (edge && time(nullptr) > edge) {
      const auto current = ::testing::UnitTest::GetInstance();
      static const void *last_reported;
      if (last_reported != current) {
        last_reported = current;
        std::cout << "[  SKIPPED ] RUNTIME_LIMIT was reached" << std::endl;
        GTEST_SUCCESS_("Skipped") << "SKIPPEND by RUNTIME_LIMIT";
      }
      return true;
    }
    return false;
  }
};

extern runtime_limiter ci_runtime_limiter;

#define GTEST_IS_EXECUTION_TIMEOUT() ci_runtime_limiter.is_timeout()
