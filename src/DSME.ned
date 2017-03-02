package inet_dsme;

import inet.linklayer.base.MACProtocolBase;
import inet.linklayer.contract.IMACProtocol;

//
// IEEE802.15.4e
// Deterministic and synchronous multi-channel extension
//
simple DSME extends MACProtocolBase like IMACProtocol
{
    parameters:
        int flipNode = default(0);
        int flipLink = default(0);
        int flipTime = default(10000);
        int flipOption = default(0);
        int evaluationTime = default(0);

        string address = default("auto"); // MAC address as hex string (12 hex digits), or
                                          // "auto". "auto" values will be replaced by
                                          // a generated MAC address in init stage 0. 
                               
        bool optimizations = default(false);
                                          
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

        // Maximum number GTS which a Device is allowed to allocate per destination
        // e.g. NumberOfGTSPerMultisuperframe / NumberOfNeighbors
        int maxNumberGTSAllocPerDevice = default(7);
        int maxNumberGTSAllocPerRequest = default(1); // also per superframe TODO ??
        int maxGTSIdleCount = default(0);	// 0 => never deallocate idle GTS
        
        int macDSMEGTSExpirationTime = default(7);

        string allocationScheme = default("next");	// ("random", "next")

        int numChannels = default(16);
        int commonChannel = default(11);
        int macPANId = default(10);
        
        string radioModule = default("^.radio");   // The path to the Radio module  //FIXME remove default value
        
       
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
        
        @signal[unicastDataSentDown](type=cPacket);
        @signal[broadcastDataSentDown](type=cPacket);
        @signal[commandSentDown](type=cPacket);
        @signal[beaconSentDown](type=cPacket);
        @signal[ackSentDown](type=cPacket);
        @signal[corruptedFrameReceived](type=cPacket);
        @signal[uncorruptedFrameReceived](type=cPacket);
        
        @statistic[unicastDataSentDown](title="unicast packet sent down of type DATA"; source=unicastDataSentDown; record=count,vector(packetBytes); interpolationmode=none);
        @statistic[broadDataSentDown](title="broadcast packet sent down of type DATA"; source=broadcastDataSentDown; record=count,vector(packetBytes); interpolationmode=none);
        @statistic[corruptedFrameReceived](title="corrupted frame received"; source=corruptedFrameReceived; record=count,vector(packetBytes); interpolationmode=none);
        @statistic[uncorruptedFrameReceived](title="uncorrupted frame received"; source=uncorruptedFrameReceived; record=count,vector(packetBytes); interpolationmode=none);
        
        @class(::dsme::DSMEPlatform);
}

