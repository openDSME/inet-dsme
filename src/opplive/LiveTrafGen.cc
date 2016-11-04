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

#include "inet/linklayer/base/MACFrameBase_m.h"

#include "LiveRecorder.h"

namespace inet {

Define_Module(LiveTrafGen);

simsignal_t LiveTrafGen::intermediatePRRSignal = registerSignal("intermediatePRR");
simsignal_t LiveTrafGen::nodeDroppedPk = registerSignal("nodeDroppedPk");

unsigned int LiveTrafGen::handledPackets[PACKET_RESULT_LENGTH];

//int LiveTrafGen::receivedCurrentInterval = 0;
int LiveTrafGen::sentCurrentInterval = 0;
//int LiveTrafGen::droppedCurrentInterval = 0;
//double LiveTrafGen::receivedPerIntervalSmooth = 0;
//double LiveTrafGen::sentPerIntervalSmooth = 0;
cMessage *LiveTrafGen::intermediatePRRTimer = 0;

std::vector<std::vector<unsigned int>> LiveTrafGen::droppedHistory;
unsigned int LiveTrafGen::history_index = 0;

//double lastV = 0;
//double lastMean = 0;
//int cnt = 0;

LiveTrafGen::LiveTrafGen()
{
    droppedHistory.resize(history_length);
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
        unsigned int portNumber = par("portNumber");
        LiveRecorder::openConnection(portNumber);

        intermediatePRRInterval = par("intermediatePRRInterval");
        intermediatePRRAlpha = par("intermediatePRRAlpha");
        meanInterval = 1;
        k = 10;

        addRemoteProcedure("http://opendsme.org/rpc/setInterval",&LiveTrafGen::setInterval);
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

void LiveTrafGen::messageDeliveredOrDropped(cPacket* pkt, PacketResult result) {
    unsigned int num = atoi(pkt->getName()+strlen("appData-"));

    // packetReceived from the base class is also used to track dropped packet

    if(packetReceived.size() < num+1) {
        packetReceived.resize(num+1,false);
    }

    if(!packetReceived[num]) { // handle duplicates
        packetReceived[num] = true;
        if(result == PacketResult::DELIVERED) {
            emit(sinkRcvdPkSignal, pkt);
        }

        handledPackets[result]++;
    }
}

void LiveTrafGen::receiveSignal(cComponent *prev, simsignal_t t, cObject *obj DETAILS_ARG) {
    messageDeliveredOrDropped((cPacket*)obj,PacketResult::DELIVERED);
}

void LiveTrafGen::handleDroppedPacket(cPacket *msg, uint16_t srcAddr, PacketResult result) {
    messageDeliveredOrDropped(msg,result);
    ASSERT(srcAddr > 0);

    unsigned int index = srcAddr - 1;

    if(droppedHistory[history_index].size() <= index) {
        droppedHistory[history_index].resize(index + 1);
    }
    droppedHistory[history_index][index]++;
}

void LiveTrafGen::handleMessage(cMessage *msg)
{
    if(msg == intermediatePRRTimer) {
        //cnt++;
        //double val = sentCurrentInterval;
        //double mean = lastMean + (val - lastMean) / cnt;
        //double v = lastV + (val - lastMean)*(val - mean);
        //double var = v / (cnt-1);
        //sentPerIntervalSmooth = intermediatePRRAlpha * sentCurrentInterval + (1-intermediatePRRAlpha) * sentPerIntervalSmooth;
        //receivedPerIntervalSmooth = intermediatePRRAlpha * receivedCurrentInterval + (1-intermediatePRRAlpha) * receivedPerIntervalSmooth;
        //std::cout.precision(3);
        //std::cout << std::fixed << var << " " << mean << " " << val << std::endl;
        //droppedCurrentInterval << " " << receivedCurrentInterval << " " << sentCurrentInterval << " " << receivedCurrentInterval/(double)sentCurrentInterval << " " <<  receivedPerIntervalSmooth << " " << sentPerIntervalSmooth << " " << receivedPerIntervalSmooth / sentPerIntervalSmooth << std::endl;
        std::stringstream stream;
        stream << simTime().dbl();
        for(int i = 0; i < PACKET_RESULT_LENGTH; i++) {
            stream << "," << handledPackets[i];
            handledPackets[i] = 0;
        }

        emit(intermediatePRRSignal, stream.str().c_str());
        //lastMean = mean;
        //lastV = v;
        sentCurrentInterval = 0;
        scheduleAt(simTime()+intermediatePRRInterval, intermediatePRRTimer);


        unsigned int maxSize = 0;
        for(std::vector<unsigned int>& vec : droppedHistory) {
            if(vec.size() > maxSize) {
                maxSize = vec.size();
            }
        }

        std::vector<unsigned int> droppedLastInterval;
        droppedLastInterval.resize(maxSize);

        for(std::vector<unsigned int>& vec : droppedHistory) {
            for(unsigned int i = 0; i < vec.size(); i++) {
                droppedLastInterval[i] += vec[i];
            }
        }

        std::stringstream droppedStream;
        Json::FastWriter writer;
        Json::Value results(Json::arrayValue);
        for(unsigned int &value : droppedLastInterval) {
            double mean = static_cast<double>(value) / history_length;
            results.append(Json::Value(mean));
        }

        droppedStream << writer.write(results);
        emit(nodeDroppedPk, droppedStream.str().c_str());

        history_index = (history_index + 1) % history_length;
        setDroppedZero();
    }
    else {
        PRRTrafGen::handleMessage(msg);
    }
}

void LiveTrafGen::sendPacket() {
    sentCurrentInterval++;
    PRRTrafGen::sendPacket();
}

void LiveTrafGen::setDroppedZero() {
    for(unsigned int& val : droppedHistory[history_index]) {
        val = 0;
    }
}

} // namespace inet
