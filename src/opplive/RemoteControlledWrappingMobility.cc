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

#include <RemoteControlledWrappingMobility.h>
#include "ParentPositionMobility.h"

#include <string>
#include <fstream>
#include <streambuf>

namespace inet_dsme {

Define_Module(RemoteControlledWrappingMobility);

void RemoteControlledWrappingMobility::setPosition(int x, int y) {
    lastPosition.x = x;
    lastPosition.y = y;
    recordScalar("x", lastPosition.x);
    recordScalar("y", lastPosition.y);
    recordScalar("z", lastPosition.z);
    checkPosition();
    updateVisualRepresentation();
    updateWrappedMobility();

}

void RemoteControlledWrappingMobility::setInitialPosition() {
    Json::Reader reader;
    Json::Value root;

    std::ifstream input("initial_position.json");
    std::string str((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

    reader.parse(str, root);
    unsigned int size = root.size();
    unsigned int index = visualRepresentation->getIndex();

    ASSERT(index < size);

    Json::Value& position = root[index];

    Json::Value& x = position["x"];
    Json::Value& y = position["y"];

    lastPosition.x = x.asInt();
    lastPosition.y = y.asInt();

    updateWrappedMobility();

    std::stringstream stream;
    stream << "http://opendsme.org/rpc/setPosition/" << index;
    addRemoteProcedure(stream.str(), &RemoteControlledWrappingMobility::setPosition);
}

void RemoteControlledWrappingMobility::updateWrappedMobility() {
    cModule *wrappedHost = this->getParentModule()->getSubmodule("wrappedHost");
    if (wrappedHost) {
        cModule *wrappedHostMobility = wrappedHost->getSubmodule("mobility");
        if (wrappedHostMobility) {
            ParentPositionMobility* wrappedHostParentPositionMobility;
            wrappedHostParentPositionMobility = dynamic_cast<ParentPositionMobility*>(wrappedHostMobility);
            if (wrappedHostParentPositionMobility) {
                wrappedHostParentPositionMobility->updatePosition();
            }
        }
    }
}

} // namespace inet

