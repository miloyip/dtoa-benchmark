#pragma once

#include "util/config.h"

#include <cassert>
#include <cstddef>
#include <cstdint>

#if defined(_MSC_VER) && __cplusplus < 201703L
#    define NOEXCEPT throw()
#    define NOEXCEPT_IF(x)
#else  // defined(_MSC_VER) && __cplusplus < 201703L
#    define NOEXCEPT       noexcept
#    define NOEXCEPT_IF(x) noexcept(x)
#endif  // defined(_MSC_VER) && __cplusplus < 201703L

#if __cplusplus < 201703L
#    define CONSTEXPR inline
#else  // __cplusplus < 201703L
#    define CONSTEXPR constexpr
#endif  // __cplusplus < 201703L
