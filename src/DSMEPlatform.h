/*
 * openDSME
 *
 * Implementation of the Deterministic & Synchronous Multi-channel Extension (DSME)
 * introduced in the IEEE 802.15.4e-2012 standard
 *
 * Authors: Florian Meier <florian.meier@tuhh.de>
 *          Maximilian Koestler <maximilian.koestler@tuhh.de>
 *          Sandrina Backhauss <sandrina.backhauss@tuhh.de>
 *
 * Based on
 *          DSME Implementation for the INET Framework
 *          Tobias Luebkert <tobias.luebkert@tuhh.de>
 *
 * Copyright (c) 2015, Institute of Telematics, Hamburg University of Technology
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef DSMEPLATFORM_H
#define DSMEPLATFORM_H

#include <stdint.h>
#include <stdlib.h>

#include <omnetpp.h>

#include <inet/linklayer/base/MacProtocolBase.h>
#include <inet/linklayer/contract/IMacProtocol.h>
#include <inet/physicallayer/contract/packetlevel/IRadio.h>

#include "DSMEMessage.h"
#include "dsme_settings.h"
#include "openDSME/dsmeAdaptionLayer/DSMEAdaptionLayer.h"
#include "openDSME/helper/DSMEDelegate.h"
#include "openDSME/interfaces/IDSMEPlatform.h"
#include "openDSME/mac_services/dataStructures/IEEE802154MacAddress.h"
#include "openDSME/mac_services/mcps_sap/MCPS_SAP.h"
#include "openDSME/mac_services/mlme_sap/MLME_SAP.h"
#include "openDSME/mac_services/pib/MAC_PIB.h"
#include "openDSME/mac_services/pib/PHY_PIB.h"

namespace dsme {

class DSMELayer;
class DSMEAdaptionLayer;

class DSMEPlatform : public inet::MacProtocolBase, public inet::IMacProtocol, public IDSMEPlatform {
    using omnetpp::cIListener::finish;
    using omnetpp::cSimpleModule::send;

public:
    DSMEPlatform();
    virtual ~DSMEPlatform();

    DSMEPlatform(const DSMEPlatform&) = delete;
    DSMEPlatform& operator=(const DSMEPlatform&) = delete;

    /****** INET ******/

    virtual inet::InterfaceEntry* createInterfaceEntry() override;

    /****** OMNeT++ ******/

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int) override;

    /** @brief Delete all dynamically allocated objects of the module*/
    virtual void finish() override;

    /** @brief Handle messages from lower layer */
    virtual void handleLowerPacket(inet::Packet*) override;

    /** @brief Handle messages from upper layer */
    virtual void handleUpperPacket(inet::Packet*) override;

    /** @brief Handle self messages such as timers */
    virtual void handleSelfMessage(omnetpp::cMessage*) override;

    /** @brief Handle control messages from lower layer */
    virtual void receiveSignal(cComponent *source, omnetpp::simsignal_t signalID, long l, cObject *details) override;

    /****** IDSMERadio ******/

    virtual bool setChannelNumber(uint8_t k) override;
    virtual uint8_t getChannelNumber() override;

    virtual bool prepareSendingCopy(IDSMEMessage* msg, Delegate<void(bool)> txEndCallback) override;

    virtual bool sendNow() override;

    virtual void abortPreparedTransmission() override;

    virtual bool sendDelayedAck(IDSMEMessage* ackMsg, IDSMEMessage* receivedMsg, Delegate<void(bool)> txEndCallback) override;

    virtual void setReceiveDelegate(receive_delegate_t receiveDelegate) override;

    virtual bool startCCA() override;

    virtual void turnTransceiverOn() override;

    virtual void turnTransceiverOff() override;

    /****** IDSMEPlatform ******/

    virtual bool isReceptionFromAckLayerPossible() override;

    virtual void handleReceivedMessageFromAckLayer(IDSMEMessage* message) override;

    virtual DSMEMessage* getEmptyMessage() override;

    virtual void releaseMessage(IDSMEMessage* msg) override;

    virtual void startTimer(uint32_t symbolCounterValue) override;

    virtual uint32_t getSymbolCounter() override;

    virtual uint16_t getRandom() override;

    virtual void updateVisual() override;

    virtual void scheduleStartOfCFP() override;

    virtual uint8_t getMinCoordinatorLQI() override;

    virtual void signalGTSChange(bool deallocation, IEEE802154MacAddress counterpart) override;

private:
    DSMEMessage* getLoadedMessage(inet::Packet*);

    void handleIndicationFromMCPS(IDSMEMessage* msg);
    void handleConfirmFromMCPS(IDSMEMessage* msg, DataStatus::Data_Status status);

    bool send(inet::Packet*);

    void signalNewMsg(DSMEMessage* msg);

    std::string getSequenceChartInfo(IDSMEMessage* msg, bool outgoing);
    std::string getDSMEManagement(uint8_t management, DSMESABSpecification& sabSpec, CommandFrameIdentifier cmd);

    PHY_PIB phy_pib;
    MAC_PIB mac_pib;
    DSMELayer* dsme;
    mcps_sap::MCPS_SAP mcps_sap;
    mlme_sap::MLME_SAP mlme_sap;
    DSMEAdaptionLayer dsmeAdaptionLayer;

    uint16_t messagesInUse{0};
    uint16_t msgId{0};
    receive_delegate_t receiveFromAckLayerDelegate{};

    std::map<DSMEMessage*, uint16_t> msgMap{};
    std::set<uint16_t> msgsActive{};

    omnetpp::cMessage* timer{nullptr};
    omnetpp::cMessage* ccaTimer{nullptr};
    omnetpp::cMessage* cfpTimer{nullptr};
    Delegate<void(bool)> txEndCallback{};
    inet::Packet* pendingTxPacket{nullptr};
    bool pendingSendRequest{false};

    /** @brief The radio. */
    inet::physicallayer::IRadio* radio{nullptr};
    inet::physicallayer::IRadio::TransmissionState transmissionState{inet::physicallayer::IRadio::TRANSMISSION_STATE_UNDEFINED};

    /** @brief the bit rate at which we transmit */
    double bitrate{0};

    inet::MacAddress addr{};
    bool channelInactive{true};

    bool transceiverIsOn{false};
    uint8_t minBroadcastLQI{0};
    uint8_t minCoordinatorLQI{0};
    uint8_t currentChannel{0};

public:
    omnetpp::SimTime symbolDuration;

    static omnetpp::simsignal_t broadcastDataSentDown;
    static omnetpp::simsignal_t unicastDataSentDown;
    static omnetpp::simsignal_t ackSentDown;
    static omnetpp::simsignal_t beaconSentDown;
    static omnetpp::simsignal_t commandSentDown;
    static omnetpp::simsignal_t uncorruptedFrameReceived;
    static omnetpp::simsignal_t corruptedFrameReceived;
    static omnetpp::simsignal_t gtsChange;

public:
    IEEE802154MacAddress& getAddress() {
        return this->mac_pib.macExtendedAddress;
    }

private:
    GTSScheduling* scheduling = nullptr;
};

} /* namespace dsme */

#endif /* DSMEPLATFORM_H */
