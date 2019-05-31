cmake_minimum_required(VERSION 3.8.2)
cmake_policy(SET CMP0054 NEW)
if(NOT CMAKE_VERSION VERSION_LESS 3.12)
  cmake_policy(SET CMP0075 NEW)
endif()

project(d2a-benchmark C CXX)

#
# Set default build type to Release. This is to ease a User's life.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
endif()
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UPPERCASE)

if(NOT DEFINED CXX_STANDARD)
  list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_std_17 HAS_CXX17)
  list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_std_14 HAS_CXX14)
  list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_std_11 HAS_CXX11)

  if(NOT HAS_CXX17 LESS 0)
    set(CXX_STANDARD 17)
  elseif(NOT HAS_CXX14 LESS 0)
    set(CXX_STANDARD 14)
  else()
    set(CXX_STANDARD 11)
  endif()
endif()
message(STATUS "Use C++${CXX_STANDARD}")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

set(SOURCES
  #stb_sprintf.cpp
  #stb_sprintf/stb_sprintf.h
  src/double-conversion/bignum-dtoa.cc
  src/double-conversion/bignum-dtoa.h
  src/double-conversion/bignum.cc
  src/double-conversion/bignum.h
  src/double-conversion/cached-powers.cc
  src/double-conversion/cached-powers.h
  src/double-conversion/diy-fp.cc
  src/double-conversion/diy-fp.h
  src/double-conversion/double-conversion.cc
  src/double-conversion/double-conversion.h
  src/double-conversion/fast-dtoa.cc
  src/double-conversion/fast-dtoa.h
  src/double-conversion/fixed-dtoa.cc
  src/double-conversion/fixed-dtoa.h
  src/double-conversion/ieee.h
  src/double-conversion/strtod.cc
  src/double-conversion/strtod.h
  src/double-conversion/utils.h
  src/doubleconvtest.cpp

  src/emyg/emyg_dtoa.c
  src/emyg/emyg_dtoa.h
  src/emygtest.cpp

  src/erthink/erthink.h
  src/erthink/erthink_arch.h
  src/erthink/erthink_bswap.h
  src/erthink/erthink_byteorder.h
  src/erthink/erthink_carryadd.h
  src/erthink/erthink_clz.h
  src/erthink/erthink_d2a.h
  src/erthink/erthink_defs.h
  src/erthink/erthink_intrin.h
  src/erthink/erthink_misc.h
  src/erthink/erthink_mul.h
  src/erthink/erthink_rot.h
  src/erthink/erthink_u2a.h
  src/erthink_test.cpp

  src/ryu/common.h
  src/ryu/d2fixed_full_table.h
  src/ryu/d2s_full_table.h
  src/ryu/d2s_intrinsics.h
  src/ryu/f2s.c
  src/ryu/generic_128.h
  src/ryu/ryu_generic_128.h
  src/ryu/d2fixed.c
  src/ryu/d2s.c
  src/ryu/d2s.h
  src/ryu/digit_table.h
  src/ryu/generic_128.c
  src/ryu/ryu2.h
  src/ryu/ryu.h
  src/ryu_test.cpp

  src/floaxie/atof.h
  src/floaxie/bit_ops.h
  src/floaxie/cached_power.h
  src/floaxie/conversion_status.h
  src/floaxie/default_fallback.h
  src/floaxie/diy_fp.h
  src/floaxie/fraction.h
  src/floaxie/ftoa.h
  src/floaxie/grisu.h
  src/floaxie/huge_val.h
  src/floaxie/integer_of_size.h
  src/floaxie/k_comp.h
  src/floaxie/krosh.h
  src/floaxie/memwrap.h
  src/floaxie/powers_ten.h
  src/floaxie/powers_ten_double.h
  src/floaxie/powers_ten_single.h
  src/floaxie/prettify.h
  src/floaxie/print.h
  src/floaxie/static_pow.h
  src/floaxie/type_punning_cast.h
  src/floaxietest.cpp

  src/fmt/core.h src/fmt/format.h src/fmt/format-inl.h
  src/fmttest.cpp

  src/fpconv/fpconv.c src/fpconv/fpconv.h src/fpconv/license src/fpconv/powers.h
  src/fpconvtest.cpp

  src/gay/dtoa.c src/gay/g_fmt.c
  src/gaytest.cpp

  src/grisu/diy_fp.h src/grisu/double.h src/grisu/fast_exponent.h
  src/grisu/grisu2.h src/grisu/grisu2b_59_56.c src/grisu/k_comp.h
  src/grisu/powers.h src/grisu/powers_ten_round64.h
  src/grisu/prettify.h
  src/grisu2btest.cpp

  src/milo/dtoa_milo.h src/milotest.cpp
  src/msinttypes/inttypes.h src/msinttypes/stdint.h

  src/ostringstream.cpp
  src/ostrstream.cpp
  src/sprintf.cpp

  src/resultfilename.h
  src/test.h
  src/timer.h
  src/null.cpp
  src/main.cpp
  )

add_executable(d2a-benchmark ${SOURCES})

if(CMAKE_COMPILER_IS_GNUCXX)
target_compile_options(d2a-benchmark PRIVATE "-ffast-math;-march=native")
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
target_compile_definitions(d2a-benchmark PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
target_compile_features(d2a-benchmark PRIVATE "cxx_std_${CXX_STANDARD}")
