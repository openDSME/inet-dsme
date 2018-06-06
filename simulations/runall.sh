#!/bin/bash

pushd ../
make -j8 || exit 1
popd

for CONFIG in DSME CSMA
do
    CONFIG=DSME
    REPETITIONS=2

    ./opp_runall -V opp_run --repeat=$REPETITIONS -u Cmdenv -c $CONFIG -n .:../src:../../inet/examples:../../inet/src:../../inet/tutorials:.:../src -l ../../inet/src/INET -l ../src/inet-dsme -l ../../inet/src/INET -l ../src/inet-dsme -l ../../inet/src/INET -l ../src/inet-dsme --debug-on-errors=false --result-dir=results/$CONFIG omnetpp.ini

done
