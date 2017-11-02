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

import inet.node.inet.WirelessHost;
import inet.linklayer.ieee802154.Ieee802154NarrowbandNic;
import inet.routing.gpsr.GPSR;

import inet_dsme.applications.generic.PRRTrafGen;

//
// A wireless host containing a WirelessNic, GPSR and a PRRTrafGen.
//
module IEEE802154ExampleHost extends WirelessHost
{
    parameters:
        @display("i=device/accesspoint;bgb=,-");

        forwarding = default(true);

        networkLayerType = "GenericNetworkLayer";
        networkLayer.gnp.procDelay = default(2ms);

        wlan[*].typename = default("WirelessNic");
        wlan[*].radioType = default("Ieee802154NarrowbandScalarRadio");

    submodules:
        gpsr: GPSR {
            parameters:
                @display("p=739,339");
                networkProtocolModule = "^.networkLayer.gnp";
        }

        trafficgen: PRRTrafGen {
            parameters:
                @display("p=188,287");
                protocol = 17;
                packetLength = default(10B);
        }

    connections:
        trafficgen.ipOut --> networkLayer.transportIn++;
        trafficgen.ipIn <-- networkLayer.transportOut++;

        gpsr.ipOut --> networkLayer.transportIn++;
        gpsr.ipIn <-- networkLayer.transportOut++;
}
