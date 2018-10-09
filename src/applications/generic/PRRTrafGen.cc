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

// fire assert also in release mode
#undef NDEBUG

#include "PRRTrafGen.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IL3AddressType.h"

namespace inet_dsme {

Define_Module(PRRTrafGen);

omnetpp::simsignal_t PRRTrafGen::sinkRcvdPkSignal = registerSignal("sinkRcvdPk");
omnetpp::simsignal_t PRRTrafGen::sourceSentPkSignal = registerSignal("sourceSentPk");
omnetpp::simsignal_t PRRTrafGen::sentDummyPkSignal = registerSignal("sentDummyPk");
omnetpp::simsignal_t PRRTrafGen::pktRcvdAfterWarmupSignal = registerSignal("pktRcvdAfterWarmup");
int PRRTrafGen::initializedCount = 0;
int PRRTrafGen::finishedCount = 0;

const std::string rcvdPkFrom = "rcvdPkFrom";
const std::string sentPkTo = "sentPkTo";

PRRTrafGen::PRRTrafGen()
{
}

PRRTrafGen::~PRRTrafGen()
{
    cancelAndDelete(shutdownTimer);
}

void PRRTrafGen::initialize(int stage)
{
    IpvxTrafGen::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL) {
        initializedCount++;
        warmUpDuration = par("warmUpDuration");
        coolDownDuration = par("coolDownDuration");
        continueSendingDummyPackets = par("continueSendingDummyPackets");

        // subscribe to signals
        std::string signalName = extractHostName(rcvdPkFrom,this->getFullPath());
        getSimulation()->getSystemModule()->subscribe(signalName.c_str(), this);
        signalName = extractHostName(sentPkTo,this->getFullPath());
        getSimulation()->getSystemModule()->subscribe(signalName.c_str(), this);
    }
    else if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
        shutdownTimer = new omnetpp::cMessage("shutdownTimer");
    }
}

void PRRTrafGen::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, omnetpp::cObject *obj, omnetpp::cObject *details) {
    inet::Packet* pkt = (inet::Packet*)obj;
    auto signalName = getSignalName(signalID);

    if(strncmp(signalName, rcvdPkFrom.c_str(), rcvdPkFrom.size()) == 0) {
        unsigned int num = atoi(pkt->getName()+strlen("appData-"));
        if(packetReceived.size() < num+1) {
            packetReceived.resize(num+1,false);
        }

        assert(!packetReceived[num]); // duplicates are already filtered in processPacket
        packetReceived[num] = true;
        emit(sinkRcvdPkSignal, obj);
    }
    else if(strncmp(signalName, sentPkTo.c_str(), sentPkTo.size()) == 0) {
        emit(sourceSentPkSignal, obj);
    }
    else {
        assert(false);
    }
}

bool PRRTrafGen::isEnabled()
{
    if(!finished && numPackets != -1 && numSent >= numPackets) {
        finished = true;
        finishedCount++;

        if(finishedCount >= initializedCount) {
            scheduleAt(omnetpp::simTime() + coolDownDuration, shutdownTimer);
        }
    }

    return (numPackets == -1 || numSent < numPackets || continueSendingDummyPackets);
}

void PRRTrafGen::handleMessage(omnetpp::cMessage *msg)
{
    const char *destAddrs = par("destAddresses");
    if(msg == shutdownTimer) {
        recordScalar("runtimeSinceWarmup",omnetpp::simTime()-startTime-warmUpDuration);
        endSimulation();
    }
    else if (msg == timer && msg->getKind() == START
            && strcmp(destAddrs,"*") == 0) {
        destAddresses.clear();
        inet::L3Address myAddress = inet::L3AddressResolver().addressOf(this->getParentModule(), inet::L3AddressResolver::ADDR_MODULEPATH);
        cModule *sys = getSimulation()->getSystemModule();
        for(cModule::SubmoduleIterator i(sys); !i.end(); i++) {
            cModule *submod = *i;
            if(submod->isVector()) {
                inet::L3Address result = inet::L3AddressResolver().addressOf(submod, inet::L3AddressResolver::ADDR_MODULEPATH);
                if(result == myAddress) {
                    continue;
                }
                if(result.isUnspecified()) {
                    EV_ERROR << "cannot resolve destination address for module " << submod->getFullPath() << inet::endl;
                }
                else {
                    destAddresses.push_back(result);
                }
            }
        }
        if (!destAddresses.empty()) {
            sendPacket();
            if (isEnabled())
                scheduleNextPacket(inet::simTime());
        }
    }
    else {
        IpvxTrafGen::handleMessage(msg);
    }
}


void PRRTrafGen::sendPacket()
{
    char msgName[32];
    sprintf(msgName, "appData-%d", numSent);

    auto packet = new inet::Packet(msgName);
    const auto& payload = inet::makeShared<inet::ByteCountChunk>(inet::B(*packetLengthPar));
    auto now = omnetpp::simTime();
    bool dummy = now < startTime+warmUpDuration || (numPackets != -1 && numSent >= numPackets);
    packet->addPar("dummy") = dummy;
    packet->insertAtBack(payload);

    auto destAddr = chooseDestAddr();

    auto addressType = destAddr.getAddressType();
    packet->addTagIfAbsent<inet::PacketProtocolTag>()->setProtocol(protocol);
    packet->addTagIfAbsent<inet::DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());
    packet->addTagIfAbsent<inet::L3AddressReq>()->setDestAddress(destAddr);

    if(!dummy) {
        EV_INFO << "Sending packet: ";
        printPacket(packet);
        addRecorderAndEmit(sentPkTo,sentPkToSignals,destAddr,packet);
        emit(inet::packetSentSignal, packet);
        send(packet, "ipOut");
    }
    else {
        EV_INFO << "Sending dummy packet: ";
        printPacket(packet);
        emit(sentDummyPkSignal, packet);
        send(packet, "ipOut");
    }

    if(now >= startTime+warmUpDuration) {
        numSent++;
    }
}

std::string PRRTrafGen::extractHostName(const std::string& signalPrefix, const std::string& sourceName) {
    std::string signalName = "";
    std::size_t hostStart = sourceName.find("host[");
    assert(hostStart != std::string::npos);
    std::size_t hostEnd = sourceName.find("]", hostStart);
    assert(hostEnd != std::string::npos);
    std::stringstream s;
    s << signalPrefix << "-host[" << sourceName.substr(hostStart + 5, hostEnd - hostStart - 5) << "]";
    signalName = s.str();
    return signalName;
}

void PRRTrafGen::addRecorderAndEmit(const std::string& signalPrefix, std::map<inet::L3Address, omnetpp::simsignal_t>& signals, inet::L3Address& address, inet::Packet *msg) {
    auto it = signals.find(address);
    if(it == signals.end()) {
        std::string signalName = extractHostName(signalPrefix,address.str());

        auto signal = registerSignal(signalName.c_str());

        omnetpp::cProperty *statisticTemplate = getProperties()->get("statisticTemplate", signalPrefix.c_str());
        getSimulation()->getActiveEnvir()->addResultRecorders(this, signal, signalName.c_str(), statisticTemplate);

        signals[address] = signal;
        it = signals.find(address);
    }

    emit(it->second, msg);
}

void PRRTrafGen::processPacket(inet::Packet *msg)
{
    auto tag = msg->getTag<inet::L3AddressInd>();
    auto sourceAddress = tag->getSrcAddress();
    auto destinationAddress = tag->getDestAddress();

    if(sourceAddress == destinationAddress) {
        // filter locally delivered packets
        return;
    }

    if(!packetReceivedFrom.count(sourceAddress)) {
        packetReceivedFrom[sourceAddress] = std::vector<bool>();
    }

    unsigned int num = atoi(msg->getName()+strlen("appData-"));
    if(packetReceivedFrom[sourceAddress].size() < num+1) {
        packetReceivedFrom[sourceAddress].resize(num+1,false);
    }

    if(packetReceivedFrom[sourceAddress][num]) {
        delete msg;
        return;
    }

    // signal packet receptions for throughput calculation
    auto now = omnetpp::simTime();
    if(now >= startTime+warmUpDuration) {
        emit(pktRcvdAfterWarmupSignal, msg);
    }

    // Throw away dummy packets
    if(msg->par("dummy")) {
        delete msg;
        return;
    }

    packetReceivedFrom[sourceAddress][num] = true;

    addRecorderAndEmit(rcvdPkFrom,rcvdPkFromSignals,sourceAddress,msg);

    IpvxTrafGen::processPacket(msg);
}

} /* namespace inet_dsme */

