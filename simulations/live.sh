#!/bin/bash

pushd ../
make || exit 1
popd

CONFIG=DSME
RUN=0
REP=0
VECTOR_RECORDING=false

opp_run -r $RUN --seed-set=$REP --repeat=1 --cmdenv-express-mode=true --vector-recording=$VECTOR_RECORDING -u Cmdenv -c $CONFIG -n .:../src:../../inet/examples:../../inet/src:../../inet/tutorials:.:../src -l ../../inet/src/INET -l ../src/inet-dsme -l ../../inet/src/INET -l ../src/inet-dsme -l ../../inet/src/INET -l ../src/inet-dsme --debug-on-errors=false live.ini

