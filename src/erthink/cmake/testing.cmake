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

cmake_minimum_required(VERSION 3.8.2)
include(CTest)
if(BUILD_TESTING)
  cmake_policy(PUSH)

  # Expected GTest was already found and/or pointed via ${gtest_root},
  # otherwise will search at ${gtest_paths} locations, if defined or default ones.
  find_package(GTest)

  if(NOT GTEST_FOUND)
    message(STATUS "Lookup GoogleTest sources...")
    if(NOT gtest_paths)
      if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        set(gtest_paths
          $ENV{SystemDrive}/
          $ENV{SystemDrive}/Source $ENV{USERPROFILE}/Source $ENV{PUBLIC}/Source
          $ENV{SystemDrive}/Source/Repos $ENV{USERPROFILE}/Source/Repos $ENV{PUBLIC}/Source/Repos)
      else()
        set(gtest_paths /usr/src /usr/local /usr/local/src)
      endif()
    endif()
    # message(STATUS "gtest_paths = ${gtest_paths}")

    find_path(gtest_root
      NAMES CMakeLists.txt
      PATHS ${gtest_paths} "${CMAKE_BINARY_DIR}/googletest-src"
      PATH_SUFFIXES googletest/googletest gtest/googletest gtest
      NO_DEFAULT_PATH NO_CMAKE_PATH)

    if(gtest_root)
      message(STATUS "Found GoogleTest sources at ${gtest_root}")
    else()
      message(STATUS "Not found GoogleTest sources, downloading it...")
      configure_file(${CMAKE_CURRENT_LIST_DIR}/googletest-download.cmake.in ${CMAKE_BINARY_DIR}/googletest-download/CMakeLists.txt)
      execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)
      if(result)
        message(FATAL_ERROR "CMake step for GoogleTest failed: ${result}")
      else()
        execute_process(COMMAND ${CMAKE_COMMAND} --build .
          RESULT_VARIABLE result
          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
        if(result)
          message(FATAL_ERROR "Build step for GoogleTest failed: ${result}")
        else()
          set(gtest_root "${CMAKE_BINARY_DIR}/googletest-src/googletest")
        endif()
      endif()
    endif()

    if(gtest_root)
      unset(GTEST_INCLUDE_DIR CACHE)
      unset(GTEST_LIBRARY CACHE)
      unset(GTEST_LIBRARY_DEBUG CACHE)
      unset(GTEST_MAIN_LIBRARY CACHE)
      unset(GTEST_MAIN_LIBRARY_DEBUG CACHE)
      unset(GTEST_BOTH_LIBRARIES CACHE)
      unset(GTEST_FOUND CACHE)

      # Prevent overriding the parent project's compiler/linker
      # settings on Windows
      set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

      # Add googletest directly to our build. This defines
      # the gtest and gtest_main targets.
      add_subdirectory(${gtest_root} ${CMAKE_BINARY_DIR}/googletest-build EXCLUDE_FROM_ALL)
      if(CMAKE_INTERPROCEDURAL_OPTIMIZATION AND NOT CMAKE_VERSION VERSION_LESS 3.9)
        file(READ ${gtest_root}/CMakeLists.txt variable gtest_cmake_content)
        string(TOLOWER "${gtest_cmake_content}" gtest_cmake_content)
        string(FIND "${gtest_cmake_content}" "check_ipo_supported" gtest_ipo_supported)
        if(NOT gtest_ipo_supported OR gtest_ipo_supported LESS 1)
          message(STATUS "Disable INTERPROCEDURAL_OPTIMIZATION for GoogleTest (CMake-3.9's check_ipo_supported NOT FOUND inside)")
          if(TARGET gtest)
            set_property(TARGET gtest gtest_main PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)
          endif()
          if(TARGET gmock)
            set_property(TARGET gmock gmock_main PROPERTY INTERPROCEDURAL_OPTIMIZATION FALSE)
          endif()
        endif()
      endif()

      if(CC_HAS_WERROR)
        if(MSVC)
          set(local_warn_no_error "/WX-")
        else()
          set(local_warn_no_error "-Wno-error")
        endif()
        if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug"
            AND NOT CMAKE_INTERPROCEDURAL_OPTIMIZATION AND NOT LTO_ENABLED)
          if (CC_HAS_OMINIMAL)
            set(local_minimal_optimize "-Ominimal")
          elseif(CMAKE_COMPILER_IS_CLANG OR CMAKE_COMPILER_IS_GNUCC)
            set(local_minimal_optimize "-O1")
          else()
            set(local_minimal_optimize "")
          endif()
        endif()
        target_compile_options(gtest PRIVATE ${local_warn_no_error} ${local_minimal_optimize})
        target_compile_options(gtest_main PRIVATE ${local_warn_no_error} ${local_minimal_optimize})
        unset(local_warn_no_error)
        unset(local_minimal_optimize)
      endif()

      set(GTEST_BOTH_LIBRARIES gtest gtest_main)
      set(GTEST_FOUND TRUE)
    endif()
  endif()

  if(GTEST_FOUND)
    enable_testing()
    set(UT_LIBRARIES ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    if(MEMORYCHECK_COMMAND OR CMAKE_MEMORYCHECK_COMMAND)
      add_custom_target(test_memcheck
        COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --test-action memcheck
        COMMAND ${CAT} "${CMAKE_BINARY_DIR}/Testing/Temporary/MemoryChecker.*.log")
    endif()
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
      # Windows don't have RPATH feature,
      # therefore we should prepare PATH or copy DLL(s)
      set(UT_NEED_DLLCRUTCH "Crutch for ${CMAKE_SYSTEM_NAME}")
      if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_VERSION VERSION_LESS 3.0)
        # will use LOCATION property to compose DLLPATH
        cmake_policy(SET CMP0026 OLD)
      endif()
    else()
      set(UT_NEED_DLLCRUTCH FALSE)
    endif()
  else()
    set(UT_INCLUDE_DIRECTORIES "")
    set(UT_LIBRARIES "")
    message(STATUS "GoogleTest NOT available, so testing NOT be ENABLED")
  endif()

  function(add_gtest name)
    set(options DISABLED)
    set(oneValueArgs TIMEOUT PREFIX)
    set(multiValueArgs SOURCE LIBRARY INCLUDE_DIRECTORY DEPEND DLLPATH)
    cmake_parse_arguments(params "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(params_UNPARSED_ARGUMENTS)
      message(FATAL_ERROR "Unknown keywords given to add_gtest(): \"${params_UNPARSED_ARGUMENTS}\".")
    endif()

    if(GTEST_FOUND)
      macro(oops)
        message(FATAL_ERROR "add_gtest(): Opps, " ${ARGV})
      endmacro()

      if(NOT params_SOURCE)
        set(params_SOURCE ${name}.cpp)
      endif()

      set(target "${params_PREFIX}${name}")
      add_executable(${target} ${params_SOURCE})
      set_target_properties(${target} PROPERTIES SKIP_BUILD_RPATH FALSE)

      if(params_DEPEND)
        add_dependencies(${target} ${params_DEPEND})
      endif()

      target_link_libraries(${target} ${UT_LIBRARIES})

      if(params_LIBRARY)
        target_link_libraries(${target} ${params_LIBRARY})
      endif()

      if(UT_NEED_DLLCRUTCH)
        string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UPPERCASE)
        foreach(dep IN LISTS params_LIBRARY GTEST_BOTH_LIBRARIES)
          get_target_property(type ${dep} TYPE)
          if(type STREQUAL SHARED_LIBRARY)
            # Windows don't have RPATH feature,
            # therefore we should prepare PATH or copy DLL(s)...
            if(CMAKE_CONFIGURATION_TYPES)
              # Could not provide static ENVIRONMENT property with configuration-depended path
              set(dir FALSE)
            else(CMAKE_CONFIGURATION_TYPES)
              get_target_property(filename ${dep} IMPORTED_LOCATION_${CMAKE_BUILD_TYPE_UPPERCASE})
              if(NOT filename)
                get_target_property(filename ${dep} IMPORTED_LOCATION)
              endif()
              get_target_property(filename ${dep} LOCATION_${CMAKE_BUILD_TYPE_UPPERCASE})
              if(NOT filename)
                get_target_property(filename ${dep} LOCATION)
              endif()
              if(filename)
                get_filename_component(dir ${filename} DIRECTORY)
              else(filename)
                get_target_property(dir ${dep} LIBRARY_OUTPUT_DIRECTORY_${CMAKE_BUILD_TYPE_UPPERCASE})
                if(NOT dir)
                  get_target_property(dir ${dep} RUNTIME_OUTPUT_DIRECTORY_${CMAKE_BUILD_TYPE_UPPERCASE})
                endif()
                if(NOT dir)
                  get_target_property(dir ${dep} LIBRARY_OUTPUT_DIRECTORY)
                endif()
                if(NOT dir)
                  get_target_property(dir ${dep} RUNTIME_OUTPUT_DIRECTORY)
                endif()
              endif(filename)
            endif(CMAKE_CONFIGURATION_TYPES)
            if(dir)
              list(APPEND params_DLLPATH ${dir})
            else(dir)
              # Path is configuration-depended or not available, should copy dll
              add_custom_command(TARGET ${target} POST_BUILD
                COMMAND if exist "$<TARGET_PDB_FILE:${dep}>"
                ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_PDB_FILE:${dep}>" "$<TARGET_FILE_DIR:${target}>")
              add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${dep}>" "$<TARGET_FILE_DIR:${target}>"
                COMMENT "${UT_NEED_DLLCRUTCH}: Copy shared library ${dep} for test ${target}")
            endif(dir)
          endif()
        endforeach(dep)
      endif(UT_NEED_DLLCRUTCH)

      if(params_INCLUDE_DIRECTORY)
        set_target_properties(${target} PROPERTIES INCLUDE_DIRECTORIES ${params_INCLUDE_DIRECTORY})
      endif()

      if(NOT params_DISABLED)
        add_test(${name} ${target})
        if(params_TIMEOUT)
          if(MEMORYCHECK_COMMAND OR CMAKE_MEMORYCHECK_COMMAND)
            # FIXME: unless there are any other ideas how to fix the
            #        timeouts problem when testing under Valgrind.
            math(EXPR params_TIMEOUT "${params_TIMEOUT} * 42")
          endif()
          set_tests_properties(${name} PROPERTIES TIMEOUT ${params_TIMEOUT})
        endif()
        if(params_DLLPATH)
          # Compose DLL's path in the ENVIRONMENT property
          if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
            set(params_DLLPATH_ENV "${params_DLLPATH};$ENV{PATH}")
          else()
            set(params_DLLPATH_ENV "${params_DLLPATH}:$ENV{PATH}")
            string(REPLACE ":" ";" params_DLLPATH_ENV "${params_DLLPATH_ENV}")
          endif()
          list(REMOVE_DUPLICATES params_DLLPATH_ENV)
          if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
            string(REPLACE ";" "\\;" params_DLLPATH_ENV "${params_DLLPATH_ENV}")
          else()
            string(REPLACE ";" ":" params_DLLPATH_ENV "${params_DLLPATH_ENV}")
          endif()
          set_tests_properties(${name} PROPERTIES ENVIRONMENT "PATH=${params_DLLPATH_ENV}")
        endif()
      endif()
    endif()
  endfunction(add_gtest)

  function(add_ut name)
    add_gtest(${name} PREFIX "ut_" ${ARGN})
  endfunction(add_ut)

  function(add_long_test name)
    add_gtest(${name} PREFIX "lt_" DISABLED ${ARGN})
  endfunction(add_long_test)

  function(add_perf_test name)
    add_gtest(${name} PREFIX "pt_" DISABLED ${ARGN})
  endfunction(add_perf_test)

  cmake_policy(POP)
endif(BUILD_TESTING)
