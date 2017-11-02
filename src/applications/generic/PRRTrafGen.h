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
#include "inet/applications/generic/IPvXTrafGen.h"

namespace inet_dsme {

/**
 * IP traffic generator application for measuring PRR.
 */
class PRRTrafGen : public inet::IPvXTrafGen, public cIListener
{
  protected:
    // statistic
    static simsignal_t sinkRcvdPkSignal;
    static simsignal_t sentDummyPkSignal;
    std::map<inet::L3Address, simsignal_t> rcvdPkFromSignals;

    static int initializedCount;
    static int finishedCount;
    bool finished = false;

    simtime_t warmUpDuration;
    simtime_t coolDownDuration;
    bool continueSendingDummyPackets;
    cMessage *shutdownTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(cPacket *msg) override;
    virtual void sendPacket() override;
    virtual bool isEnabled() override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, bool b, cObject *details) override {}
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, long l, cObject *details) override {}
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, unsigned long l, cObject *details) override {}
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, double d, cObject *details) override {}
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, const SimTime& t, cObject *details) override {}
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, const char *s, cObject *details) override {}
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    std::vector<bool> packetReceived;

  public:
    PRRTrafGen();
    virtual ~PRRTrafGen();

private:
    std::string extractHostName(const std::string& sourceName);
};

} /* namespace inet_dsme */

#endif /* __INET_DSME_PRRTRAFGEN_H */
