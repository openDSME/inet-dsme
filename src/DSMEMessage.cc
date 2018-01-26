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

#include "DSMEMessage.h"
#include "dsme_platform.h"

namespace dsme {

void DSMEMessage::prependFrom(DSMEMessageElement* messageElement) {
    std::vector<uint8_t> buffer(messageElement->getSerializationLength());
    Serializer serializer(buffer.data(), SERIALIZATION);
    messageElement->serialize(serializer);

    auto chunk = inet::makeShared<inet::BytesChunk>(buffer);
    packet->insertHeader(chunk);
}

void DSMEMessage::decapsulateTo(DSMEMessageElement* messageElement) {
    /*
     * We can't just pop the chunk right away, since the SerializationLength of the
     * element is only known after it is deserialized (depends on Frame Control).
     */
    std::vector<uint8_t> buffer(packet->peekAllBytes()->getBytes());
    Serializer serializer(buffer.data(), DESERIALIZATION);
    messageElement->serialize(serializer);

    packet->removeHeader(inet::B(messageElement->getSerializationLength()));
}

inet::Packet* DSMEMessage::getSendableCopy() {
    auto duplicate = DSMEMessage(packet->dup());
    macHdr.prependTo(&duplicate);
    return duplicate.decapsulatePacket();
}

bool DSMEMessage::hasPayload() {
    return packet->getByteLength() > 0;
}

uint16_t DSMEMessage::getTotalSymbols() {
    uint16_t bytes = macHdr.getSerializationLength() + packet->getByteLength()
                     + 4  // Preamble
                     + 1  // SFD
                     + 1  // PHY Header
                     + 2; // FCS
    return bytes * 2; // 4 bit per symbol
}

uint32_t DSMEMessage::getStartOfFrameDelimiterSymbolCounter() {
    return startOfFrameDelimiterSymbolCounter;
}

void DSMEMessage::setStartOfFrameDelimiterSymbolCounter(uint32_t value) {
    startOfFrameDelimiterSymbolCounter = value;
}

IEEE802154eMACHeader& DSMEMessage::getHeader() {
    return macHdr;
}

void DSMEMessage::setLQI(uint8_t lqi) {
    this->lqi = lqi;
}

uint8_t DSMEMessage::getLQI() {
    return lqi;
}

bool DSMEMessage::getReceivedViaMCPS() {
    return this->receivedViaMCPS;
}

void DSMEMessage::setReceivedViaMCPS(bool receivedViaMCPS) {
    this->receivedViaMCPS = receivedViaMCPS;
}

bool DSMEMessage::getCurrentlySending() {
    return this->currentlySending;
}

void DSMEMessage::setCurrentlySending(bool currentlySending) {
    this->currentlySending = currentlySending;
}

void DSMEMessage::increaseRetryCounter() {
    retries++;
}

uint8_t DSMEMessage::getRetryCounter() {
    return retries;
}

DSMEMessage::DSMEMessage() : packet(new inet::Packet{}) {
}

DSMEMessage::DSMEMessage(inet::Packet* packet) : packet{packet} {
}

DSMEMessage::~DSMEMessage() {
    if(packet != nullptr) {
        delete packet;
    }
}

inet::Packet* DSMEMessage::decapsulatePacket() {
    inet::Packet* temp_packet = packet;
    packet = nullptr;
    return temp_packet;
}

}
