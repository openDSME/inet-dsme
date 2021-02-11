package inet_dsme;

import inet.linklayer.base.MacProtocolBase;
import inet.linklayer.contract.IMacProtocol;

//
// IEEE802.15.4e
// Deterministic and synchronous multi-channel extension
//
simple DSME extends MacProtocolBase like IMacProtocol
{
    parameters:
        string address = default("auto"); // MAC address as hex string (12 hex digits), or
                                          // "auto". "auto" values will be replaced by
                                          // a generated MAC address in init stage 0.
        string radioModule = default("^.radio");   // The path to the Radio module  //FIXME remove default value


        bool macCapReduction = default(false);
        bool sendMultiplePacketsPerGTS = default(true);
        
        bool gackEnabled = default(false);
        int  groupAckOrder = default(5);		// 3 bits, same as MSFOrder

        bool isPANCoordinator = default(false);
        bool isCoordinator = default(false);
        double isCoordinatorProbability = default(0.5);

        // aMaxPHYPacketSize = 127 Octets (802.15.4-2006, page 45)
        // aMinMPDUOverhead = 9 Octets (802.15.4-2006, page 159)
        // aMaxMACPayloadSize = aMaxPHYPacketSize - aMinMPDUOverhead (802.15.4-2006, page 159)
        int mtu @unit("B") = 127 Byte - 9 Byte;

        int numCSMASlots = 8;

        int superframeOrder = default(3);			// 4 bits
        int multiSuperframeOrder = default(5);		// 3 bits
        int beaconOrder = default(7);				// 4 bits

        int finalCAPSlot = default(8);				// 4 bits

        // Scheduling
        string scheduling = default("TPS");
        double TPSalpha = default(0.1); // EWMA factor for TPS, ignored otherwise
        bool useHysteresis = default(true);
        xml staticSchedule = default(xml("<root/>"));

        int macDSMEGTSExpirationTime = default(7);
        int macResponseWaitTime = default(32);

        int numChannels = default(16);
        int commonChannel = default(11);
        int scanDuration = default(6);
        int macPANId = default(10);


        // bit rate
        double bitrate @unit(bps) = default(250000 bps);

        // minimum backoff exponent
        // Minimum backoff exponent
        // 802.15.4-2006, page 164
        int macMinBE = default(3);

        // Maximum backoff exponent
        // 802.15.4-2006, page 163
        int macMaxBE = default(5);

        // Maximum number of extra backoffs (excluding the first unconditional one) before frame drop
        // 802.15.4-2006, page 163
        int macMaxCSMABackoffs = default(4);

        // Maximum number of frame retransmission
        // 802.15.4-2006, page 164
        int macMaxFrameRetries = default(3);

        // LQI 150 corresponds roughly to 20% PER
        int minBroadcastLQI = default(150);
        int minCoordinatorLQI = default(150);

        @signal[sig_unicastDataSentDown](type=cPacket);
        @signal[sig_broadcastDataSentDown](type=cPacket);
        @signal[sig_commandSentDown](type=cPacket);
        @signal[sig_beaconSentDown](type=cPacket);
        @signal[sig_ackSentDown](type=cPacket);
        @signal[sig_corruptedFrameReceived](type=cPacket);
        @signal[sig_uncorruptedFrameReceived](type=cPacket);
        @signal[sig_GTSChange](type=long);
        @signal[sig_gackGTSChange](type=long);
        @signal[sig_queueLength](type=unsigned long);
        @signal[sig_numDroppedRetransmissionPackets](type=unsigned long);
        @signal[sig_numDroppedPackets](type=unsigned long);
        @signal[sig_acksInGack](type=unsigned long);
        @signal[sig_retransmissionQueueLength](type=unsigned long);
        @signal[sig_packetsTXPerSlot](type=unsigned long);
        @signal[sig_packetsRXPerSlot](type=unsigned long);
        @signal[sig_commandFrameDwellTime](type=unsigned long);
        @signal[sig_messagesInUse](type=unsigned long);
        @signal[sig_packetRetransmissionRate](type=double);

        @statistic[val_unicastDataSentDown](title="unicast packet sent down of type DATA"; source=sig_unicastDataSentDown; record=count; interpolationmode=none);
        @statistic[val_broadDataSentDown](title="broadcast packet sent down of type DATA"; source=sig_broadcastDataSentDown; record=count; interpolationmode=none);
        @statistic[val_corruptedFrameReceived](title="corrupted frame received"; source=sig_corruptedFrameReceived; record=count; interpolationmode=none);
        @statistic[val_uncorruptedFrameReceived](title="uncorrupted frame received"; source=sig_uncorruptedFrameReceived; record=count; interpolationmode=none);
        
        @statistic[vec_queueLength](title="Length of the neighbor queue"; source=sig_queueLength; record=min,mean,max,vector; interpolationmode=none);
        @statistic[vec_retransmissionQueueLength](title="Length of the retransmission queue"; source=sig_retransmissionQueueLength; record=min,mean,max,vector; interpolationmode=none);
        @statistic[vec_packetsTXPerSlot](title="Length of the neighbor queue"; source=sig_packetsTXPerSlot; record=min,mean,max,vector; interpolationmode=none);
        @statistic[vec_packetsRXPerSlot](title="Length of the neighbor queue"; source=sig_packetsRXPerSlot; record=min,mean,max,vector; interpolationmode=none);
        @statistic[vec_commandFrameDwellTime](title="Dwell time for command frames"; source=sig_commandFrameDwellTime; record=min,mean,max,vector; interpolationmode=none);
		@statistic[vec_messagesInUse](title="Messages In Use"; source=sig_messagesInUse; record=min,mean,max,vector; interpolationmode=none);
		@statistic[vec_GTSChange](title="Number of allocated GTS"; source=sig_GTSChange; record=min,mean,max,vector; interpolationmode=none);
		@statistic[vec_gackGTSChange](title="Number of allocated Gack GTS"; source=sig_gackGTSChange; record=min,mean,max,vector; interpolationmode=none);
		@statistic[vec_numDroppedRetransmissionPackets](title="Number of dropped packets because of full retransmission queue"; source=sig_numDroppedRetransmissionPackets; record=min,mean,max,vector; interpolationmode=none);
		@statistic[vec_numDroppedPackets](title="Number of dropped packets because of full neighbor queue"; source=sig_numDroppedPackets; record=min,mean,max,vector; interpolationmode=none);
		@statistic[vec_packetRetransmissionRate](title="Packet Retransmission Rate by group acknowledgments"; source=sig_packetRetransmissionRate; record=min,mean,max,vector; interpolationmode=none);
		@statistic[vec_acksInGack](title="Number of acknowledged packets in Gack Message"; source=sig_acksInGack; record=min,mean,max,vector; interpolationmode=none);
        @class(::dsme::DSMEPlatform);
}
