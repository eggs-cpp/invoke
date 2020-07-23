# Eggs.Invoke
#
# Copyright Agustin K-ballo Berge, Fusion Fenix 2017-2020
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.1)

set(_benchmark_report YES)
if (NOT CMAKE_GENERATOR MATCHES "Make|Ninja")
  set(_benchmark_report NO)
  set(_benchmark_report_warning
    "CXX_COMPILER_LAUNCHER not supported by this generator, "
    "needed for generating benchmark reports.")
else()
  find_package(Python3 3.6)
  if (NOT Python3_FOUND)
    set(_benchmark_report NO)
    set(_benchmark_report_warning
      "Python3 not found, needed for generating benchmark reports.")
  endif()
endif()

function(add_benchmark NAME INPUT)
  cmake_parse_arguments("ARG"
    ""
    "INSTANTIATIONS;OUTPUT;REPORT_TEMPLATE"
    "CASES"
    ${ARGN})
  if (NOT DEFINED ARG_INSTANTIATIONS)
    set(ARG_INSTANTIATIONS 10000)
  endif()
  if (NOT DEFINED ARG_OUTPUT)
    set(ARG_OUTPUT ${NAME}.html)
  endif()

  set(_instantiations "")
  foreach(_i RANGE 1 ${ARG_INSTANTIATIONS})
    string(APPEND _instantiations "\n    benchmark<${_i}>();")
  endforeach()

  set(_cases)
  set(_labels)
  while (ARG_CASES)
    list(GET ARG_CASES 0 _case)
    list(APPEND _cases ${_case})
    list(GET ARG_CASES 1 _label)
    list(APPEND _labels ${_label})
    list(REMOVE_AT ARG_CASES 0 1)
  endwhile()
  list(APPEND _cases 0)
  list(APPEND _labels "baseline")

  set(_sources)
  get_filename_component(_base ${INPUT} NAME_WE)
  foreach(_benchmark ${_cases})
    set(_output ${CMAKE_CURRENT_BINARY_DIR}/${_base}.${_benchmark}.cpp)
    set(BENCHMARK_INSTANTIATIONS "
#if !defined BENCHMARK_WARMUP
void benchmark${_benchmark}() {${_instantiations}\n}
#endif
")
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
  set_target_properties(${NAME} PROPERTIES
    JOB_POOL_COMPILE "console")

  if (NOT _benchmark_report)
    message(WARNING ${_benchmark_report_warning})
  else()
    string(REPLACE ";" "|" _cases "${_cases}")
    string(REPLACE ";" "|" _labels "${_labels}")
    set_property(TARGET ${NAME}
      PROPERTY CXX_COMPILER_LAUNCHER
        ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/compiler_launcher.py
          --compiler_id=${CMAKE_CXX_COMPILER_ID}
          --compiler_version=${CMAKE_CXX_COMPILER_VERSION}
          --target=${NAME}
          --cases=${_cases}
          --labels=${_labels}
          --instantiations=${ARG_INSTANTIATIONS}
          --object=<OBJECT>
          --)

    get_filename_component(INPUT ${INPUT} ABSOLUTE)
    get_filename_component(ARG_REPORT_TEMPLATE ${ARG_REPORT_TEMPLATE} ABSOLUTE)
    get_filename_component(ARG_OUTPUT ${ARG_OUTPUT} ABSOLUTE
      BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})
    add_custom_command(
      OUTPUT ${ARG_OUTPUT}
      COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/generate_report.py
        --target=${NAME}
        --source=${INPUT}
        --instantiations=${ARG_INSTANTIATIONS}
        --output=${ARG_OUTPUT}
        --template=${ARG_REPORT_TEMPLATE}
        --objects="$<TARGET_OBJECTS:${NAME}>"
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS ${NAME} ${ARG_REPORT_TEMPLATE} $<TARGET_OBJECTS:${NAME}>)

    add_custom_target(${NAME}.report DEPENDS ${ARG_OUTPUT})
    set_property(TARGET ${NAME}.report PROPERTY FOLDER "benchmark/report")
  endif()
endfunction()

set(_benchmarks
  callable
  function
  member_function
  member_object)

add_custom_target(benchmark)
set_property(TARGET benchmark PROPERTY FOLDER "benchmark")

foreach(_benchmark ${_benchmarks})
  add_benchmark(benchmark.${_benchmark} ${_benchmark}.cpp.in
    INSTANTIATIONS ${BENCHMARK_INSTANTIATIONS}
    REPORT_TEMPLATE template/report.html.in
    OUTPUT reports/${_benchmark}.html
    CASES
      1 "plain call"
      2 "std::invoke"
      3 "eggs::invoke")
  target_link_libraries(benchmark.${_benchmark} Eggs::Invoke)
  set_property(TARGET benchmark.${_benchmark} PROPERTY FOLDER "benchmark")

  add_dependencies(benchmark benchmark.${_benchmark})
  if (_benchmark_report)
    add_dependencies(benchmark benchmark.${_benchmark}.report)
  endif()
endforeach()
