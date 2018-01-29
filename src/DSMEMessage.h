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

#include <omnetpp.h>

#include <inet/common/packet/Packet.h>

#include "openDSME/dsmeLayer/messages/IEEE802154eMACHeader.h"
#include "openDSME/interfaces/IDSMEMessage.h"
#include "openDSME/mac_services/dataStructures/DSMEMessageElement.h"

namespace dsme {

class DSMEPlatform;

class DSMEMessage : public omnetpp::cOwnedObject, public IDSMEMessage {
    friend class DSMEPlatform;
    friend class DSMEMessageElement;

public:
    void prependFrom(DSMEMessageElement*) override;

    void decapsulateTo(DSMEMessageElement*) override;

    bool hasPayload() override;

    uint16_t getTotalSymbols() override;

    uint32_t getStartOfFrameDelimiterSymbolCounter() override;

    void setStartOfFrameDelimiterSymbolCounter(uint32_t) override;

    IEEE802154eMACHeader& getHeader() override;

    uint8_t getLQI() override;
    void setLQI(uint8_t);

    bool getReceivedViaMCPS() override;

    void setReceivedViaMCPS(bool) override;

    bool getCurrentlySending() override;

    void setCurrentlySending(bool) override;

    uint8_t getRetryCounter() override;

    void increaseRetryCounter() override;

private:
    DSMEMessage();
    explicit DSMEMessage(inet::Packet*);
    ~DSMEMessage();

    inet::Packet* getSendableCopy();
    inet::Packet* decapsulatePacket();

    IEEE802154eMACHeader macHdr;
    inet::Packet* packet{nullptr};

    bool receivedViaMCPS{false}; // TODO better handling?
    bool firstTry{false};
    bool currentlySending{false};

    uint8_t lqi{0};
    uint8_t retries{0};
    uint32_t startOfFrameDelimiterSymbolCounter{0};
};
}

#endif
