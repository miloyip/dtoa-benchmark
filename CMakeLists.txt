cmake_minimum_required(VERSION 3.1.0)

project(dtoa-benchmark)

set(CMAKE_CXX_STANDARD 17)

add_executable(
  dtoa-benchmark
  src/main.cpp
  # Tests:
  src/doubleconvtest.cpp
  src/fpconvtest.cpp
  src/gaytest.cpp
  src/grisu2btest.cpp
  src/milotest.cpp
  src/null.cpp
  src/ostringstream.cpp
  src/ostrstream.cpp
  src/sprintf.cpp
  src/utilformat.cpp
  # Libraries:
  src/fpconv/fpconv.c
  src/gay/dtoa.c
  src/gay/g_fmt.c
  src/grisu/grisu2b_59_56.c
  src/double-conversion/bignum-dtoa.cc
  src/double-conversion/bignum.cc
  src/double-conversion/cached-powers.cc
  src/double-conversion/diy-fp.cc
  src/double-conversion/double-conversion.cc
  src/double-conversion/fast-dtoa.cc
  src/double-conversion/fixed-dtoa.cc
  src/double-conversion/strtod.cc
  src/util/string_cvt.cpp
)
target_include_directories(dtoa-benchmark PRIVATE src)
