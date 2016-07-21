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

#ifndef PLATFORM_ATOMIC_OMNETPP_H
#define PLATFORM_ATOMIC_OMNETPP_H

// recursive locks necessary!
inline void dsme_atomicBegin() {
    // not needed for INET
}

inline void dsme_atomicEnd() {
    // not needed for INET
}

#endif /* PLATFORM_ATOMIC_OMNETPP_H */
