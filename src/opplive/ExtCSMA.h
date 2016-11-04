#ifndef __INET_EXT_CSMA_H
#define __INET_EXT_CSMA_H

#include "inet/linklayer/csma/CSMA.h"
#include "../LiveTrafGen.h"

namespace inet {

class INET_API ExtCSMA : public CSMA
{
    virtual void packetDropped(CSMAFrame* macPkt, PacketResult result);
    virtual void manageMissingAck(t_mac_event event, cMessage *msg);
    virtual void updateStatusCCA(t_mac_event event, cMessage *msg);
    virtual void updateStatusIdle(t_mac_event event, cMessage *msg);
};

} // namespace inet

#endif
