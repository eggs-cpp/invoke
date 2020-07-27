# Eggs.Invoke
#
# Copyright Agustin K-ballo Berge, Fusion Fenix 2017-2020
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.0...3.18)

project(Eggs.Invoke_Example CXX)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(Eggs.Invoke_SOURCE_DIR "" CACHE PATH "Path to Eggs.Invoke source root")

  if (Eggs.Invoke_SOURCE_DIR)
    message(STATUS "Embedding Eggs.Invoke via add_subdirectory")
    if (NOT EXISTS ${Eggs.Invoke_SOURCE_DIR}/CMakeLists.txt)
      message(FATAL_ERROR "Given path to Eggs.Invoke is invalid")
    endif()
    add_subdirectory(${Eggs.Invoke_SOURCE_DIR} eggs.invoke)
  else()
    if (DEFINED Eggs.Invoke_DIR)
      find_package(Eggs.Invoke CONFIG REQUIRED)
    else()
      find_package(Eggs.Invoke CONFIG)
    endif()

    if (Eggs.Invoke_FOUND)
      message(STATUS "Using Eggs.Invoke via config package file")
    else()
      include(FetchContent OPTIONAL RESULT_VARIABLE FetchContent_FOUND)
      if (NOT FetchContent_FOUND) # CMake 3.10 or older
        message(FATAL_ERROR "Eggs.Invoke not found and FetchContent not available")
      endif()

      message(STATUS "Embedding Eggs.Invoke via FetchContent")
      FetchContent_Declare(eggs.invoke
        GIT_REPOSITORY https://github.com/eggs-cpp/invoke.git)
      if (COMMAND FetchContent_MakeAvailable) # CMake 3.14 or newer
        message(VERBOSE "  using FetchContent_MakeAvailable")
        FetchContent_MakeAvailable(eggs.invoke)
      else()
        message(VERBOSE "  using FetchContent_Populate")
        FetchContent_GetProperties(eggs.invoke)
        if (NOT eggs.invoke_POPULATED)
          FetchContent_Populate(eggs.invoke)
          add_subdirectory(${eggs.invoke_SOURCE_DIR} ${eggs.invoke_BINARY_DIR})
        endif()
      endif()
    endif()
  endif()
endif()

add_executable(example example.cpp)
target_link_libraries(example Eggs::Invoke)
set_property(TARGET example PROPERTY FOLDER "example")
