# Eggs.Invoke
#
# Copyright Agustin K-ballo Berge, Fusion Fenix 2020
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import argparse
import json
import os
import pathlib
import re
import subprocess
import sys
import time

# ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/compiler_launcher.py
#   --compiler_id=${CMAKE_CXX_COMPILER_ID}
#   --compiler_version=${CMAKE_CXX_COMPILER_VERSION}
#   --target=${NAME}
#   --cases=0|1|...
#   --labels=Baseline|Label 1|...
#   --instantiations=${ARG_INSTANTIATIONS}
#   --object=<OBJECT>
#   -- ...
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('cmd', nargs='+')
    parser.add_argument('--compiler_id', required=True)
    parser.add_argument('--compiler_version', required=True)
    parser.add_argument('--target', required=True)
    parser.add_argument('--cases', required=True)
    parser.add_argument('--labels', required=True)
    parser.add_argument('--instantiations', required=True)
    parser.add_argument('--object', required=True)
    args = parser.parse_args()

    target = args.target
    benchmark = next(x for x in args.cmd if x.startswith('DBENCHMARK=', 1))[len('-DBENCHMARK='):]
    case_index = args.cases.split('|').index(benchmark)
    label = args.labels.split('|')[case_index]
    instantiations = args.instantiations
    #print(f'subprocess.run({args.cmd})')

    # warm up
    process = subprocess.Popen(args.cmd + ['-DBENCHMARK_WARMUP'],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        universal_newlines=True)
    try:
        stdout, stderr = process.communicate(input)
    except:
        process.kill()
        raise
    returncode = process.poll()
    if returncode != 0:
        print(stdout)
        sys.exit(returncode)

    # run compiler
    start_time = time.time()
    process = subprocess.Popen(args.cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        universal_newlines=True)
    try:
        stdout, stderr = process.communicate(input)
    except:
        process.kill()
        raise
    returncode = process.poll()
    end_time = time.time()

    print(stdout)
    if returncode != 0:
        sys.exit(returncode)

    # extract information
    compilation_time = end_time - start_time
    memory_usage = '-'
    object_size = os.path.getsize(args.object)

    try:
        import resource
        rusage = resource.getrusage(resource.RUSAGE_CHILDREN)
        compilation_time = rusage.ru_utime + rusage.ru_stime
        memory_usage = rusage.ru_maxrss
    except ImportError:
        pass

    if args.compiler_id == 'GNU':
        # TOTAL : <USER_TIME> <SYSTEM_TIME> <WALL_TIME> <MEMORY_USAGE> kB
        m = re.search(
            '^ TOTAL +: +([0-9.]+) +([0-9.]+) +([0-9.]+) +([0-9]+) kB$',
            stdout,
            re.MULTILINE)
        if m:
            user_time = float(m.group(1))
            system_time = float(m.group(2))
            compilation_time = user_time + system_time
            wall_time = float(m.group(3))
            memory_usage = int(m.group(4))
    elif 'Clang' in args.compiler_id:
        pos = stdout.find("Clang front-end time report")
        if pos != -1:
            # <USER_TIME> (100.0%) <SYSTEM_TIME> (100.0%) <COMPILATION_TIME> (100.0%) <WALL_TIME> (100.0%) Total
            m = re.search(
                '^ +([0-9.]+) +\\(100.0%\\) +([0-9.]+) +\\(100.0%\\) +([0-9.]+) +\\(100.0%\\) +([0-9.]+) +\\(100.0%\\) +Total$',
                stdout[pos:], re.MULTILINE)
            if m:
                user_time = float(m.group(1))
                system_time = float(m.group(2))
                compilation_time = float(m.group(3))
                wall_time = float(m.group(4))
    elif args.compiler_id == 'MSVC':
        # time(*\c1xx.dll)=<COMPILATION_TIME>s < <START> - <END> > BB [<SOURCE>]
        # time(*\c2.dll)=<COMPILATION_TIME>s < <START> - <END> > BB [<SOURCE>]
        compilation_time = 0.0
        for m in re.finditer(
            '^time\\(.*\\)=([0-9.]+)s',
            stdout, re.MULTILINE):
            compilation_time += float(m.group(1))

    # print report
    print(f'Benchmark: {target}/{benchmark}, {label}')
    print(f'Instantiations: {instantiations}')
    print(f'Compilation time: {compilation_time} s')
    print(f'Memory usage: {memory_usage} kB')
    print(f'Object size: {object_size} B')

    # dump to json
    report = {
        'target': target,
        'benchmark': int(benchmark),
        'label': label,
        'compilation_time': compilation_time,
        'memory_usage': memory_usage * 1024 if memory_usage != '-' else '-',
        'object_size': object_size,
    }

    pathlib.Path(args.target).parent.mkdir(parents=True, exist_ok=True)
    with open(args.object + '.benchmark.json', 'w') as file:
        json.dump(report, file)
