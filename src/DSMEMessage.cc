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

void DSMEMessage::prependFrom(DSMEMessageElement* me) {
    // TODO better fill buffer from the end
    uint8_t oldSize = this->frame->getData().size();
    this->frame->getData().resize(oldSize + me->getSerializationLength());
    memmove(this->frame->getData().data() + me->getSerializationLength(), this->frame->getData().data(), oldSize);
    Serializer s(this->frame->getData().data(), SERIALIZATION);
    me->serialize(s);
    DSME_ASSERT(this->frame->getData().data() + me->getSerializationLength() == s.getData());
}

void DSMEMessage::decapsulateTo(DSMEMessageElement* me) {
    this->copyTo(me);
    uint8_t newSize = this->frame->getData().size() - me->getSerializationLength();
    memmove(this->frame->getData().data(), this->frame->getData().data() + me->getSerializationLength(), newSize);
    this->frame->getData().resize(newSize);
}

void DSMEMessage::copyTo(DSMEMessageElement* me) {
    Serializer s(this->frame->getData().data(), DESERIALIZATION);
    me->serialize(s);
    DSME_ASSERT(this->frame->getData().data() + me->getSerializationLength() == s.getData());
}

DSMEFrame* DSMEMessage::getSendableCopy() {
    DSMEMessage msg(frame->dup());

    // Preamble (4) | SFD (1) | PHY Hdr (1) | MAC Payload | FCS (2)
    // Preamble, sfd and phy header will be added by the lower layer (not actually, but for calculating the duration!)
    // However, the FCS will not be added by the lower layer since it is the task of the MAC layer, so do not subtract it!
    // By this it will be virtually added, though it is not part of the DSMEFrame itself.
    auto symbolsPayload = getTotalSymbols()
                          - 2 * 4 // Preamble
                          - 2 * 1 // SFD
                          - 2 * 1; // PHY Header
    msg.frame->setBitLength(symbolsPayload * 4); // 4 bit per symbol
    macHdr.prependTo(&msg);
    DSMEFrame* f = msg.frame;
    msg.frame = nullptr;
    return f;
}


}
