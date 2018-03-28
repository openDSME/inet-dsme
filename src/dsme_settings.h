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

#ifndef DSME_SETTINGS_H
#define DSME_SETTINGS_H

#include <stdint.h>

#define STATISTICS_BEACONS

namespace dsme {

namespace const_redefines {
    constexpr uint8_t aNumSuperframeSlots = 16; // fixed value, see Table 8-80 of IEEE 802.15.4-2015, this includes beacon, CAP and GTS
    constexpr uint8_t macLIFSPeriod = 40;       // fixed value, see 8.1.3 of IEEE 802.15.4-2011 (assuming no UWB PHY)
    constexpr uint8_t macSIFSPeriod = 12;       // fixed value, see 8.1.3 of IEEE 802.15.4-2011 (assuming no UWB PHY)
}

constexpr uint8_t PRE_EVENT_SHIFT = const_redefines::macSIFSPeriod;
constexpr uint8_t MIN_CSMA_SLOTS = 0; // 0 for CAP reduction
constexpr uint8_t MAX_GTSLOTS = const_redefines::aNumSuperframeSlots - MIN_CSMA_SLOTS - 1;
constexpr uint8_t MAX_CHANNELS = 16;
constexpr uint8_t MAX_NEIGHBORS = 25;

constexpr uint8_t MIN_SO = 1;
constexpr uint8_t MAX_BO = 10;
constexpr uint8_t MAX_MO = 9;
constexpr uint16_t MAX_TOTAL_SUPERFRAMES = 1 << (uint16_t)(MAX_BO - MIN_SO);
constexpr uint16_t MAX_SUPERFRAMES_PER_MULTI_SUPERFRAME = 1 << (uint16_t)(MAX_MO - MIN_SO);
constexpr uint16_t MAX_OCCUPIED_SLOTS = MAX_SUPERFRAMES_PER_MULTI_SUPERFRAME * MAX_GTSLOTS * MAX_CHANNELS;

constexpr uint8_t MAX_SAB_UNITS = 1;

constexpr uint16_t CAP_QUEUE_SIZE = 8;
constexpr uint16_t TOTAL_GTS_QUEUE_SIZE = 30 - CAP_QUEUE_SIZE;
constexpr uint16_t UPPER_LAYER_QUEUE_SIZE = 12;
constexpr uint16_t MSG_POOL_SIZE = CAP_QUEUE_SIZE + TOTAL_GTS_QUEUE_SIZE + 2 * UPPER_LAYER_QUEUE_SIZE + 10;
constexpr uint8_t ADDITIONAL_ACK_WAIT_DURATION = 0;
}

#endif
