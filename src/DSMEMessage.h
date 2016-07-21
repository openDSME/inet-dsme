/*
 * Implementation of the Deterministic & Synchronous Multi-channel Extension (DSME)
 * introduced in the IEEE 802.15.4e-2012 standard
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

#ifndef DSMEMESSAGE_H
#define DSMEMESSAGE_H

#include <stdint.h>

#include "openDSME/mac_services/dataStructures/DSMEMessageElement.h"
#include "openDSME/dsmeLayer/messages/IEEE802154eMACHeader.h"
#include "openDSME/interfaces/IDSMEMessage.h"

#include "DSMEFrame_m.h"

namespace dsme {

class DSMEPlatform;

class DSMEMessage : public omnetpp::cOwnedObject, public IDSMEMessage {
public:
    void prependFrom(DSMEMessageElement* msg);

    void decapsulateTo(DSMEMessageElement* msg);

    void copyTo(DSMEMessageElement* msg);

    uint8_t getByte(uint8_t pos) {
        return frame->getData().at(pos);
    }

    bool hasPayload() {
        return (frame->getData().size() > 0 ) || (frame->hasEncapsulatedPacket() && frame->getEncapsulatedPacket()->getByteLength() > 0);
    }

    uint16_t getTotalSymbols() {
        uint16_t bytes = macHdr.getSerializationLength()
                                   + frame->getData().size()
                                   + 2 // FCS
                                   + 4 // Preamble
                                   + 1 // SFD
                                   + 1; // PHY Header

        if(frame->hasEncapsulatedPacket()) {
            bytes += frame->getEncapsulatedPacket()->getByteLength();
        }

        return bytes*2; // 4 bit per symbol
    }


    // gives the symbol counter at the end of the SFD
    uint32_t getStartOfFrameDelimiterSymbolCounter() override {
        return startOfFrameDelimiterSymbolCounter;
    }

    uint32_t getReceptionSymbolCounter() override {
        return startOfFrameDelimiterSymbolCounter + 2 * (this->getHeader().getSerializationLength() + frame->getData().size()) + 2; // 2 Symbols for PHY header
    }


    IEEE802154eMACHeader& getHeader() {
        return macHdr;
    }

    bool receivedViaMCPS; // TODO better handling?
    bool firstTry;
    bool currentlySending;

private:
    // TODO
    //    uint16_t bits;
    IEEE802154eMACHeader macHdr;

    DSMEFrame* frame;

    DSMEMessage() :
            currentlySending(false),
            frame(new DSMEFrame()) {
    }

    DSMEMessage(DSMEFrame* frame) :
            currentlySending(false),
            frame(frame) {
    }

    ~DSMEMessage() {
        if (frame != nullptr) {
            delete frame;
        }
    }

    uint32_t startOfFrameDelimiterSymbolCounter;

    friend class DSMEPlatform;
    friend class DSMEMessageElement;

    DSMEFrame* getSendableCopy();

    DSMEFrame* decapsulateFrame() {
        DSMEFrame* f = frame;
        frame = nullptr;
        return f;
    }
};

}

#endif
