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

#ifndef STATICSCHEDULE_H 
#define STATICSCHEDULE_H

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

struct StaticSlot {
    uint8_t superframeID; 
    uint8_t slotID; 
    uint8_t channelID; 
    uint8_t direction; // 0->TX, 1->RX
    uint16_t address; 
};

class StaticSchedule {
public:
    StaticSchedule() = delete;
    virtual ~StaticSchedule() = delete;

    static std::vector<StaticSlot> loadSchedule(omnetpp::cXMLElement *xmlFile, uint16_t address);  
};

} /* namespace dsme */

#endif /* STATICSCHEDULE_H */
