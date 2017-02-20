#!/bin/bash

make clean
cov-build --dir cov-int make -j 8
tar czvf coverity-analysis.tgz cov-int > /dev/null 2>&1
rm -r cov-int

curl --form token=IKVzYx93kdr3ygL9mnXU9g \
    --form email=maxkoestler@yahoo.de \
    --form file=@coverity-analysis.tgz \
    https://scan.coverity.com/builds?project=openDSME%2Finet-dsme

rm coverity-analysis.tgz
