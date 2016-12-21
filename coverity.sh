#!/bin/bash

make clean
cov-build --dir cov-int make -j 8
tar czvf coverity-analysis.tgz cov-int > /dev/null 2>&1
rm -r cov-int
