##  Copyright (c) 2012-2021 Leonid Yuriev <leo@yuriev.ru>.
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

if(NOT DEFINED STDTHREAD_WORKS)
  if(CMAKE_VERSION VERSION_LESS 3.12)
    cmake_minimum_required(VERSION 3.8.2)
  else()
    cmake_minimum_required(VERSION 3.12)
  endif()

  cmake_policy(PUSH)
  cmake_policy(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})
  include(CMakePushCheckState)
  cmake_push_check_state()

  if(NOT DEFINED THREADS_PREFER_PTHREAD_FLAG)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
  endif()
  find_package(Threads)
  set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_THREAD_LIBS_INIT} ${CMAKE_REQUIRED_LIBRARIES}")

  set(std_thread_probe "
    #include <thread>
    void thread_proc() {}
    int main() {
    std::thread instance(thread_proc);
    instance.join();
    return 0;
    }")

  if(CMAKE_CROSSCOMPILING)
    include(CheckCXXSourceCompiles)
    check_cxx_source_compiles("${std_thread_probe}" STDTHREAD_WORKS)
    if(STDTHREAD_WORKS)
      message(WARNING
        "Cannot run probe in CrossCompiling mode to check whether `std::thread` works. "
        "So assuming it will works because compiled successfully.")
    endif()
  else()
    include(CheckCXXSourceRuns)
    check_cxx_source_runs("${std_thread_probe}" STDTHREAD_WORKS)
  endif()
  cmake_pop_check_state()

  set(STDTHREAD_WORKS "${STDTHREAD_WORKS}" CACHE BOOL "Whether C++11 std::thread works." FORCE)
  cmake_policy(POP)
endif()
