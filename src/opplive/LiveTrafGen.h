//
// Copyright (C) 2016 Florian Kauer <florian.kauer@koalo.de>
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

#ifndef __INET_LIVETRAFGEN_H
#define __INET_LIVETRAFGEN_H

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/L3Address.h"
#include "inet/applications/generic/PRRTrafGen.h"
#include "wamp_cpp/RPCallable.h"

namespace inet {

enum PacketResult {
    DELIVERED = 0,
    NO_ACK = 1,
    CHANNEL_BUSY = 2,
    NO_ROUTE = 3,
    QUEUE_FULL = 4,
    NO_GTS = 5,
    PACKET_RESULT_LENGTH = 6
};

/**
 * IP traffic generator application for live results
 */
class INET_API LiveTrafGen : public PRRTrafGen,  public RPCallable<LiveTrafGen>, public cVisitor
{
  protected:
    simtime_t intermediatePRRInterval;
    double intermediatePRRAlpha;

    static simsignal_t intermediatePowerMeasurement;
    static simsignal_t intermediatePRRSignal;
    static simsignal_t nodeDroppedPk;

    static cMessage *intermediatePRRTimer;
    static int sentCurrentInterval;
    //static int receivedCurrentInterval;
    //static int droppedCurrentInterval;
    //static double sentPerIntervalSmooth;
    //static double receivedPerIntervalSmooth;

    double meanInterval;
    double k;

    static std::vector<std::vector<std::array<unsigned int,PACKET_RESULT_LENGTH>>> droppedHistory;
    static unsigned int history_index;
    static constexpr unsigned int history_length = 5;

  protected:
    virtual void initialize(int stage) override;
    virtual void scheduleNextPacket(simtime_t previous) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void sendPacket() override;

    virtual void receiveSignal(cComponent *prev, simsignal_t t, cObject *obj DETAILS_ARG) override;

    void messageDeliveredOrDropped(cPacket* pkt, PacketResult result);

    virtual void visit(cObject *obj) override;

  public:
    LiveTrafGen();
    virtual ~LiveTrafGen();

    void handleDroppedPacket(cPacket *msg, uint16_t srcAddr, PacketResult result);
    void setInterval(double interval);
    void scheduleStop();

  private:
    void setDroppedZero();

    static unsigned int handledPackets[PACKET_RESULT_LENGTH];

    bool waitForStop;
    int startingCountdown;
};

} // namespace inet

#endif // ifndef __INET_PRRTRAFGEN_H

