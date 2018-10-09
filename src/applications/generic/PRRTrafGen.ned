//
// Copyright (C) 2016 Florian Kauer <florian.kauer@koalo.de>
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


package inet_dsme.applications.generic;

import inet.applications.generic.IpvxTrafGen;

simple PRRTrafGen extends IpvxTrafGen
{
    parameters:
        // After the startTime, the generator starts sending dummy packets that do
        // not count to the numPackets and will not be counted at the receiver.
        // After the warmUpTime, numPackets will be sent and counted.
        // Afterwards, the generator will continue generating dummy packets,
        // if continueSendingDummyPackets is set. If all traffic generators are finished,
        // a timer of coolDownDuration is scheduled that will end the simulation.
        double warmUpDuration @unit("s") = default(10s);
        double coolDownDuration @unit("s") = default(10s);
        bool continueSendingDummyPackets = default(true);
        
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @signal[sinkRcvdPk](type=inet::Packet);
        @signal[sourceSentPk](type=inet::Packet);
        @signal[sentDummyPk](type=inet::Packet);
        @signal[rcvdPkFrom-*](type=inet::Packet);
        @signal[sentPkTo-*](type=inet::Packet);
        @signal[pktRcvdAfterWarmup](type=inet::Packet);

        @statisticTemplate[rcvdPkFrom](record=count);
        @statisticTemplate[sentPkTo](record=count);
        @statistic[sinkRcvdPk](title="own packets received by sink"; source=sinkRcvdPk; record=count,vector(packetBytes); interpolationmode=none);
        @statistic[sourceSentPk](title="packets sent to me"; source=sourceSentPk; record=count,vector(packetBytes); interpolationmode=none);
        @statistic[sentDummyPk](title="dummy packets sent"; source=sentDummyPk; record=count,vector(packetBytes); interpolationmode=none);
        @statistic[sinkRcvdPkDelay](title="delay of own packets received by sink"; source=messageAge(sinkRcvdPk); record=mean,max; interpolationmode=none);
        @statistic[PRR](title="PRR"; source=count(sinkRcvdPk)/count(packetSent); record=last,vector; interpolationmode=none);
	@statistic[pktRcvdAfterWarmup](title="Packets received after warmup"; source=pktRcvdAfterWarmup; record=count,vector(packetBytes));

        @class(PRRTrafGen);
}

