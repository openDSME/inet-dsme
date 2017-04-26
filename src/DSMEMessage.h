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

#ifndef DSMEMESSAGE_H
#define DSMEMESSAGE_H

#include <stdint.h>

#include "openDSME/dsmeLayer/messages/IEEE802154eMACHeader.h"
#include "openDSME/interfaces/IDSMEMessage.h"
#include "openDSME/mac_services/dataStructures/DSMEMessageElement.h"

#include "DSMEFrame_m.h"

namespace dsme {

class DSMEPlatform;

class DSMEMessage : public omnetpp::cOwnedObject, public IDSMEMessage {
    friend class DSMEPlatform;
    friend class DSMEMessageElement;

public:
    void prependFrom(DSMEMessageElement* msg) override;

    void decapsulateTo(DSMEMessageElement* msg) override;

    void copyTo(DSMEMessageElement* msg) override;

    uint8_t getByte(uint8_t pos) override {
        return frame->getData().at(pos);
    }

    bool hasPayload() override {
        return (frame->getData().size() > 0) || (frame->hasEncapsulatedPacket() && frame->getEncapsulatedPacket()->getByteLength() > 0);
    }

    uint16_t getTotalSymbols() override {
        uint16_t bytes = macHdr.getSerializationLength() + frame->getData().size() + 4 // Preamble
                         + 1                                                           // SFD
                         + 1                                                           // PHY Header
                         + 2;                                                          // FCS

        if(frame->hasEncapsulatedPacket()) {
            bytes += frame->getEncapsulatedPacket()->getByteLength();
        }

        return bytes * 2; // 4 bit per symbol
    }

    uint32_t getStartOfFrameDelimiterSymbolCounter() override {
        return startOfFrameDelimiterSymbolCounter;
    }

    uint32_t getReceptionSymbolCounter() override {
        return startOfFrameDelimiterSymbolCounter + 2 * (this->getHeader().getSerializationLength() + frame->getData().size()) + 2; // 2 Symbols for PHY header
    }

    void setReceptionSymbolCounter(uint32_t counter) override {
        startOfFrameDelimiterSymbolCounter = counter - (2 * (this->getHeader().getSerializationLength() + frame->getData().size()) + 2);
    }

    IEEE802154eMACHeader& getHeader() override {
        return macHdr;
    }

    void setLQI(uint8_t lqi) {
        this->lqi = lqi;
    }

    uint8_t getLQI() override {
        return lqi;
    }

    bool getReceivedViaMCPS() override {
        return this->receivedViaMCPS;
    }

    void setReceivedViaMCPS(bool receivedViaMCPS) override {
        this->receivedViaMCPS = receivedViaMCPS;
    }

    bool getCurrentlySending() override {
        return this->currentlySending;
    }

    void setCurrentlySending(bool currentlySending) override {
        this->currentlySending = currentlySending;
    }

    void increaseRetryCounter() override {
        retries++;
    }

    uint8_t getRetryCounter() override {
        return retries;
    }

private:
    bool receivedViaMCPS{false}; // TODO better handling?
    bool firstTry{false};
    bool currentlySending{false};

    IEEE802154eMACHeader macHdr;

    DSMEFrame* frame;

    uint8_t lqi{0};
    uint8_t retries{0};

    DSMEMessage() : frame(new DSMEFrame()) {
    }

    DSMEMessage(DSMEFrame* frame) : frame(frame) {
    }

    ~DSMEMessage() {
        if(frame != nullptr) {
            delete frame;
        }
    }

    uint32_t startOfFrameDelimiterSymbolCounter{0};

    DSMEFrame* getSendableCopy();

    DSMEFrame* decapsulateFrame() {
        DSMEFrame* f = frame;
        frame = nullptr;
        return f;
    }
};
}

#endif
