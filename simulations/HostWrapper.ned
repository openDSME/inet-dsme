//
// Copyright (C) 2016 Florian Kauer <florian.meier@koalo.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet_dsme.simulations;

import inet.mobility.contract.IMobility;
import inet.node.inet.INetworkNode;

module HostWrapper like INetworkNode
{
    parameters:
        @networkNode;
        @display("i=device/accesspoint;bgb=91.64,51.350002");

        string mobilityType;
        int numWlanInterfaces = default(1);
        string hostType = default("SimpleHost");
        
        //wrappedHost.initialX = mobility.lastPosition.x;
        //wrappedHost.**.mobility.**.lastPosition = mobility.lastPosition;
        

    gates:
        input radioIn[numWlanInterfaces] @directIn;
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);

    submodules:
        mobility: <mobilityType> like IMobility if mobilityType != "" {
            parameters:
                @display("p=29,27");
        }
        
        wrappedHost: <hostType> like IWirelessNode {
            parameters:
                @display("i=device/accesspoint;p=64.385,26.465");
        }

    connections:
        for i=0..numWlanInterfaces-1 {
            radioIn[i] --> { @display("m=s"); } --> wrappedHost.radioIn[i];
        }
}
