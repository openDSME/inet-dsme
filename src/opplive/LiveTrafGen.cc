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

#include "LiveTrafGen.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"

namespace inet {

Define_Module(LiveTrafGen);

simsignal_t LiveTrafGen::intermediatePRRSignal = registerSignal("intermediatePRR");
int LiveTrafGen::receivedCurrentInterval = 0;
int LiveTrafGen::sentCurrentInterval = 0;
int LiveTrafGen::droppedCurrentInterval = 0;
double LiveTrafGen::receivedPerIntervalSmooth = 0;
double LiveTrafGen::sentPerIntervalSmooth = 0;
cMessage *LiveTrafGen::intermediatePRRTimer = 0;

double lastV = 0;
double lastMean = 0;
int cnt = 0;

LiveTrafGen::LiveTrafGen()
{
}

LiveTrafGen::~LiveTrafGen()
{
    if(intermediatePRRTimer != nullptr) {
        cancelAndDelete(intermediatePRRTimer);
        intermediatePRRTimer = nullptr;
    }
}

void LiveTrafGen::setInterval(double interval) {
    meanInterval = interval;
}

void LiveTrafGen::initialize(int stage)
{
    PRRTrafGen::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        intermediatePRRInterval = par("intermediatePRRInterval");
        intermediatePRRAlpha = par("intermediatePRRAlpha");
        meanInterval = 1;
        k = 10;

        addRemoteProcedure("http://example.com/simple/setInterval",&LiveTrafGen::setInterval);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        if(intermediatePRRTimer == nullptr) { // only one node shall handle these events
            intermediatePRRTimer = new cMessage("intermediatePRRTimer");
            scheduleAt(simTime()+intermediatePRRInterval, intermediatePRRTimer);
        }
    }
}

void LiveTrafGen::scheduleNextPacket(simtime_t previous)
{
    simtime_t next;
    if (previous == -1) {
        next = simTime() <= startTime ? startTime : simTime();
        timer->setKind(START);
    }
    else {
        next = previous + gamma_d(k, meanInterval/k);
        timer->setKind(NEXT);
    }
    if (stopTime < SIMTIME_ZERO || next < stopTime)
        scheduleAt(next, timer);
}

void LiveTrafGen::messageDeliveredOrDropped(cPacket* pkt, bool dropped) {
    unsigned int num = atoi(pkt->getName()+strlen("appData-"));

    // packetReceived from the base class is also used to track dropped packet

    if(packetReceived.size() < num+1) {
        packetReceived.resize(num+1,false);
    }

    if(!packetReceived[num]) { // handle duplicates
        packetReceived[num] = true;
        if(!dropped) {
            receivedCurrentInterval++;
            emit(sinkRcvdPkSignal, pkt);
        }
        else {
            droppedCurrentInterval++;
        }
    }
}

void LiveTrafGen::receiveSignal(cComponent *prev, simsignal_t t, cObject *obj DETAILS_ARG) {
    messageDeliveredOrDropped((cPacket*)obj,false);
}

void LiveTrafGen::handleDroppedPacket(cPacket *msg) {
    messageDeliveredOrDropped(msg,true);
    delete msg;
}

void LiveTrafGen::handleMessage(cMessage *msg)
{
    if(msg == intermediatePRRTimer) {
        cnt++;
        double val = sentCurrentInterval;
        double mean = lastMean + (val - lastMean) / cnt;
        double v = lastV + (val - lastMean)*(val - mean);
        double var = v / (cnt-1);
        sentPerIntervalSmooth = intermediatePRRAlpha * sentCurrentInterval + (1-intermediatePRRAlpha) * sentPerIntervalSmooth;
        receivedPerIntervalSmooth = intermediatePRRAlpha * receivedCurrentInterval + (1-intermediatePRRAlpha) * receivedPerIntervalSmooth;
        std::cout.precision(3);
        std::cout << std::fixed << var << " " << mean << " " << val << std::endl;
        //droppedCurrentInterval << " " << receivedCurrentInterval << " " << sentCurrentInterval << " " << receivedCurrentInterval/(double)sentCurrentInterval << " " <<  receivedPerIntervalSmooth << " " << sentPerIntervalSmooth << " " << receivedPerIntervalSmooth / sentPerIntervalSmooth << std::endl;
        std::stringstream stream;
        stream << receivedCurrentInterval << "," << droppedCurrentInterval;
        emit(intermediatePRRSignal, stream.str().c_str());
        lastMean = mean;
        lastV = v;
        receivedCurrentInterval = 0;
        sentCurrentInterval = 0;
        droppedCurrentInterval = 0;
        scheduleAt(simTime()+intermediatePRRInterval, intermediatePRRTimer);
    }
    else {
        PRRTrafGen::handleMessage(msg);
    }
}

void LiveTrafGen::sendPacket() {
    sentCurrentInterval++;
    PRRTrafGen::sendPacket();
}

} // namespace inet

