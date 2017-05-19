#!/bin/bash

if [ -z "$1" ]
then 
	echo "Call it as "
	echo "./build.sh inet-path"
	exit 1
fi

export INET=`readlink -f $1`
echo $INET
pushd src && opp_makemake -f --deep --make-so -I$INET/src -I$INET/src/inet/common -I.. -KINET_PROJ=$INET && popd
make -j2
