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

#ifndef PLATFORM_OMNETPP_H
#define PLATFORM_OMNETPP_H

#include <omnetpp.h>

using omnetpp::getThisPtr;

#if 1
#define palId_id() ((static_cast<DSMEPlatform*>(omnetpp::cSimulation::getActiveSimulation()->getContextModule()))->getAddress().getShortAddress())
#define cometos std
#define LOG_INFO(x)                                                               \
    do {                                                                          \
        EV_INFO << (omnetpp::simTime()) << " \t " << palId_id() << ": " << x << std::endl; \
    } while(0)
#define LOG_INFO_PURE(x) \
    do {                 \
        EV_INFO << x;    \
    } while(0)
#define LOG_INFO_PREFIX                                         \
    do {                                                        \
        EV_INFO << (omnetpp::simTime()) << " \t " << palId_id() << ": "; \
    } while(0)
#define HEXOUT std::hex
#define DECOUT std::dec
#define LOG_ENDL std::endl
#else
#define LOG_INFO(x)
#define LOG_INFO_PURE(x)
#define LOG_INFO_PREFIX
#define HEXOUT
#endif

#define LOG_ERROR(x) LOG_INFO(x)
#define LOG_WARN(x) LOG_INFO(x)
#define LOG_DEBUG(x) LOG_INFO(x)
#define LOG_DEBUG_PURE(x) LOG_INFO_PURE(x)
#define LOG_DEBUG_PREFIX LOG_INFO_PREFIX
#define FLOAT_OUTPUT(x) (x)

void _simulation_will_terminate(void);

#define DSME_ASSERT(x)                    \
    do {                                  \
        if(!(x)) {                        \
            _simulation_will_terminate(); \
            std::cout << "ASSERT: " << __FILE__ << " " << __LINE__ << std::endl;\
            std::cerr << "ASSERT: " << __FILE__ << " " << __LINE__ << std::endl;\
            exit(1);                      \
        }                                 \
    } while(0)
#define DSME_SIM_ASSERT(x) DSME_ASSERT(x)

#include "DSMEMessage.h"
#include "DSMEPlatform.h"
#include "dsme_atomic.h"
#include "dsme_settings.h"

#endif
