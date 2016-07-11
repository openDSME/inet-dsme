/*
 * Copyright (C) 2014 Florian Meier <florian.meier@koalo.de>
 *
 * Based on:
 * Copyright (C) 2006 Isabel Dietrich <isabel.dietrich@informatik.uni-erlangen.de>
 * Copyright (C) 2013 OpenSim Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "ParentPositionCircularMobility.h"

#include <omnetpp.h>

namespace inet_dsme {

Define_Module(ParentPositionCircularMobility);

void ParentPositionCircularMobility::updatePosition() {
    unsigned int numAntennas = par("numAntennas");
    double distance = par("distance");

    unsigned int index = visualRepresentation->getIndex();

    cModule *parent = this->getParentModule();
    if (parent) {
        cModule *grandParent = parent->getParentModule();
        if (grandParent) {
            cModule *parentMobility = grandParent->getSubmodule("mobility");
            if (parentMobility) {
                DSMEMobilityBase* parentMobilityBase = check_and_cast<DSMEMobilityBase*>(parentMobility);

                lastPosition.x = parentMobilityBase->getCurrentPosition().x;
                lastPosition.y = parentMobilityBase->getCurrentPosition().y;
                lastPosition.z = parentMobilityBase->getCurrentPosition().z;

                if (numAntennas > 1) {
                    double radius = distance;
                    double angularStep = 2.0 * M_PI / numAntennas;
                    double angle = angularStep * index;
                    lastPosition.x += (radius * cos(angle));
                    lastPosition.y += (radius * sin(angle));
                }

                recordScalar("x", lastPosition.x);
                recordScalar("y", lastPosition.y);
                recordScalar("z", lastPosition.z);
            }
        }
    }
}

} // namespace inet

