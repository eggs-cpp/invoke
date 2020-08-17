**Eggs.Invoke**
==================

See results at http://eggs-cpp.github.io/invoke/benchmark.html.

## Benchmark ##

These benchmarks measure the build overhead (compilation time, memory usage,
object size) introduced by the use of different implementations of the
`INVOKE` abstraction.

These benchmarks do NOT look at runtime performance. `INVOKE` is a zero-cost
abstraction: no runtime overhead is introduced by its use in an _optimized_
build.

## Requirements ##

**C++17** is required for benchmarking `std::invoke`.

Python 3.6 and a Makefile or Ninja CMake generator is required for generating
benchmark reports.

## CI Status ##

![](https://github.com/eggs-cpp/invoke/workflows/Eggs.Invoke%20Benchmark/badge.svg?branch=master)

---

> Copyright _Agustín Bergé_, _Fusion Fenix_ 2017-2020
> 
> Distributed under the Boost Software License, Version 1.0. (See accompanying
> file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
