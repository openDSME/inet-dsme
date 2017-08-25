#!/bin/bash

pushd ../
make -j8 || exit 1
popd

CONFIG=DSME-Multi
RUN=6
REP=2
VECTOR_RECORDING=true
#VECTOR_RECORDING=false

opp_run -r $RUN --seed-set=$REP --repeat=1 --cmdenv-express-mode=false --vector-recording=$VECTOR_RECORDING -u Cmdenv -c $CONFIG -n .:../src:../../inet/examples:../../inet/src:../../inet/tutorials:.:../src -l ../../inet/src/INET -l ../src/inet-dsme -l ../../inet/src/INET -l ../src/inet-dsme -l ../../inet/src/INET -l ../src/inet-dsme --debug-on-errors=false omnetpp.ini > mac.log

