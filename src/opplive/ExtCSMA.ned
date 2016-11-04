package inet_dsme.opplive;

import inet.linklayer.ieee802154.Ieee802154NarrowbandMac;

simple ExtCSMA extends Ieee802154NarrowbandMac
{
    parameters:
        @class(inet::ExtCSMA);
        @signal[NF_LINK_BREAK](type=long); // not sent anyway
}
