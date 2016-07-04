/*
 * Implementation of the Deterministic & Synchronous Multi-channel Extension (DSME)
 * described in the IEEE 802.15.4-2015 standard
 *
 * Authors: Florian Meier <florian.meier@tuhh.de>
 *          Institute of Telematics, Hamburg University of Technology
 *          Copyright 2015
 *
 * Based on
 *  DSME Implementation for the INET Framework
 *  Tobias Luebkert <tobias.luebkert@tuhh.de>
 *  Institute of Telematics, Hamburg University of Technology
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License Version 3
 * or later (the "LGPL").
 *
 * As a special exception, the copyright holders of this library give
 * you permission to link this library with independent modules to produce an
 * executable, regardless of the license terms of these independent modules,
 * and to copy and distribute the resulting executable under terms of your
 * choice, provided that you also meet, for each linked independent module,
 * the terms and conditions of the license of that module.
 * An independent module is a module which is not derived from or based
 * on this library.
 * If you modify this library, you may extend this exception to your version
 * of the library, but you are not obligated to do so. If you do not wish to
 * do so, delete this exception statement from your version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and the GNU Lesser General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "DSMEMessage.h"
#include "dsme_platform.h"

namespace dsme {

void DSMEMessage::prependFrom(DSMEMessageElement* me) {
    // TODO better fill buffer from the end (but this is not how it works in CometOS)
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
    memmove(this->frame->getData().data(), this->frame->getData().data()+me->getSerializationLength(), newSize);
    this->frame->getData().resize(newSize);
}

void DSMEMessage::copyTo(DSMEMessageElement* me) {
    Serializer s(this->frame->getData().data(), DESERIALIZATION);
    me->serialize(s);
    DSME_ASSERT(this->frame->getData().data() + me->getSerializationLength() == s.getData());
}

DSMEFrame* DSMEMessage::getSendableCopy() {
    DSMEMessage msg(frame->dup());

    // preamble, sfd and phy header will be added by the lower layer
    auto symbolsPayload = getTotalSymbols()
            - 2*4 // Preamble
            - 2*1 // SFD
            - 2*1; // PHY Header
    msg.frame->setBitLength(symbolsPayload*4); // 4 bit per symbol
    macHdr.prependTo(&msg);
    DSMEFrame* f = msg.frame;
    msg.frame = nullptr;
    return f;
}


}
