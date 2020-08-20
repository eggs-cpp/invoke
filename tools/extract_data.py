# Eggs.Invoke
#
# Copyright Agustin K-ballo Berge, Fusion Fenix 2020
#
# Distributed under the Boost Software License, Version 1.0.  (See accompanying
# file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
import glob
import itertools
import json
import re

def overhead(aspect, values, baseline):
    value = float(values[aspect])
    base = float(baseline[aspect])
    return (value, value / base - 1.0)

if __name__ == '__main__':
    table = []
    for filename in glob.glob("benchmark.*.json"):
        with open(filename) as file:
            benchmark = json.load(file)

        template = re.match('^benchmark.([a-zA-Z_]+).json$', filename).group(1)
        for values in benchmark['values']:
            values['memory_usage'] = values['memory_usage'] / 1024.0 / 1024.0
            values['object_size'] = values['object_size'] / 1024.0 / 1024.0

            row = {}
            row['benchmark'] = template
            row['index'] = values['benchmark']
            row['case'] = values['label']
            row['compilation_time']  = values['compilation_time']
            row['memory_usage']  = values['memory_usage']
            row['object_size']  = values['object_size']
            table.append(row)

    results = []
    benchmark = lambda x: x['benchmark']
    table.sort(key=benchmark)
    grouped = itertools.groupby(table, key=benchmark)
    for key, group in grouped:
        values = list(group)
        for e in values:
            del e['benchmark']
            if e['index'] == 1:
                baseline = e
            del e['index']
        values.remove(baseline)

        result = {}
        result['benchmark'] = key
        result['godbolt'] = 'https://godbolt.org/z/'
        result['baseline'] = baseline
        result['values'] = values
        results.append(result)
    print(json.dumps(results, sort_keys=True) + ',')
