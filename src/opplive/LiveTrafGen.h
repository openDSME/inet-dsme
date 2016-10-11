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

namespace inet {

/**
 * IP traffic generator application for live results
 */
class INET_API LiveTrafGen : public PRRTrafGen
{
  protected:
    simtime_t intermediatePRRInterval;
    double intermediatePRRAlpha;
    static simsignal_t intermediatePRRSignal;
    static cMessage *intermediatePRRTimer;
    static int sentCurrentInterval;
    static int receivedCurrentInterval;
    static int droppedCurrentInterval;
    static double sentPerIntervalSmooth;
    static double receivedPerIntervalSmooth;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void sendPacket() override;

    virtual void receiveSignal(cComponent *prev, simsignal_t t, cObject *obj DETAILS_ARG) override;

    void messageDeliveredOrDropped(cPacket* pkt, bool dropped);

  public:
    LiveTrafGen();
    virtual ~LiveTrafGen();

    void handleDroppedPacket(cPacket *msg);
};

} // namespace inet

#endif // ifndef __INET_PRRTRAFGEN_H

