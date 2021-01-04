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
        double startupTime @unit("s") = default(0s);


        string address = default("auto"); // MAC address as hex string (12 hex digits), or
                                          // "auto". "auto" values will be replaced by
                                          // a generated MAC address in init stage 0.
        string radioModule = default("^.radio");   // The path to the Radio module  //FIXME remove default value

        bool useQAgent = default(false);

        bool macCapReduction = default(false);
        bool sendMultiplePacketsPerGTS = default(false);

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
        
        bool slottedCSMA = default(false);
	bool batteryLifeExtension = default(false);

        @signal[unicastDataSentDown](type=cPacket);
        @signal[broadcastDataSentDown](type=cPacket);
        @signal[commandSentDown](type=cPacket);
        @signal[beaconSentDown](type=cPacket);
        @signal[ackSentDown](type=cPacket);
        @signal[corruptedFrameReceived](type=cPacket);
        @signal[uncorruptedFrameReceived](type=cPacket);
        @signal[GTSChange](type=long);
        @signal[queueLength](type=unsigned long);
        @signal[queueLevelCAP](type=unsigned long);
        @signal[packetsTXPerSlot](type=long);
        @signal[packetsRXPerSlot](type=long);
        @signal[commandFrameDwellTime](type=long);

        @signal[packetsPerCAP](type=unsigned long);
        @signal[failedPacketsPerCAP](type=unsigned long);
        @signal[failedCCAs](type=unsigned long);
        @signal[prrCAP](type=double);
	@signal[successPacketsCAP](type=unsigned long);

        @signal[csmaRetransmissions](type=unsigned int);
        @signal[csmaSuccess](type=unsigned int);
        @signal[reward](type=double);
        @signal[q](type=double);
        @signal[be](type=unsigned int);
        @signal[eps](type=double);
        @signal[capSuccess](type=unsigned int);
        @signal[capFailed](type=unsigned int);
        @signal[overheardMsg](type=unsigned int);


        @statistic[unicastDataSentDown](title="unicast packet sent down of type DATA"; source=unicastDataSentDown; record=count; interpolationmode=none);
        @statistic[broadDataSentDown](title="broadcast packet sent down of type DATA"; source=broadcastDataSentDown; record=count; interpolationmode=none);
        @statistic[corruptedFrameReceived](title="corrupted frame received"; source=corruptedFrameReceived; record=count; interpolationmode=none);
        @statistic[uncorruptedFrameReceived](title="uncorrupted frame received"; source=uncorruptedFrameReceived; record=count; interpolationmode=none);
        @statistic[GTSChange](title="GTS allocation or deallocation"; source=GTSChange; record=mean,max,vector; interpolationmode=none);
        @statistic[queueLevelCAP](title="Level of the CAP queue"; source=queueLevelCAP; record=mean,max,vector; interpolationmode=none);
        @statistic[queueLength](title="Length of the neighbor queue"; source=queueLength; record=mean,max,vector; interpolationmode=none);
        @statistic[packetsTXPerSlot](title="Length of the neighbor queue"; source=packetsTXPerSlot; record=min,mean,max,vector; interpolationmode=none);
        @statistic[packetsRXPerSlot](title="Length of the neighbor queue"; source=packetsRXPerSlot; record=min,mean,max,vector; interpolationmode=none);
        @statistic[commandFrameDwellTime](title="Dwell time for command frames"; source=commandFrameDwellTime; record=min,mean,max; interpolationmode=none);

        @statistic[csmaRetransmissions](title="Ratio of succesful CSMA transmission to number of backoffs"; source=csmaRetransmissions; record=vector; interpolationmode=none);
        @statistic[csmaSuccess](title="CSMA transmission result"; source=csmaSuccess; record=vector; interpolationmode=none);
        @statistic[reward](title="Reward received by the QAgent"; source=reward; record=vector; interpolationmode=none);
        @statistic[q](title="Q achieved by the QAgent"; source=q; record=vector,mean; interpolationmode=none);
        @statistic[be](title="BE used by the QAgent"; source=be; record=vector; interpolationmode=none);
        @statistic[eps](title="EPS used by the QAgent"; source=eps; record=vector; interpolationmode=none);

        @statistic[capSuccess](title="capSuccess"; source=capSuccess; record=mean,max,min; interpolationmode=none);
        @statistic[capFailed](title="capFailed"; source=capFailed; record=mean,max,min; interpolationmode=none);
        @statistic[overheardMsg](title="overheardMsg"; source=overheardMsg; record=mean,max,min; interpolationmode=none);


	@statistic[packetsPerCAP](title="Packets sent per CAP"; source=packetsPerCAP; record=min,mean,max,vector; interpolationmode=none);
        @statistic[failedCCAs](title="Failed CCA attempts per CAP"; source=failedCCAs; record=min,mean,max,vector; interpolationmode=none);
        @statistic[prrCAP](title="Packet reception ration per CAP"; source=prrCAP; record=min,mean,max; interpolationmode=none);
        @statistic[failedPacketsPerCAP](title="Failed packets per CAP"; source=failedPacketsPerCAP; record=min,mean,max; interpolationmode=none);
	@statistic[successPacketsCAP](title="Successfully sent packets per CAP"; source=successPacketsCAP; record=min,mean,max; interpolationmode=none);

        @class(::dsme::DSMEPlatform);
}
