#!/bin/bash
export INET=$1
export RUN=0
export REP=0
export CONSTRAINT="\$sendInt>=2"

mkdir -p results

function run {
    opp_run -r $RUN --constraint="$CONSTRAINT" --seed-set=$REP --repeat=1 --**.mac.cmdenv-log-level=info --cmdenv-express-mode=false --cmdenv-status-frequency=6s --result-dir=../results --vector-recording=true -u Cmdenv -c $1 -n simulations:src:$INET/examples:$INET/src -l $INET/src/INET -l src/inet-dsme --debug-on-errors=false simulations/evaluation.ini | tee results/$1-mac.log | awk 'NR < 20 || NR % 100000 == 0'
}

run DSME &
run CSMA &
wait

