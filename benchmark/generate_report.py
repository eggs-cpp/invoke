# Eggs.Invoke
#
# Copyright Agustin K-ballo Berge, Fusion Fenix 2020
#
# Distributed under the Boost Software License, Version 1.0.  (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
import argparse
import html
import json
import os
import re

# ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/generate_report.py
#   --target=${NAME}
#   --source=${INPUT}
#   --instantiations=${ARG_INSTANTIATIONS}
#   --output=${OUTPUT}
#   --template=${ARG_REPORT_TEMPLATE}
#   --objects="$<TARGET_OBJECTS:${NAME}>"
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--target', required=True)
    parser.add_argument('--source', required=True)
    parser.add_argument('--instantiations', required=True)
    parser.add_argument('--output', required=True)
    parser.add_argument('--template', required=True)
    parser.add_argument('--objects', required=True)
    args = parser.parse_args()

    reports = []
    for object in args.objects.split(';'):
        with open(object + '.benchmark.json', 'r') as file:
            report = json.load(file)
        del report['target']

        if (report['benchmark'] == 0):
            baseline = report
        else:
            reports.append(report)

    datum = sorted(reports, key=lambda x: x['benchmark'])
    DATUM = json.dumps({ 'values' : reports }, indent=2)

    compilation_time = {
        'field': 'compilation_time',
        'key': 'Compilation time',
        'baseline': baseline['compilation_time'],
        'format': ',.2f',
        'unit': 's'
    }
    memory_usage = {
        'field': 'memory_usage',
        'key': 'Memory usage',
        'baseline': baseline['memory_usage'],
        'format': '.5s',
        'unit': 'B'
    }
    object_size = {
        'field': 'object_size',
        'key': 'Object size',
        'baseline': baseline['object_size'],
        'format': '.5s',
        'unit': 'B'
    }
    aspects = [compilation_time, memory_usage, object_size]
    ASPECTS = json.dumps(aspects, indent=2)

    with open(args.target + '.json', 'w') as file:
        json.dump({ 'aspects': aspects, 'values': reports }, file)

    with open(args.source, 'r') as file:
        source = file.read()
    source = source.replace('@BENCHMARK_INSTANTIATIONS@', '').rstrip()
    SOURCE_CODE = html.escape(source)

    INSTANTIATIONS = args.instantiations;

    template_dir = os.path.dirname(args.template)
    def INCLUDE(path):
        path = os.path.join(template_dir, path)
        with open(path, 'r') as file:
            return file.read()

    with open(args.template, 'r') as file:
        template = file.read()
    output = re.sub(r'@(.*)@', lambda m: str(eval(m.group(1))), template)
    with open(args.output, 'w') as file:
        file.write(output)
