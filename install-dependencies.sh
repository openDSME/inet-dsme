#!/bin/bash
if [ "$CXX" = "g++" ]; then export CXX="g++-4.8" CC="gcc-4.8"; fi

# Download OMNET++
mkdir -p omnetdownload
export OMNETTAR=omnetdownload/omnetpp-5.3-src-linux.tgz
#export OMNETURL='https://omnetpp.org/component/jdownloads/send/31-release-test-versions/2325-omnetpp-5-3p4-linux'
#export OMNETREF='https://omnetpp.org/component/jdownloads/category/31-release-test-versions'
#if test -e "$OMNETTAR";
#then 
#    echo curl -o "$OMNETTAR" -z "$OMNETTAR" -e $OMNETREF $OMNETURL;
#    curl -o "$OMNETTAR" -z "$OMNETTAR" -e $OMNETREF $OMNETURL;
#else
#    echo curl -o "$OMNETTAR" -e $OMNETREF $OMNETURL;
#    curl -o "$OMNETTAR" -e $OMNETREF $OMNETURL;
#fi

screen -dmS rtorrent rtorrent -d omnetdownload utils/omnetpp-5.3-src-linux.tgz.torrent

while :
do
    if md5sum -c utils/omnet.md5; then
	break
    fi
    sleep 5
done
    
# Compile OMNET++
echo $OMNETTAR
ls
tar -xzf $OMNETTAR
ls
export OMNET_PATH=`pwd`/omnetpp-5.3
export PATH=$PATH:$OMNET_PATH/bin
pushd $OMNET_PATH && sed -i 's/WITH_QTENV.*/WITH_QTENV=no/' configure.user && sed -i 's/WITH_TKENV.*/WITH_TKENV=no/' configure.user && sed -i 's/WITH_OSG.*/WITH_OSG=no/' configure.user && ./configure && make -j2 && popd
#cp utils/opp_makedep_fix $OMNET_PATH/bin/opp_makedep
ls $OMNET_PATH
rm $OMNET_PATH/config.log # unnecessarily forces repacking the cache
ls $OMNET_PATH

# Checkout INET
export INET=`pwd`/inet
#export BRANCH=integration
export BRANCH=ieee802154_example
if [ ! -d $INET/.git ];
then
    rm -rf $INET;
    git clone --depth=2 --branch=$BRANCH https://github.com/openDSME/inet.git $INET;
else
    pushd $INET && git fetch -f origin $BRANCH && git reset --hard FETCH_HEAD && popd;
fi

# Compile INET
pushd $INET && make makefiles && make -j2 && popd

