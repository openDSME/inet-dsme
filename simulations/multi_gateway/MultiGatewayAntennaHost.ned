//
// Copyright (C) 2016 Maximilian Köstler <maximilian.koestler@tuhh.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
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

import inet.applications.generic.PRRTrafGen;
import inet.node.inet.WirelessHost;
import inet.linklayer.ieee802154.Ieee802154NarrowbandNic;
import inet.node.base.TransportLayerNodeBase;

import inet_dsme.sp_gpsr.SP_GPSR;

module MultiGatewayAntennaHost extends TransportLayerNodeBase
{
    parameters:
        @display("i=device/accesspoint;bgb=,-");

        numWlanInterfaces = default(1);
        
        hasIpv4 = false;
        hasGn = true;
        networkLayerType = "GenericNetworkLayer";
        forwarding = default(true);

        wlan[0].typename = default("WirelessNic");
        wlan[0].radioType = default("Ieee802154NarrowbandScalarRadio");

        *.routingTableModule = "^.generic.routingTable";
        *.networkProtocolModule = "^.generic.gnp";

    submodules:
        gpsr: SP_GPSR {
            @display("p=739,339");
        }

    connections:
        gpsr.ipOut --> tn.upperLayerIn++;
        gpsr.ipIn <-- tn.upperLayerOut++;
}
