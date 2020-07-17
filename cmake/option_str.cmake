# Copyright Agustin K-ballo Berge, Fusion Fenix 2020
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Provide an option that the user can optionally select.
#
#     option_str(<variable> "<help_text>" <value>...)
#
# Provides an option for the user to select. If <variable> is already set as a
# normal or cache variable, then the command does nothing (see policy CMP0077).
function(option_str variable helpstring)
  set(_value ${ARGN})

  # see if a cache variable with this name already exists
  if (DEFINED CACHE{${variable}})
    # if so just make sure the doc state is correct
    get_property(_type CACHE ${variable} PROPERTY TYPE)
    if (NOT _type STREQUAL "UNINITIALIZED")
      set_property(CACHE ${variable} PROPERTY HELPSTRING "${helpstring}")
    endif()
    return()
  endif()

  # see if a local variable with this name already exists
  if (DEFINED ${variable})
    cmake_policy(GET CMP0077 _policy_status)
    if (_policy_status STREQUAL "NEW")
      # if so we ignore the option_str command
      return()
    else()
      message(AUTHOR_WARNING
        "Policy CMP0077 is not set: option() honors normal variables. "
        "Run \"cmake --help-policy CMP0077\" for policy details. "
        "Use the cmake_policy command to set the policy and suppress this warning.\n"
        "For compatibility with older versions of CMake, option_str is clearing "
        "the normal variable '${variable}'.")
    endif()
  endif()

  # nothing in the cache so add it
  set(${variable} "${_value}" CACHE STRING "${helpstring}")
endfunction()
