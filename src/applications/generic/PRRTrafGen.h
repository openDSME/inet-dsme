//
// Copyright (C) 2016 Florian Kauer <florian.kauer@koalo.de>
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_DSME_PRRTRAFGEN_H
#define __INET_DSME_PRRTRAFGEN_H

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/L3Address.h"
#include "inet/applications/generic/IpvxTrafGen.h"

namespace inet_dsme {

/**
 * IP traffic generator application for measuring PRR.
 */
class PRRTrafGen : public inet::IpvxTrafGen, public omnetpp::cIListener
{
  protected:
    // statistic
    static omnetpp::simsignal_t sinkRcvdPkSignal;
    static omnetpp::simsignal_t sourceSentPkSignal;
    static omnetpp::simsignal_t sentDummyPkSignal;
    static omnetpp::simsignal_t pktRcvdAfterWarmupSignal;
    std::map<inet::L3Address, omnetpp::simsignal_t> rcvdPkFromSignals;
    std::map<inet::L3Address, omnetpp::simsignal_t> sentPkToSignals;

    static int initializedCount;
    static int finishedCount;
    bool finished = false;

    omnetpp::simtime_t warmUpDuration;
    omnetpp::simtime_t coolDownDuration;
    bool continueSendingDummyPackets;
    omnetpp::cMessage *shutdownTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(inet::Packet *msg) override;
    virtual void sendPacket() override;
    virtual bool isEnabled() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;

    virtual void receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, bool b, omnetpp::cObject *details) override {}
    virtual void receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, long l, omnetpp::cObject *details) override {}
    virtual void receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, unsigned long l, omnetpp::cObject *details) override {}
    virtual void receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, double d, omnetpp::cObject *details) override {}
    virtual void receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, const omnetpp::SimTime& t, omnetpp::cObject *details) override {}
    virtual void receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, const char *s, omnetpp::cObject *details) override {}
    virtual void receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, omnetpp::cObject *obj, omnetpp::cObject *details) override;

    std::vector<bool> packetReceived;
    std::map<inet::L3Address,std::vector<bool>> packetReceivedFrom;

  public:
    PRRTrafGen();
    virtual ~PRRTrafGen();

private:
    std::string extractHostName(const std::string& prefix, const std::string& sourceName);
    void addRecorderAndEmit(const std::string& signalPrefix, std::map<inet::L3Address, omnetpp::simsignal_t>& signals, inet::L3Address& address, inet::Packet *msg);
};

} /* namespace inet_dsme */

#endif /* __INET_DSME_PRRTRAFGEN_H */
