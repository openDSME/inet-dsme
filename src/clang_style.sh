#!/bin/bash

find . -maxdepth 1 -iname "*.h" -o -iname "*.cc" | xargs clang-format -i
