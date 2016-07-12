//
// Copyright (C) 2013 Opensim Ltd
// Author: Levente Meszaros
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "SP_PositionTable.h"

namespace inet {

std::vector<L3Address> SP_PositionTable::getAddresses() const
{
    std::vector<L3Address> addresses;
    for (const std::pair<L3Address, AddressToPositionMapValue> & elem : addressToPositionMap)
        addresses.push_back(elem.first);
    return addresses;
}

bool SP_PositionTable::hasPosition(const L3Address& address) const
{
    AddressToPositionMap::const_iterator it = addressToPositionMap.find(address);
    return it != addressToPositionMap.end();
}

Coord SP_PositionTable::getPosition(const L3Address& address) const
{
    AddressToPositionMap::const_iterator it = addressToPositionMap.find(address);
    if (it == addressToPositionMap.end())
        return Coord(NaN, NaN, NaN);
    else
        return std::get<1>(it->second);
}

int SP_PositionTable::getInterfaceId(const L3Address& address) const
{
    AddressToPositionMap::const_iterator it = addressToPositionMap.find(address);
    if (it == addressToPositionMap.end())
        return -1;
    else
        return std::get<2>(it->second);
}

void SP_PositionTable::setPosition(const L3Address& address, const Coord& coord, int interfaceId)
{
    ASSERT(!address.isUnspecified());
    addressToPositionMap[address] = AddressToPositionMapValue(simTime(), coord, interfaceId);
}

void SP_PositionTable::removePosition(const L3Address& address)
{
    auto it = addressToPositionMap.find(address);
    addressToPositionMap.erase(it);
}

void SP_PositionTable::removeOldPositions(simtime_t timestamp)
{
    for (auto it = addressToPositionMap.begin(); it != addressToPositionMap.end(); )
        if (std::get<0>(it->second) <= timestamp)
            addressToPositionMap.erase(it++);
        else
            it++;

}

void SP_PositionTable::clear()
{
    addressToPositionMap.clear();
}

simtime_t SP_PositionTable::getOldestPosition() const
{
    simtime_t oldestPosition = SimTime::getMaxTime();
    for (const auto & elem : addressToPositionMap) {
        const simtime_t& time = std::get<0>(elem.second);
        if (time < oldestPosition)
            oldestPosition = time;
    }
    return oldestPosition;
}

std::ostream& operator << (std::ostream& o, const SP_PositionTable& t)
{
    o << "{ ";
    for(auto elem : t.addressToPositionMap) {
        o << elem.first << ":(" << std::get<0>(elem.second) << ";" << std::get<1>(elem.second) << ";" << std::get<2>(elem.second) << ") ";
    }
    o << "}";
    return o;
}

} // namespace inet

