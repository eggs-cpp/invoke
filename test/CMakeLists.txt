# Eggs.Invoke
#
# Copyright Agustin K-ballo Berge, Fusion Fenix 2017-2020
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(_tests
  invoke
  invoke_result
  is_invocable
  macro)
foreach(_test ${_tests})
  set(_test_file ${_test}.cpp)
  string(REPLACE "/" "." _test ${_test})

  add_executable(test.${_test} ${_test_file})
  target_link_libraries(test.${_test} Eggs::Invoke)
  set_property(TARGET test.${_test} PROPERTY FOLDER "test")

  add_test(
    NAME test.${_test}
    COMMAND test.${_test})
endforeach()
