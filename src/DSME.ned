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

        bool macCapReduction = default(false);

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
        int futureLength = default(5);  
        xml staticSchedule = default(xml("<root/>")); 

        int macDSMEGTSExpirationTime = default(7);
        int macResponseWaitTime = default(32);

        int numChannels = default(16);
        int commonChannel = default(11);
        int scanDuration = default(6);
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

        // LQI 150 corresponds roughly to 20% PER
        int minBroadcastLQI = default(150);
        int minCoordinatorLQI = default(150);

        // @signal[unicastDataSentDown](type=cPacket);
        // @signal[broadcastDataSentDown](type=cPacket);
        // @signal[commandSentDown](type=cPacket);
        // @signal[beaconSentDown](type=cPacket);
        // @signal[ackSentDown](type=cPacket);
        // @signal[corruptedFrameReceived](type=cPacket);
        // @signal[uncorruptedFrameReceived](type=cPacket);
        @signal[GTSChange](type=long);

        @signal[statStartOfCFP](type=long);    
        @signal[statStartOfSF](type=long);     
        @signal[statStartOfMSF](type=long);    
        @signal[statRequestInitialized](type=long);
        @signal[statRequestBackoffs](type=long);
        @signal[statRequestSendSuccess](type=long);
        @signal[statRequestSendFailedNoAck](type=long);
        @signal[statRequestSendFailedChannelAccess](type=long);
        @signal[statRequestSendFailedTransactionOverflow](type=long);
        @signal[statResponseReceiveSuccess](type=long);
        @signal[statResponseReceiveFailedTimeout](type=long);
        @signal[statResponseReceiveFailedDenied](type=long);
        @signal[statNotifyInitialized](type=long);
        @signal[statNotifyBackoffs](type=long);
        @signal[statNotifySendSuccess](type=long);
        @signal[statNotifySendFailedChannelAccess](type=long);
        @signal[statNotifySendFailedTransactionOverflow](type=long);
        @signal[statDeallocationScheduler](type=long);
        @signal[statDeallocationInvalid](type=long);
        @signal[statDeallocationUnconfirmed](type=long);
        @signal[statDeallocationExpiration](type=long);
        @signal[statDeallocationRequestInitialized](type=long);
        @signal[statDeallocationRequestBackoffs](type=long);
        @signal[statDeallocationRequestSendSuccess](type=long);
        @signal[statDeallocationRequestSendFailedNoAck](type=long);
        @signal[statDeallocationRequestSendFailedChannelAccess](type=long);
        @signal[statDeallocationRequestSendFailedTransactionOverflow](type=long);
        @signal[statDeallocationResponseReceiveSuccess](type=long);
        @signal[statDeallocationResponseReceiveFailedTimeout](type=long);
        @signal[statDeallocationResponseReceiveFailedDenied](type=long);
        @signal[statDeallocationNotifyInitialized](type=long);
        @signal[statDeallocationNotifyBackoffs](type=long);
        @signal[statDeallocationNotifySendSuccess](type=long);
        @signal[statDeallocationNotifySendFailedChannelAccess](type=long);
        @signal[statDeallocationNotifySendFailedTransactionOverflow](type=long);
         
        @signal[GTSRequestsTotal](type=long);
        @signal[GTSRequestsSuccess](type=long);
        @signal[GTSRequestsFailed](type=long);
        @signal[GTSRequestsFailedNoAck](type=long);
        @signal[GTSRequestsFailedChannelAccess](type=long);
        @signal[GTSRequestsFailedTransactionOverflow](type=long);
        @signal[GTSRequestsFailedDenied](type=long);
        @signal[GTSRequestsFailedTimeout](type=long);
        @signal[GTSRequestsFailedQueue](type=long);
        @signal[GTSRequestsFailedDeallocated](type=long);

        // @statistic[unicastDataSentDown](title="unicast packet sent down of type DATA"; source=unicastDataSentDown; record=count; interpolationmode=none);
        // @statistic[broadDataSentDown](title="broadcast packet sent down of type DATA"; source=broadcastDataSentDown; record=count; interpolationmode=none);
        // @statistic[corruptedFrameReceived](title="corrupted frame received"; source=corruptedFrameReceived; record=count; interpolationmode=none);
        // @statistic[uncorruptedFrameReceived](title="uncorrupted frame received"; source=uncorruptedFrameReceived; record=count; interpolationmode=none);
        @statistic[GTSChange](title="GTS allocation or deallocation"; source=GTSChange; record=vector; interpolationmode=none);


        @statistic[statStartOfCFP](title="NAN"; source=statStartOfCFP; record=vector; interpolationmode=none);    
        @statistic[statStartOfSF](title="NAN"; source= statStartOfSF; record=vector; interpolationmode=none);     
        @statistic[statStartOfMSF](title="NAN"; source=statStartOfMSF ; record=vector; interpolationmode=none);    
        @statistic[statRequestInitialized](title="NAN"; source=statRequestInitialized ; record=vector; interpolationmode=none);
        @statistic[statRequestBackoffs](title="NAN"; source=statRequestBackoffs ; record=vector; interpolationmode=none);
        @statistic[statRequestSendSuccess](title="NAN"; source=statRequestSendSuccess ; record=vector; interpolationmode=none);
        @statistic[statRequestSendFailedNoAck](title="NAN"; source=statRequestSendFailedNoAck ; record=vector; interpolationmode=none);
        @statistic[statRequestSendFailedChannelAccess](title="NAN"; source=statRequestSendFailedChannelAccess ; record=vector; interpolationmode=none);
        @statistic[statRequestSendFailedTransactionOverflow](title="NAN"; source=statRequestSendFailedTransactionOverflow ; record=vector; interpolationmode=none);
        @statistic[statResponseReceiveSuccess](title="NAN"; source=statResponseReceiveSuccess ; record=vector; interpolationmode=none);
        @statistic[statResponseReceiveFailedTimeout](title="NAN"; source=statResponseReceiveFailedTimeout ; record=vector; interpolationmode=none);
        @statistic[statResponseReceiveFailedDenied](title="NAN"; source=statResponseReceiveFailedDenied ; record=vector; interpolationmode=none);
        @statistic[statNotifyInitialized](title="NAN"; source=statNotifyInitialized ; record=vector; interpolationmode=none);
        @statistic[statNotifyBackoffs](title="NAN"; source=statNotifyBackoffs ; record=vector; interpolationmode=none);
        @statistic[statNotifySendSuccess](title="NAN"; source=statNotifySendSuccess ; record=vector; interpolationmode=none);
        @statistic[statNotifySendFailedChannelAccess](title="NAN"; source=statNotifySendFailedChannelAccess ; record=vector; interpolationmode=none);
        @statistic[statNotifySendFailedTransactionOverflow](title="NAN"; source=statNotifySendFailedTransactionOverflow ; record=vector; interpolationmode=none);
        @statistic[statDeallocationScheduler](title="NAN"; source=statDeallocationScheduler ; record=vector; interpolationmode=none);
        @statistic[statDeallocationInvalid](title="NAN"; source=statDeallocationInvalid ; record=vector; interpolationmode=none);
        @statistic[statDeallocationUnconfirmed](title="NAN"; source=statDeallocationUnconfirmed ; record=vector; interpolationmode=none);
        @statistic[statDeallocationExpiration](title="NAN"; source=statDeallocationExpiration ; record=vector; interpolationmode=none);
        @statistic[statDeallocationRequestInitialized](title="NAN"; source=statDeallocationRequestInitialized ; record=vector; interpolationmode=none);
        @statistic[statDeallocationRequestBackoffs](title="NAN"; source=statDeallocationRequestBackoffs ; record=vector; interpolationmode=none);
        @statistic[statDeallocationRequestSendSuccess](title="NAN"; source=statDeallocationRequestSendSuccess ; record=vector; interpolationmode=none);
        @statistic[statDeallocationRequestSendFailedNoAck](title="NAN"; source=statDeallocationRequestSendFailedNoAck ; record=vector; interpolationmode=none);
        @statistic[statDeallocationRequestSendFailedChannelAccess](title="NAN"; source=statDeallocationRequestSendFailedChannelAccess ; record=vector; interpolationmode=none);
        @statistic[statDeallocationRequestSendFailedTransactionOverflow](title="NAN"; source=statDeallocationRequestSendFailedTransactionOverflow ; record=vector; interpolationmode=none);
        @statistic[statDeallocationResponseReceiveSuccess](title="NAN"; source=statDeallocationResponseReceiveSuccess ; record=vector; interpolationmode=none);
        @statistic[statDeallocationResponseReceiveFailedTimeout](title="NAN"; source=statDeallocationResponseReceiveFailedTimeout ; record=vector; interpolationmode=none);
        @statistic[statDeallocationResponseReceiveFailedDenied](title="NAN"; source=statDeallocationResponseReceiveFailedDenied ; record=vector; interpolationmode=none);
        @statistic[statDeallocationNotifyInitialized](title="NAN"; source=statDeallocationNotifyInitialized ; record=vector; interpolationmode=none);
        @statistic[statDeallocationNotifyBackoffs](title="NAN"; source=statDeallocationNotifyBackoffs ; record=vector; interpolationmode=none);
        @statistic[statDeallocationNotifySendSuccess](title="NAN"; source=statDeallocationNotifySendSuccess ; record=vector; interpolationmode=none);
        @statistic[statDeallocationNotifySendFailedChannelAccess](title="NAN"; source=statDeallocationNotifySendFailedChannelAccess ; record=vector; interpolationmode=none);
        @statistic[statDeallocationNotifySendFailedTransactionOverflow](title="NAN"; source=statDeallocationNotifySendFailedTransactionOverflow ; record=vector; interpolationmode=none);
 
        @statistic[GTSRequestsTotal](title="Requests per MSF"; source=GTSRequestsTotal; record=vector; interpolationmode=none);
        @statistic[GTSRequestsSuccess](title="Sucessful requests per MSF"; source=GTSRequestsSuccess; record=vector; interpolationmode=none);
        @statistic[GTSRequestsFailed](title="Failed requests per MSF"; source=GTSRequestsFailed; record=vector; interpolationmode=none);
        @statistic[GTSRequestsFailedNoAck](title="Failed requests per MSF"; source=GTSRequestsFailedNoAck; record=vector; interpolationmode=none);
        @statistic[GTSRequestsFailedChannelAccess](title="Failed requests per MSF"; source=GTSRequestsFailedChannelAccess; record=vector; interpolationmode=none);
        @statistic[GTSRequestsFailedDenied](title="Failed requests per MSF"; source=GTSRequestsFailedDenied; record=vector; interpolationmode=none);
        @statistic[GTSRequestsFailedTimeout](title="Failed requests per MSF"; source=GTSRequestsFailedTimeout; record=vector; interpolationmode=none);
        @statistic[GTSRequestsFailedTransactionOverflow](title="Failed requests per MSF"; source=GTSRequestsFailedTransactionOverflow; record=vector; interpolationmode=none);
        @statistic[GTSRequestsFailedQueue](title="Failed requests per MSF due to queue"; source=GTSRequestsFailedQueue; record=vector; interpolationmode=none);
        @statistic[GTSRequestsFailedDeallocated](title="Failed requests per MSF due to queue"; source=GTSRequestsFailedDeallocated; record=vector; interpolationmode=none);

        @class(::dsme::DSMEPlatform);
}

