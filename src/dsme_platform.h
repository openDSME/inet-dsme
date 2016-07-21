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

#ifndef PLATFORM_OMNETPP_H
#define PLATFORM_OMNETPP_H

#if 1
#define palId_id() (((DSMEPlatform*)(cSimulation::getActiveSimulation()->getContextModule()))->getAddress().getShortAddress())
#define cometos std
#define LOG_INFO(x) do{ EV_INFO << (simTime()) << " \t " << palId_id() << ": " << x << std::endl; } while(0)
#define LOG_INFO_PURE(x) do{ EV_INFO << x;} while(0)
#define LOG_INFO_PREFIX do{ EV_INFO << (simTime()) << " \t " << palId_id() << ": ";} while(0)
#define HEXOUT std::hex
#define DECOUT std::dec
#else
#define LOG_INFO(x)
#define LOG_INFO_PURE(x)
#define LOG_INFO_PREFIX
#define HEXOUT
#endif

#define LOG_WARN(x) LOG_INFO(x)
#define LOG_DEBUG(x) LOG_INFO(x)
#define LOG_DEBUG_PURE(x) LOG_INFO_PURE(x)
#define LOG_DEBUG_PREFIX LOG_INFO_PREFIX

#define DSME_ASSERT(x) if(!(x)) LOG_WARN("ASSERT"); ASSERT(x)

#include "DSMEMessage.h"
#include "dsme_settings.h"
#include "dsme_atomic.h"
#include "DSMEPlatform.h"
#include "phy_constants.h"


#endif
