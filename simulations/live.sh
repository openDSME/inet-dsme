#!/bin/bash

pushd ../
make || exit 1
popd

CONFIG1=DSME
CONFIG2=CSMA
RUN=0
REP=0
VECTOR_RECORDING=false

echo "while :; do opp_run -r $RUN --seed-set=$REP --repeat=1 --cmdenv-express-mode=true --vector-recording=$VECTOR_RECORDING -u Cmdenv -c $CONFIG1 -n\
    .:../src:../../inet/examples:../../inet/src:../../inet/tutorials:.:../src\
    -l ../../inet/src/INET\
    -l ../src/inet-dsme\
    -l ../../inet/src/INET\
    -l ../src/inet-dsme\
    -l ../../inet/src/INET\
    -l ../src/inet-dsme\
    --debug-on-errors=false live.ini; done" >> /tmp/dsmerun.sh
bash /tmp/dsmerun.sh & pid=$!
PID_LIST+=" $pid";

echo "while :; do opp_run -r $RUN --seed-set=$REP --repeat=1 --cmdenv-express-mode=true --vector-recording=$VECTOR_RECORDING -u Cmdenv -c $CONFIG2 -n\
    .:../src:../../inet/examples:../../inet/src:../../inet/tutorials:.:../src\
    -l ../../inet/src/INET\
    -l ../src/inet-dsme\
    -l ../../inet/src/INET\
    -l ../src/inet-dsme\
    -l ../../inet/src/INET\
    -l ../src/inet-dsme\
    --debug-on-errors=false live.ini; done" >> /tmp/csmarun.sh
bash /tmp/csmarun.sh & pid=$!
PID_LIST+=" $pid";

trap "kill $PID_LIST" SIGINT

echo "Parallel processes have started";

wait $PID_LIST

echo
echo "All processes have completed";
