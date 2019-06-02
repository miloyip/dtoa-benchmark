##  Copyright (c) 2012-2019 Leonid Yuriev <leo@yuriev.ru>.
##
##  Licensed under the Apache License, Version 2.0 (the "License");
##  you may not use this file except in compliance with the License.
##  You may obtain a copy of the License at
##
##      http://www.apache.org/licenses/LICENSE-2.0
##
##  Unless required by applicable law or agreed to in writing, software
##  distributed under the License is distributed on an "AS IS" BASIS,
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##  See the License for the specific language governing permissions and
##  limitations under the License.
##

##
##  This CMakeLists.txt required for testing, but most things are header-only.
##  https://github.com/leo-yuriev/erthink
##

cmake_minimum_required(VERSION 3.8.2)
cmake_policy(PUSH)
cmake_policy(VERSION 3.8.2)
if(NOT CMAKE_VERSION VERSION_LESS 3.12)
  cmake_policy(SET CMP0075 NEW)
endif()
if(NOT CMAKE_VERSION VERSION_LESS 3.9)
  cmake_policy(SET CMP0069 NEW)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT CMAKE_INTERPROCEDURAL_OPTIMIZATION_AVAILABLE)
else()
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_AVAILABLE FALSE)
endif()

if(DEFINED PROJECT_NAME)
  set(SUBPROJECT ON)
else()
  set(SUBPROJECT OFF)
  project(erthink
    VERSION 0.1
    LANGUAGES CXX)
endif()

if(NOT "$ENV{TEAMCITY_PROCESS_FLOW_ID}" STREQUAL "")
  set(CI TEAMCITY)
  message(STATUS "TeamCity CI")
elseif(NOT "$ENV{TRAVIS}" STREQUAL "")
  set(CI TRAVIS)
  message(STATUS "Travis CI")
elseif(NOT "$ENV{CIRCLECI}" STREQUAL "")
  set(CI CIRCLE)
  message(STATUS "Circle CI")
elseif(NOT "$ENV{APPVEYOR}" STREQUAL "")
  set(CI APPVEYOR)
  message(STATUS "AppVeyor CI")
elseif(NOT "$ENV{CI}" STREQUAL "")
  set(CI "$ENV{CI}")
  message(STATUS "Other CI (${CI})")
else()
  message(STATUS "Assume No any CI environment")
  unset(CI)
endif()

include(CMakeDependentOption)
#
# Set default build type to Release. This is to ease a User's life.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
endif()
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UPPERCASE)

if (MSVC)
  if (MSVC_VERSION LESS 1915)
    message(FATAL_ERROR At least \"Microsoft C/C++ Compiler\" version 19.15.26730 (Visual Studio 2017 15.8) is required.)
  else()
    add_compile_options("/Zc:__cplusplus")
  endif()
endif()
list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_std_17 HAS_CXX17)
list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_std_14 HAS_CXX14)

if(NOT DEFINED ERTHINK_CXX_STANDARD)
  if(DEFINED CMAKE_CXX_STANDARD)
    set(ERTHINK_CXX_STANDARD ${CMAKE_CXX_STANDARD})
  elseif(NOT HAS_CXX17 LESS 0)
    set(ERTHINK_CXX_STANDARD 17)
  elseif(NOT HAS_CXX14 LESS 0)
    set(ERTHINK_CXX_STANDARD 14)
  else()
    set(ERTHINK_CXX_STANDARD 11)
  endif()
endif()
message(STATUS "Use C++${ERTHINK_CXX_STANDARD} for Erthink")

include(cmake/utils.cmake)
include(cmake/compiler.cmake)
include(cmake/profile.cmake)

if(NOT SUBPROJECT)
  option(CMAKE_POSITION_INDEPENDENT_CODE "Generate position independed (PIC)." ON)
  option(BUILD_SHARED_LIBS "Build shared libraries (DLLs)." OFF)

  find_program(VALGRIND valgrind)
  if(VALGRIND)
    # LY: cmake is ugly and nasty.
    #      - therefore memcheck-options should be defined before including ctest;
    #      - otherwise ctest may ignore it.
    set(MEMORYCHECK_SUPPRESSIONS_FILE
      "${PROJECT_SOURCE_DIR}/test/valgrind_suppress.txt"
      CACHE FILEPATH "Suppressions file for Valgrind" FORCE)
    set(MEMORYCHECK_COMMAND_OPTIONS
      "--trace-children=yes --leak-check=full --track-origins=yes --error-exitcode=42 --error-markers=@ --errors-for-leak-kinds=definite --fair-sched=yes --suppressions=${MEMORYCHECK_SUPPRESSIONS_FILE}"
      CACHE STRING "Valgrind options" FORCE)
    set(VALGRIND_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS}" CACHE STRING "Valgrind options" FORCE)
  endif()
  include(cmake/testing.cmake)

  if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD ${ERTHINK_CXX_STANDARD})
  endif()

  # Enable 'make tags' target.
  find_program(CTAGS ctags)
  if(CTAGS)
    add_custom_target(tags COMMAND ${CTAGS} -R -f tags
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    add_custom_target(ctags DEPENDS tags)
  endif(CTAGS)

  # Enable 'make reformat' target.
  find_program(CLANG_FORMAT
    NAMES clang-format-6.0 clang-format-5.0 clang-format-4.0
    clang-format-3.9 clang-format-3.8 clang-format-3.7 clang-format)
  if(CLANG_FORMAT AND UNIX)
    add_custom_target(reformat
      VERBATIM
      COMMAND
      git ls-files |
      grep -E \\.\(c|cxx|cc|cpp|h|hxx|hpp\)\(\\.in\)?\$ |
      xargs ${CLANG_FORMAT} -i --style=file
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  endif()

  find_program(GIT git)
  if(NOT "${PROJECT_BINARY_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}")
    add_custom_target(distclean)
    add_custom_command(TARGET distclean
      COMMAND ${CMAKE_COMMAND} -E remove_directory "${PROJECT_BINARY_DIR}"
      COMMENT "Removing the build directory and its content")
  elseif(IS_DIRECTORY .git AND GIT)
    add_custom_target(distclean)
    add_custom_command(TARGET distclean
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      COMMAND ${GIT} submodule foreach --recursive git clean -f -X -d
      COMMAND ${GIT} clean -f -X -d
      COMMENT "Removing all build files from the source directory")
  endif()
endif(NOT SUBPROJECT)

################################################################################

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_library(erthink INTERFACE)

set(erthink_HEADERS
  erthink_arch.h erthink_bswap.h erthink_byteorder.h erthink_carryadd.h
  erthink_clz.h erthink_d2a.h erthink_defs.h erthink_dynamic_constexpr.h
  erthink_intrin.h erthink_misc.h erthink_mul.h erthink_rot.h erthink_u2a.h
  erthink_optimize4size.h erthink_optimize4speed.h erthink_short_alloc.h
  erthink_ifunc.h erthink_endian.h erthink.h)

target_compile_features(erthink INTERFACE "cxx_std_${ERTHINK_CXX_STANDARD}")

option(ERTHINK_ENABLE_TESTS "Build erthink tests." ${BUILD_TESTING})
if(ERTHINK_ENABLE_TESTS)
  add_subdirectory(test)
endif()

cmake_policy(POP)
