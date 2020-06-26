# Eggs.Invoke
#
# Copyright Agustin K-ballo Berge, Fusion Fenix 2017-2020
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

add_executable(example example.cpp)
target_link_libraries(example Eggs::Invoke)
set_property(TARGET example PROPERTY FOLDER "example")
