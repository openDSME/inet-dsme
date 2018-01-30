//
// Copyright (C) 2016 Florian Kauer <florian.meier@koalo.de>
// Copyright (C) 2017 Maximilian Köstler <maximilian.koestler@tuhh.de>
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


import inet.linklayer.ieee802154.Ieee802154NarrowbandInterface;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.routing.gpsr.Gpsr;

import inet_dsme.applications.generic.PRRTrafGen;

//
// A wireless host containing a WirelessInterface, GPSR and a PRRTrafGen.
//
module IEEE802154ExampleHost extends WirelessHost
{
    parameters:
        @display("i=device/accesspoint;bgb=,-");

        forwarding = default(true);

        hasIpv4 = false;
        hasIpv6 = false;
        hasGn   = true;

        hasUdp = true;
        hasTcp = false;
        hasSctp = false;

        networkLayerType = "GenericNetworkLayer";

	generic.gnp.procDelay = default(2ms);

        wlan[*].typename = default("WirelessInterface");
        wlan[*].radioType = default("Ieee802154NarrowbandScalarRadio");

    submodules:
        gpsr: Gpsr {
            parameters:
                @display("p=739,339");
                routingTableModule = "^.generic.routingTable";
                networkProtocolModule = "^.generic.gnp";
        }

        trafficgen: PRRTrafGen {
            parameters:
                @display("p=188,287");
                protocol     = default(6); // TCP (was unused)
                packetLength = default(10B);
        }

    connections:
        trafficgen.ipOut --> tn.in++;
        trafficgen.ipIn <-- tn.out++;

        gpsr.ipOut --> tn.in++;
        gpsr.ipIn <-- tn.out++;
}
