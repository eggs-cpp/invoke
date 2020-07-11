# Eggs.Invoke
#
# Copyright Agustin K-ballo Berge, Fusion Fenix 2017-2020
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.1)

function(add_benchmark NAME INPUT)
  cmake_parse_arguments("ARG"
    ""
    "INSTANTIATIONS"
    ""
    ${ARGN})
  if (NOT DEFINED ARG_INSTANTIATIONS)
    set(ARG_INSTANTIATIONS 10000)
  endif()

  set(_instantiations "")
  foreach(_i RANGE 1 ${ARG_INSTANTIATIONS})
    string(APPEND _instantiations "\n    benchmark<${_i}>();")
  endforeach()

  set(_sources)
  get_filename_component(_base ${INPUT} NAME_WE)
  foreach(_benchmark 0 1 2 3)
    set(_output ${CMAKE_CURRENT_BINARY_DIR}/${_base}.${_benchmark}.cpp)
    set(BENCHMARK_INSTANTIATIONS "void benchmark${_benchmark}() {${_instantiations}\n}")
    configure_file(${INPUT} ${_output} @ONLY)

    set_source_files_properties(${_output} PROPERTIES
      COMPILE_DEFINITIONS BENCHMARK=${_benchmark}
      INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR})
    list(APPEND _sources ${input} ${_output})
  endforeach()

  add_library(${NAME} OBJECT EXCLUDE_FROM_ALL ${_sources})
  if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${NAME} PRIVATE -ftime-report)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${NAME} PRIVATE /bigobj /Bt+)
  endif()
endfunction()

set(_benchmarks
  callable
  function
  member_function
  member_object)
add_custom_target(benchmark)
foreach(_benchmark ${_benchmarks})
  add_benchmark(benchmark.${_benchmark} ${_benchmark}.cpp.in)
  target_link_libraries(benchmark.${_benchmark} Eggs::Invoke)
  set_property(TARGET benchmark.${_benchmark} PROPERTY FOLDER "benchmark")

  add_dependencies(benchmark benchmark.${_benchmark})
endforeach()
