#!/usr/bin/env python3

import sys
import pathlib
import json

def main():
    file_name = 'make.log.out'

    with open(file_name) as f:
        warnings = json.load(f)
        for k, v in warnings.items():
            if k == sys.argv[1]:
                for l in v:
                    print('\n--------------------------------------------')
                    print(l[0])
                    print(str(l[1]).replace('\\n', '\n'))


if __name__ == '__main__':
    main()
