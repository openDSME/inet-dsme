#!/bin/bash
export INET=$1
export RUN=5
export REP=0

function run {
    opp_run -r $RUN --seed-set=$REP --repeat=1 --cmdenv-express-mode=false --vector-recording=true -u Cmdenv -c $1 -n simulations:src:$INET/examples:$INET/src -l $INET/src/INET -l src/inet-dsme --debug-on-errors=false simulations/omnetpp.ini > $1-mac.log
}

run DSME &
run CSMA &
wait


