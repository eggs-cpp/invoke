**Eggs.Invoke**
==================

## Example ##

This directory contains an example of use of **Eggs.Invoke**.

## Build

Execute the following commands from the `example` directory:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

When building the example cmake project as standalone, it will attempt to
locate **Eggs.Invoke** in a number of ways:

#### The `add_subdirectory` way

```cmake
add_subdirectory(${Eggs.Invoke_SOURCE_DIR} eggs.invoke)
```

If `Eggs.Invoke_SOURCE_DIR` is defined, embeds the project at the given path
as a subproject of the example.

[_Note_: the second argument is the corresponding binary directory, and it is
required when the source directory is absolute].

#### The `find_package` way

```cmake
find_package(Eggs.Invoke CONFIG)
```

Looks for the cmake package config file `eggs.invoke-config.cmake`, deployed
during installation of the library.

If the library has not been installed to a well-known system location, add the
installation prefix of the library to `CMAKE_PREFIX_PATH` or set
`Eggs.Invoke_DIR` to the directory containing `eggs.invoke-config.cmake`.

[_Note_: use argument `REQUIRED` for stopping the configuration process when
the package is not found].

#### The `FetchContent` way

```cmake
include(FetchContent)
FetchContent_Declare(eggs.invoke
  GIT_REPOSITORY https://github.com/eggs-cpp/invoke.git)
FetchContent_MakeAvailable(eggs.invoke)
```

Downloads the sources from github, then embeds the project as a subproject of
the example (as if by `add_subdirectory`).

---

> Copyright _Agustín Bergé_, _Fusion Fenix_ 2017-2020
> 
> Distributed under the Boost Software License, Version 1.0. (See accompanying
> file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
