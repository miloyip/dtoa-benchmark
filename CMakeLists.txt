## Copyright (c) 2012-2019 Leonid Yuriev <leo@yuriev.ru>.
## ZLib License
##
## This software is provided 'as-is', without any express or implied
## warranty. In no event will the authors be held liable for any damages
## arising from the use of this software.
##
## Permission is granted to anyone to use this software for any purpose,
## including commercial applications, and to alter it and redistribute it
## freely, subject to the following restrictions:
##
## 1. The origin of this software must not be misrepresented; you must not
##    claim that you wrote the original software. If you use this software
##    in a product, an acknowledgement in the product documentation would be
##    appreciated but is not required.
## 2. Altered source versions must be plainly marked as such, and must not be
##    misrepresented as being the original software.
## 3. This notice may not be removed or altered from any source distribution.

##
##  This CMakeLists.txt required for testing, but most things are header-only.
##  https://github.com/leo-yuriev/erthink
##

cmake_minimum_required(VERSION 3.8.2)
cmake_policy(SET CMP0054 NEW)
if(NOT CMAKE_VERSION VERSION_LESS 3.12)
  cmake_policy(SET CMP0075 NEW)
endif()

if(DEFINED PROJECT_NAME)
  set(SUBPROJECT ON)
else()
  set(SUBPROJECT OFF)
  project(erthink CXX)
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

include(CheckIncludeFiles)
include(CheckCXXSourceRuns)
include(CheckCXXSourceCompiles)
include(CheckFunctionExists)
include(CMakeDependentOption)
#
# Set default build type to Release. This is to ease a User's life.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
endif()
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UPPERCASE)

list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_std_17 HAS_CXX17)
list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_std_14 HAS_CXX14)

if(NOT DEFINED ERTHINK_CXX_STANDARD)
  if(SUBPROJECT AND DEFINED CMAKE_CXX_STANDARD)
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

option(ERTHINK_ENABLE_TESTS "Build erthink tests." ON)
option(BUILD_SHARED_LIBS "Build shared libraries (DLLs)." OFF)
if(NOT SUBPROJECT)
  option(CMAKE_POSITION_INDEPENDENT_CODE "Generate position independed (PIC)." ON)

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

  if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD ${ERTHINK_CXX_STANDARD})
  endif()
  include(cmake/testing.cmake)

  #
  # Enable 'make tags' target.
  find_program(CTAGS ctags)
  if(CTAGS)
    add_custom_target(tags COMMAND ${CTAGS} -R -f tags
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    add_custom_target(ctags DEPENDS tags)
  endif(CTAGS)

  #
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

#######################################################################

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_library(erthink INTERFACE)

set(erthink_HEADERS erthink_defs.h erthink_byteorder.h erthink_intrin.h erthink_arch.h
  erthink_bswap.h erthink_rot.h erthink_carryadd.h erthink_mul.h erthink_clz.h
  erthink_u2a.h erthink_d2a.h erthink_misc.h erthink.h)

target_compile_features(erthink INTERFACE "cxx_std_${ERTHINK_CXX_STANDARD}")

if(ERTHINK_ENABLE_TESTS)
  add_subdirectory(test)
endif()
