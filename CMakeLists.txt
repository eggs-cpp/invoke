# Eggs.Invoke
#
# Copyright Agustin K-ballo Berge, Fusion Fenix 2017-2020
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.0)

project(Eggs.Invoke CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  include(CTest) # option(BUILD_TESTING ...)
  option(BUILD_BENCHMARKS "Build the benchmarks" OFF)
  option(BUILD_EXAMPLE "Build the example" ON)
  option(ENABLE_INSTALL "Enable installing the library" ON)
else()
  option(EGGS_INVOKE_BUILD_BENCHMARKS "Build the benchmarks" OFF)
  option(EGGS_INVOKE_BUILD_EXAMPLE "Build the example" OFF)
  option(EGGS_INVOKE_BUILD_TESTING "Build the testing tree" OFF)
  option(EGGS_INVOKE_ENABLE_INSTALL "Enable installing the library" OFF)
  set(BUILD_BENCHMARKS ${EGGS_INVOKE_BUILD_BENCHMARKS})
  set(BUILD_EXAMPLE ${EGGS_INVOKE_BUILD_EXAMPLE})
  set(BUILD_TESTING ${EGGS_INVOKE_BUILD_TESTING})
  set(ENABLE_INSTALL ${EGGS_INVOKE_ENABLE_INSTALL})
endif()

# Build
add_library(_eggs_invoke INTERFACE)
target_include_directories(_eggs_invoke INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
set_target_properties(_eggs_invoke
  PROPERTIES EXPORT_NAME Eggs::Invoke)

add_library(Eggs::Invoke ALIAS _eggs_invoke)

# Benchmarks
if (BUILD_BENCHMARKS)
  add_subdirectory(benchmark)
endif()

# Example
if (BUILD_EXAMPLE)
  add_subdirectory(example)
endif()

# Test
if (BUILD_TESTING)
  if (NOT CMAKE_TESTING_ENABLED)
    message(AUTHOR_WARNING "Tests enabled but enable_testing() not called")
  endif()
  add_subdirectory(test)
endif()

# Install
if (ENABLE_INSTALL)
  install(DIRECTORY include/eggs DESTINATION include)

  install(TARGETS _eggs_invoke EXPORT _targets)
  install(EXPORT _targets
    DESTINATION lib/cmake/eggs.invoke
    FILE eggs.invoke-config.cmake)
endif()
