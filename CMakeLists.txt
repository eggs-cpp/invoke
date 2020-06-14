# Eggs.Invoke
#
# Copyright Agustin K-ballo Berge, Fusion Fenix 2017-2020
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.0)

project(Eggs.Invoke CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Build
add_library(_eggs_invoke INTERFACE)
target_include_directories(_eggs_invoke INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
set_target_properties(_eggs_invoke
  PROPERTIES EXPORT_NAME Eggs::Invoke)

add_library(Eggs::Invoke ALIAS _eggs_invoke)
