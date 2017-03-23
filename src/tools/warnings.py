#!/usr/bin/env python3

import sys
import re
from enum import Enum
import pathlib
import json

class State(Enum):
    INFO    = 0
    WARNING = 1
    LINE    = 2

def main():
    file_name = 'make.log'

    html_directory = 'pages/'
    document_directory = 'documents/'

    currentBuffer = ''
    currentWarning = ''
    currentFile = ''
    state = State.INFO

    warnings = {}

    pattern = re.compile('.* \[(.*)\]')
    with open(file_name) as f:
        for line_number, line in enumerate(f, 1):
            if line.startswith('In file'):
                state = State.INFO
            if line.startswith('openDSME/') or line.startswith('/home/'):
                state = State.WARNING

            if state == State.INFO:
                currentBuffer += line
            elif state == State.WARNING:
                if line.find('warning: ') != -1:
                    match = pattern.match(line)

                    path = line.split(':')[0]
                    currentFile = pathlib.Path(path).resolve()
                    if match:
                        currentWarning = match.group(1)
                    else:
                        print('ERROR1 + ' + str(line_number) + ' ' + line)
                        sys.exit(-1)

                    currentBuffer += line
                    state = State.LINE
                elif line.find('note: ') != -1:
                    currentBuffer += line
                    state = State.INFO
                else:
                    currentBuffer += line
                    state = State.INFO

            elif state == State.LINE:
                currentBuffer += line
                state = State.INFO

                if not currentWarning in warnings:
                    warnings[currentWarning] = []

                warnings[currentWarning].append((str(currentFile), currentBuffer))
                currentBuffer = ''

    print(dict(map(lambda i: (i[0],len(i[1])), warnings.items())))

    output = json.dumps(dict(warnings))
    text_file = open(file_name + '.out', 'w')
    text_file.write(str(output))
    text_file.close()


if __name__ == '__main__':
    main()
