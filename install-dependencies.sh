#!/bin/bash
if [ "$CXX" = "g++" ]; then export CXX="g++-4.8" CC="gcc-4.8"; fi

# Download OMNET++
mkdir -p omnetdownload
export OMNETTAR=omnetdownload/omnetpp.tgz
export OMNETURL='https://omnetpp.org/omnetpp/send/30-omnet-releases/2305-omnetpp-50-linux'
export OMNETREF='https://omnetpp.org/omnetpp'
if test -e "$OMNETTAR";
then 
    curl -o "$OMNETTAR" -z "$OMNETTAR" -e $OMNETREF $OMNETURL;
else
    curl -o "$OMNETTAR" -e $OMNETREF $OMNETURL;
fi
    
# Compile OMNET++
tar -xzf $OMNETTAR
export OMNET_PATH=`pwd`/omnetpp-5.0
export PATH=$PATH:$OMNET_PATH/bin
pushd $OMNET_PATH && sed -i 's/WITH_QTENV.*/WITH_QTENV=no/' configure.user && sed -i 's/WITH_TKENV.*/WITH_TKENV=no/' configure.user && ./configure && make -j2 && popd
cp opp_makedep_fix $OMNET_PATH/bin/opp_makedep

# Checkout INET
export INET=`pwd`/inet
if [ ! -d $INET/.git ];
then
    rm -rf $INET;
    git clone --depth=2 --branch=ieee802154_example https://github.com/openDSME/inet.git $INET;
else
    pushd $INET && git pull && popd;
fi

# Compile INET
pushd $INET && make makefiles && make -j2 && popd

