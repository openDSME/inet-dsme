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

#include "omnetpp.h"
#include "inet/linklayer/base/MACProtocolBase.h"
#include "inet/linklayer/contract/IMACProtocol.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

#include "DSMEFrame_m.h"
#include "DSMEMessage.h"
#include "openDSME/dsmeAdaptionLayer/DSMEAdaptionLayer.h"
#include "openDSME/helper/DSMEDelegate.h"
#include "openDSME/interfaces/IDSMEPlatform.h"
#include "openDSME/mac_services/dataStructures/IEEE802154MacAddress.h"
#include "openDSME/mac_services/mcps_sap/MCPS_SAP.h"
#include "openDSME/mac_services/mlme_sap/MLME_SAP.h"
#include "openDSME/mac_services/pib/MAC_PIB.h"
#include "openDSME/mac_services/pib/PHY_PIB.h"
#include "dsme_settings.h"

namespace dsme {

class DSMESettings;
class DSMELayer;
class DSMEAdaptionLayer;

class DSMEPlatform : public inet::MACProtocolBase, public inet::IMACProtocol, public IDSMEPlatform {
public:
    typedef Delegate<void(DSMEMessage* msg)> receive_delegate_t;

    DSMEPlatform();
    virtual ~DSMEPlatform();

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int) override;

    /** @brief Delete all dynamically allocated objects of the module*/
    virtual void finish() override;

    /** @brief Handle messages from lower layer */
    virtual void handleLowerPacket(cPacket *) override;

    /** @brief Handle messages from upper layer */
    virtual void handleUpperPacket(cPacket *) override;

    /** @brief Handle self messages such as timers */
    virtual void handleSelfMessage(cMessage *) override;

    /** @brief Handle control messages from lower layer */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, long value DETAILS_ARG) override;

    inet::InterfaceEntry *createInterfaceEntry() override;

    /* IDSMEPlatform */

    /**
     * Directly send packet without delay and without CSMA
     * but keep the message (the caller has to ensure that the message is eventually released)
     * This might lead to an additional memory copy in the platform
     */
    bool sendCopyNow(DSMEMessage *msg, Delegate<void(bool)> txEndCallback);

    /**
     * Send an ACK message, delay until aTurnaRoundTime after reception_time has expired
     */
    bool sendDelayedAck(DSMEMessage *ackMsg, DSMEMessage *receivedMsg, Delegate<void(bool)> txEndCallback);

    void handleReceivedMessageFromAckLayer(DSMEMessage* message) override;

    void setReceiveDelegate(receive_delegate_t receiveDelegate) override;

    void updateVisual() override;

    void startCCA() override {
        channelInactive = true;
        scheduleAt(simTime() + 8*symbolDuration, ccaTimer);
    }

    void startTimer(uint32_t symbolCounterValue) override {
        SimTime time = symbolCounterValue*symbolDuration;
        if(timer->isScheduled()) {
            cancelEvent(timer);
        }
        scheduleAt(time, timer);
    }

    uint32_t getSymbolCounter() override {
        return simTime()/symbolDuration;
    }

    bool setChannelNumber(uint8_t k) override;

    // TODO handle error case
    DSMEMessage* getEmptyMessage() override;

    void releaseMessage(DSMEMessage* msg) override;

    virtual void scheduleStartOfCFP();

private:
    DSMEMessage* getLoadedMessage(DSMEFrame* frame);

    void handleIndicationFromMCPS(DSMEMessage* msg);
    void handleConfirmFromMCPS(DSMEMessage* msg, bool success);

    bool send(DSMEFrame* frame);

private:
    PHY_PIB phy_pib;
    MAC_PIB mac_pib;

    DSMELayer* dsme;

    mcps_sap::MCPS_SAP mcps_sap;
    mlme_sap::MLME_SAP mlme_sap;

    DSMEAdaptionLayer dsmeAdaptionLayer;

    uint16_t messagesInUse;

    receive_delegate_t receiveFromAckLayerDelegate;

public:
    SimTime symbolDuration;

    static simsignal_t broadcastDataSentDown;
    static simsignal_t unicastDataSentDown;
    static simsignal_t ackSentDown;
    static simsignal_t beaconSentDown;
    static simsignal_t commandSentDown;
    static simsignal_t uncorruptedFrameReceived;
    static simsignal_t corruptedFrameReceived;

private:
    void signalNewMsg(DSMEMessage* msg);

    uint16_t msgId;
    std::map<DSMEMessage*, uint16_t> msgMap;
    std::set<uint16_t> msgsActive;

    cMessage* timer;
    cMessage* ccaTimer;
    cMessage* cfpTimer;
    Delegate<void(bool)> txEndCallback;

public:

    static void setSeed(uint16_t seed) {
        srand(seed);
    }

    virtual uint16_t getRandom() override {
        return intrand(UINT16_MAX);
    }

    IEEE802154MacAddress& getAddress() {
        return this->mac_pib.macExtendedAddress;
    }

private:
    DSMEFrame* pendingTxFrame;

    DSMESettings* settings;

    /** @brief The radio. */
    inet::physicallayer::IRadio *radio;
    inet::physicallayer::IRadio::TransmissionState transmissionState;

    /** @brief the bit rate at which we transmit */
    double bitrate;

    inet::MACAddress addr;

    void printSequenceChartInfo(DSMEMessage* msg);
    void printDSMEManagement(uint8_t management, DSMESABSpecification& sabSpec, CommandFrameIdentifier cmd);

    bool channelInactive;

private:
    /** @brief Copy constructor is not allowed.
     */
    DSMEPlatform(const DSMEPlatform&);
    /** @brief Assignment operator is not allowed.
     */
    DSMEPlatform& operator=(const DSMEPlatform&);
};

}

#endif
