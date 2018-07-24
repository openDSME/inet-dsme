//
// Copyright (C) 2013 Opensim Ltd
// Author: Levente Meszaros
// Author: Institute of Telematics, Hamburg University of Technology (SP_GPSR)
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

#include "SP_GPSR.h"

using namespace inet;

namespace inet_dsme {

Define_Module(SP_GPSR);

GpsrOption *SP_GPSR::createGpsrOption(L3Address destination)
{
    if(!par("straightPath")) {
        return inet::Gpsr::createGpsrOption(destination);
    }

    GpsrOption *gpsrOption = new GpsrOption();
    gpsrOption->setRoutingMode(GPSR_GREEDY_ROUTING);
    gpsrOption->setDestinationPosition(lookupPositionInGlobalRegistry(destination));

    gpsrOption->setPerimeterRoutingStartPosition(mobility->getCurrentPosition()); // for straight path
    gpsrOption->setLength(computeOptionLength(gpsrOption));
    return gpsrOption;
}

L3Address SP_GPSR::findGreedyRoutingNextHop(const Ptr<const NetworkHeaderBase>& networkHeader, const L3Address& destination)
{
    if(!par("straightPath")) {
        return inet::Gpsr::findGreedyRoutingNextHop(networkHeader, destination);
    }

    EV_DEBUG << "Finding next hop using greedy routing: destination = " << destination << endl;
    const GpsrOption *gpsrOption = getGpsrOptionFromNetworkDatagram(networkHeader);
    L3Address selfAddress = getSelfAddress();
    Coord selfPosition = mobility->getCurrentPosition();
    Coord destinationPosition = gpsrOption->getDestinationPosition();
    L3Address bestNeighbor;
    Coord sourcePosition = gpsrOption->getPerimeterRoutingStartPosition();

    double sqDistSelf = selfPosition.sqrdist(destinationPosition);     // distance self to destination
    double sqDistNext;
    double sqLineDistBest = INFINITY;
    // Straight line from src to dest

    std::vector<L3Address> neighborAddresses = neighborPositionTable.getAddresses();
    for (auto& neighborAddress : neighborAddresses) {
        if(neighborAddress == destination) {
            bestNeighbor = destination;
            break;
        }

        Coord p = neighborPositionTable.getPosition(neighborAddress);
        sqDistNext = destinationPosition.sqrdist(p);    // distance of neighbor to destination

        if (sqDistNext < sqDistSelf) {                  // only nodes which are closer to destination
            const Coord &a = sourcePosition;
            const Coord &b = destinationPosition;
            const Coord &c = b - a;

            double sp = (c.x * (p.x - sourcePosition.x) + c.y * (p.y - sourcePosition.y)) / c.squareLength();
            double d2 = (p - (a + c * sp)).squareLength();

            if(d2 < sqLineDistBest) {
                sqLineDistBest = d2;
                bestNeighbor = neighborAddress;
            }
        }
    }

    if (bestNeighbor.isUnspecified()) {
        EV_DEBUG << "Switching to perimeter routing: destination = " << destination << endl;
        // KLUDGE: TODO: const_cast<GpsrOption *>(gpsrOption)
        const_cast<GpsrOption *>(gpsrOption)->setRoutingMode(GPSR_PERIMETER_ROUTING);
        const_cast<GpsrOption *>(gpsrOption)->setPerimeterRoutingStartPosition(selfPosition);
        const_cast<GpsrOption *>(gpsrOption)->setPerimeterRoutingForwardPosition(selfPosition);
        const_cast<GpsrOption *>(gpsrOption)->setCurrentFaceFirstSenderAddress(selfAddress);
        const_cast<GpsrOption *>(gpsrOption)->setCurrentFaceFirstReceiverAddress(L3Address());
        return findPerimeterRoutingNextHop(networkHeader, destination);
    }
    else
        return bestNeighbor;
}

L3Address SP_GPSR::findPerimeterRoutingNextHop(const Ptr<const NetworkHeaderBase>& networkHeader, const L3Address& destination)
{
    if(!par("straightPath")) {
        return inet::Gpsr::findPerimeterRoutingNextHop(networkHeader, destination);
    }

    EV_DEBUG << "Finding next hop using perimeter routing: destination = " << destination << endl;
    const GpsrOption *gpsrOption = getGpsrOptionFromNetworkDatagram(networkHeader);
    L3Address selfAddress = getSelfAddress();
    Coord selfPosition = mobility->getCurrentPosition();
    Coord perimeterRoutingStartPosition = gpsrOption->getPerimeterRoutingStartPosition();
    Coord destinationPosition = gpsrOption->getDestinationPosition();
    double selfDistance = (destinationPosition - selfPosition).length();
    double perimeterRoutingStartDistance = (destinationPosition - perimeterRoutingStartPosition).length();
    if (selfDistance < perimeterRoutingStartDistance) {
        EV_DEBUG << "Switching to greedy routing: destination = " << destination << endl;
        // KLUDGE: TODO: const_cast<GpsrOption *>(gpsrOption)
        const_cast<GpsrOption *>(gpsrOption)->setRoutingMode(GPSR_GREEDY_ROUTING);
        //const_cast<GpsrOption *>(gpsrOption)->setPerimeterRoutingStartPosition(Coord()); also used for straightest path routing
        const_cast<GpsrOption *>(gpsrOption)->setPerimeterRoutingForwardPosition(Coord());
        const_cast<GpsrOption *>(gpsrOption)->setCurrentFaceFirstSenderAddress(L3Address());
        const_cast<GpsrOption *>(gpsrOption)->setCurrentFaceFirstReceiverAddress(L3Address());
        return findGreedyRoutingNextHop(networkHeader, destination);
    }
    else {
        const L3Address& firstSenderAddress = gpsrOption->getCurrentFaceFirstSenderAddress();
        const L3Address& firstReceiverAddress = gpsrOption->getCurrentFaceFirstReceiverAddress();
        auto senderNeighborAddress = getSenderNeighborAddress(networkHeader);
        auto neighborAngle = senderNeighborAddress.isUnspecified() ? getVectorAngle(destinationPosition - mobility->getCurrentPosition()) : getNeighborAngle(senderNeighborAddress);
        L3Address selectedNeighborAddress;
        std::vector<L3Address> neighborAddresses = getPlanarNeighborsCounterClockwise(neighborAngle);
        for (auto& neighborAddress : neighborAddresses) {
            Coord neighborPosition = getNeighborPosition(neighborAddress);
            Coord intersection = computeIntersectionInsideLineSegments(perimeterRoutingStartPosition, destinationPosition, selfPosition, neighborPosition);
            if (std::isnan(intersection.x)) {
                selectedNeighborAddress = neighborAddress;
                break;
            }
            else {
                EV_DEBUG << "Edge to next hop intersects: intersection = " << intersection << ", nextNeighbor = " << selectedNeighborAddress << ", firstSender = " << firstSenderAddress << ", firstReceiver = " << firstReceiverAddress << ", destination = " << destination << endl;
                // KLUDGE: TODO: const_cast<GpsrOption *>(gpsrOption)
                const_cast<GpsrOption *>(gpsrOption)->setCurrentFaceFirstSenderAddress(selfAddress);
                const_cast<GpsrOption *>(gpsrOption)->setCurrentFaceFirstReceiverAddress(L3Address());
                const_cast<GpsrOption *>(gpsrOption)->setPerimeterRoutingForwardPosition(intersection);
            }
        }
        if (selectedNeighborAddress.isUnspecified()) {
            EV_DEBUG << "No suitable planar graph neighbor found in perimeter routing: firstSender = " << firstSenderAddress << ", firstReceiver = " << firstReceiverAddress << ", destination = " << destination << endl;
            return L3Address();
        }
        else if (firstSenderAddress == selfAddress && firstReceiverAddress == selectedNeighborAddress) {
            EV_DEBUG << "End of perimeter reached: firstSender = " << firstSenderAddress << ", firstReceiver = " << firstReceiverAddress << ", destination = " << destination << endl;
            return L3Address();
        }
        else {
            if (gpsrOption->getCurrentFaceFirstReceiverAddress().isUnspecified())
                // KLUDGE: TODO: const_cast<GpsrOption *>(gpsrOption)
                const_cast<GpsrOption *>(gpsrOption)->setCurrentFaceFirstReceiverAddress(selectedNeighborAddress);
            return selectedNeighborAddress;
        }
    }
}

} // namespace inet

