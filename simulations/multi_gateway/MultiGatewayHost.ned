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

package inet_dsme.simulations.multi_gateway;

import inet.mobility.contract.IMobility;

module MultiGatewayHost like IWirelessNode
{
    parameters:
        @networkNode;

        string mobilityType = default("ParentPositionMobility");
        int numWlanInterfaces;
        
        centerHost.mobilityType = "ParentPositionMobility";
        antennas[*].mobilityType = "ParentPositionCircularMobility";
        antennas[*].mobility.numAntennas = numWlanInterfaces;

    gates:
        input radioIn[numWlanInterfaces] @directIn;
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);

    submodules:
        centerHost: MultiGatewayCenterHost {
            parameters:
                @display("p=222,72");
        }

		antennas[numWlanInterfaces]: MultiGatewayAntennaHost {
            parameters:
                @display("p=99,72");
        }

        mobility: <mobilityType> like IMobility if mobilityType != "" {
            parameters:
                @display("p=40,19");
        }

    connections:
        for i=0..numWlanInterfaces-1 {
            centerHost.pppg++ <--> ned.DatarateChannel <--> antennas[i].pppg++;
            radioIn[i] --> { @display("m=s"); } --> antennas[i].radioIn[0];
        }
}
