#include "DSMEPlatform.h"

#include <iomanip>
#include <stdlib.h>

#include "./StaticSchedule.h"

#include <inet/common/ModuleAccess.h>
#include <inet/linklayer/common/InterfaceTag_m.h>
#include <inet/linklayer/common/MacAddressTag_m.h>
#include <inet/common/ProtocolTag_m.h>
#include <inet/common/ProtocolGroup.h>
#include <inet/common/packet/chunk/ByteCountChunk.h>
#include <inet/physicallayer/base/packetlevel/FlatRadioBase.h>
#include <inet/physicallayer/contract/packetlevel/SignalTag_m.h>
#include <inet/physicallayer/contract/packetlevel/IRadio.h>

#include "openDSME/dsmeLayer/DSMELayer.h"
#include "openDSME/dsmeLayer/messages/MACCommand.h"
#include "openDSME/dsmeAdaptionLayer/scheduling/PIDScheduling.h"
#include "openDSME/dsmeAdaptionLayer/scheduling/TPS.h"
#include "openDSME/dsmeAdaptionLayer/scheduling/StaticScheduling.h"
#include "openDSME/mac_services/pib/dsme_phy_constants.h"

// coverity[+kill]
void _simulation_will_terminate(void) {
    /* This is only used during static code analysis to signal that the simulation will terminate. */
    return;
}

using namespace inet;
using namespace inet::physicallayer;

Define_Module(dsme::DSMEPlatform);

namespace dsme {

simsignal_t DSMEPlatform::sig_broadcastDataSentDown;
simsignal_t DSMEPlatform::sig_unicastDataSentDown;
simsignal_t DSMEPlatform::sig_commandSentDown;
simsignal_t DSMEPlatform::sig_beaconSentDown;
simsignal_t DSMEPlatform::sig_ackSentDown;
simsignal_t DSMEPlatform::sig_uncorruptedFrameReceived;
simsignal_t DSMEPlatform::sig_corruptedFrameReceived;
simsignal_t DSMEPlatform::sig_gtsChange;
simsignal_t DSMEPlatform::sig_gackGTSChange;
simsignal_t DSMEPlatform::sig_queueLength;
simsignal_t DSMEPlatform::sig_GTSCount;
simsignal_t DSMEPlatform::sig_gackGTSCount;
simsignal_t DSMEPlatform::sig_numDroppedRetransmissionPackets;
simsignal_t DSMEPlatform::sig_numDroppedPackets;
simsignal_t DSMEPlatform::sig_acksInGack;
simsignal_t DSMEPlatform::sig_packetRetransmissionRate;
simsignal_t DSMEPlatform::sig_retransmissionQueueLength;
simsignal_t DSMEPlatform::sig_packetsTXPerSlot;
simsignal_t DSMEPlatform::sig_packetsRXPerSlot;
simsignal_t DSMEPlatform::sig_commandFrameDwellTime;
simsignal_t DSMEPlatform::sig_messagesInUse;
simsignal_t DSMEPlatform::sig_cfpAckDelay;
simsignal_t DSMEPlatform::sig_capAckDelay;


static void translateMacAddress(MacAddress& from, IEEE802154MacAddress& to) {
    // TODO only handles short address
    if(from.isBroadcast()) {
        to = IEEE802154MacAddress(IEEE802154MacAddress::SHORT_BROADCAST_ADDRESS);
    } else {
        to.setShortAddress((from.getAddressByte(4) << 8) | from.getAddressByte(5));
    }
}

static void translateMacAddress(IEEE802154MacAddress& from, MacAddress& to) {
    if (from == IEEE802154MacAddress(IEEE802154MacAddress::SHORT_BROADCAST_ADDRESS)) {
        to.setBroadcast();
    } else {
        to.setAddress("00 00 00 00 00 00");
        to.setAddressByte(4, from.getShortAddress() >> 8);
        to.setAddressByte(5, (from.getShortAddress() & 0xFF));
    }
}

static uint8_t PERtoLQI(double per) {
    // inverse function of the graph given in the ATmega256RFR2 datasheet
    double lqi = -22.2222 * log(0.00360656 * (-1 + (1 / (1 - per))));
    if(lqi > 255) {
        lqi = 255;
    } else if(lqi < 0) {
        lqi = 0;
    }
    return (uint8_t)(lqi + 0.5);
}

static std::string getErrorInfo(inet::Packet* packet) {
    auto errorRateInd = packet->getTag<inet::ErrorRateInd>();

    std::stringstream ss;
    ss << std::setprecision(3) << errorRateInd->getBitErrorRate() * 100.0 << "%, ";
    ss << errorRateInd->getPacketErrorRate() * 100.0 << "%, ";
    ss << "LQI " << (uint16_t) PERtoLQI(errorRateInd->getPacketErrorRate());
    return ss.str();
}

DSMEPlatform::DSMEPlatform()
    : phy_pib(),
      mac_pib(phy_pib),

      dsme(new DSMELayer()),
      mcps_sap(*dsme),
      mlme_sap(*dsme),
      dsmeAdaptionLayer(*dsme) {
    sig_broadcastDataSentDown = registerSignal("sig_broadcastDataSentDown");
    sig_unicastDataSentDown = registerSignal("sig_unicastDataSentDown");
    sig_commandSentDown = registerSignal("sig_commandSentDown");
    sig_beaconSentDown = registerSignal("sig_beaconSentDown");
    sig_ackSentDown = registerSignal("sig_ackSentDown");
    sig_uncorruptedFrameReceived = registerSignal("sig_uncorruptedFrameReceived");
    sig_corruptedFrameReceived = registerSignal("sig_corruptedFrameReceived");
    sig_gtsChange = registerSignal("sig_GTSChange");
    sig_gackGTSChange = registerSignal("sig_gackGTSChange");
    sig_queueLength = registerSignal("sig_queueLength");
    sig_GTSCount = registerSignal("sig_GTSCount");
    sig_gackGTSCount = registerSignal("sig_gackGTSCount");
    sig_numDroppedRetransmissionPackets = registerSignal("sig_numDroppedRetransmissionPackets");
    sig_numDroppedPackets = registerSignal("sig_numDroppedPackets");
    sig_packetRetransmissionRate = registerSignal("sig_packetRetransmissionRate");
    sig_retransmissionQueueLength = registerSignal("sig_retransmissionQueueLength");
    sig_packetsTXPerSlot = registerSignal("sig_packetsTXPerSlot");
    sig_packetsRXPerSlot = registerSignal("sig_packetsRXPerSlot");
    sig_commandFrameDwellTime = registerSignal("sig_commandFrameDwellTime");
    sig_messagesInUse = registerSignal("sig_messagesInUse");
    sig_acksInGack = registerSignal("sig_acksInGack");
    sig_cfpAckDelay = registerSignal("sig_cfpAckDelay");
    sig_capAckDelay = registerSignal("sig_capAckDelay");
}

DSMEPlatform::~DSMEPlatform() {
    delete dsme;
    delete scheduling;

    cancelAndDelete(ccaTimer);
    cancelAndDelete(cfpTimer);
    cancelAndDelete(timer);
}

/****** INET ******/

void DSMEPlatform::configureInterfaceEntry() {
    InterfaceEntry *e = getContainingNicModule(this);

    // data rate
    e->setDatarate(bitrate);

    // generate a link-layer address to be used as interface token for IPv6
    e->setMacAddress(addr);
    e->setInterfaceToken(addr.formInterfaceIdentifier());

    // capabilities
    e->setMtu(par("mtu").intValue());
    e->setMulticast(true);
    e->setBroadcast(true);
}

/****** OMNeT++ ******/

void DSMEPlatform::initialize(int stage) {
    MacProtocolBase::initialize(stage);

    if(stage == INITSTAGE_LOCAL) {
        /* 11 <= phyCurrentChannel <= 26 for 2450 MHz band DSSS */
        channelList_t DSSS2450_channels(16);
        for(uint8_t i = 0; i < 16; i++) {
            DSSS2450_channels[i] = 11 + i;
        }
        this->phy_pib.setDSSS2450ChannelPage(DSSS2450_channels);

        this->dsme->setPHY_PIB(&(this->phy_pib));
        this->dsme->setMAC_PIB(&(this->mac_pib));
        this->dsme->setMCPS(&(this->mcps_sap));
        this->dsme->setMLME(&(this->mlme_sap));

        channelList_t scanChannels;
        scanChannels.add(par("commonChannel"));
        const char* schedulingSelection = par("scheduling");
        if(!strcmp(schedulingSelection, "PID")) {
            scheduling = new PIDScheduling(this->dsmeAdaptionLayer);
        }
        else if(!strcmp(schedulingSelection, "TPS")) {
            TPS* tps = new TPS(this->dsmeAdaptionLayer);
            tps->setAlpha(par("TPSalpha").doubleValue());
            tps->setMinFreshness(this->mac_pib.macDSMEGTSExpirationTime);
            tps->setUseHysteresis(par("useHysteresis").boolValue());
            tps->setUseMultiplePacketsPerGTS(par("sendMultiplePacketsPerGTS").boolValue());
            scheduling = tps;
        }
        else if(!strcmp(schedulingSelection, "STATIC")) {
            scheduling = new StaticScheduling(this->dsmeAdaptionLayer);
        }
        else {
            ASSERT(false);
        }

        uint8_t scanDuration = par("scanDuration").intValue();

        this->dsmeAdaptionLayer.initialize(scanChannels,scanDuration,scheduling);

        /* Initialize Address */
        IEEE802154MacAddress address;
        const char* addrstr = par("address");

        if(!strcmp(addrstr, "auto")) {
            // assign automatic address
            addr = MacAddress::generateAutoAddress();

            // change module parameter from "auto" to concrete address
            par("address").setStringValue(addr.str().c_str());
        } else {
            addr.setAddress(addrstr);
        }

        translateMacAddress(addr, this->mac_pib.macExtendedAddress);
        //registerInterface();

        /* Find Radio Module */
        cModule* radioModule = getModuleFromPar<cModule>(par("radioModule"), this);
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radioModule->subscribe(IRadio::receptionStateChangedSignal, this);
        radio = check_and_cast<IRadio*>(radioModule);

        symbolDuration = SimTime(16, SIMTIME_US);
        timer = new cMessage();
        cfpTimer = new cMessage();
        ccaTimer = new cMessage();

        // check parameters for consistency
        // aTurnaroundTimeSymbols should match (be equal or bigger) the RX to TX
        // switching time of the radio
        if(radioModule->hasPar("timeRXToTX")) {
            simtime_t rxToTx = radioModule->par("timeRXToTX").doubleValue();
            if(rxToTx > aTurnaroundTime) {
                throw cRuntimeError(
                    "Parameter \"aTurnaroundTimeSymbols\" (%f) does not match"
                    " the radios RX to TX switching time (%f)! It"
                    " should be equal or bigger",
                    SIMTIME_DBL(aTurnaroundTime * symbolDuration), SIMTIME_DBL(rxToTx));
            }
        }

        this->mac_pib.macShortAddress = this->mac_pib.macExtendedAddress.getShortAddress();

        if(par("isPANCoordinator")) {
            this->mac_pib.macPANId = par("macPANId");
        }

        this->mac_pib.macCapReduction = par("macCapReduction");

        this->mac_pib.macAssociatedPANCoord = par("isPANCoordinator");
        this->mac_pib.macBeaconOrder = par("beaconOrder");
        this->mac_pib.macSuperframeOrder = par("superframeOrder");
        this->mac_pib.macMultiSuperframeOrder = par("multiSuperframeOrder");

        this->mac_pib.macMinBE = par("macMinBE");
        this->mac_pib.macMaxBE = par("macMaxBE");
        this->mac_pib.macMaxCSMABackoffs = par("macMaxCSMABackoffs");
        this->mac_pib.macMaxFrameRetries = par("macMaxFrameRetries");

        this->mac_pib.macDSMEGTSExpirationTime = par("macDSMEGTSExpirationTime");
        this->mac_pib.macResponseWaitTime = par("macResponseWaitTime");

        this->mac_pib.macIsPANCoord = par("isPANCoordinator");

        this->mac_pib.macIsCoord = (par("isCoordinator") || this->mac_pib.macIsPANCoord);

        this->phy_pib.phyCurrentChannel = par("commonChannel");

        this->dsmeAdaptionLayer.setIndicationCallback(DELEGATE(&DSMEPlatform::handleIndicationFromMCPS, *this));
        this->dsmeAdaptionLayer.setConfirmCallback(DELEGATE(&DSMEPlatform::handleConfirmFromMCPS, *this));

        this->minBroadcastLQI = par("minBroadcastLQI");
        this->minCoordinatorLQI = par("minCoordinatorLQI");

        this->dsme->initialize(this);

        this->dsme->getMessageDispatcher().setSendMultiplePacketsPerGTS(par("sendMultiplePacketsPerGTS").boolValue());
        this->gackEnabled = par("gackEnabled").boolValue(); //maybe move to MessageDispatcher
        this->gackBeaconEnabled = par("gackBeaconEnabled").boolValue();
        this->gackCAPEnabled = par("gackCAPEnabled").boolValue();
        this->gackGTSEnabled = par("gackGTSEnabled").boolValue();
        this->mac_pib.macGroupAckOrder = par("groupAckOrder");

        // static schedules need to be initialized after dsmeLayer
        if(!strcmp(schedulingSelection, "STATIC")) {
            cXMLElement *xmlFile = par("staticSchedule");
            std::vector<StaticSlot> slots = StaticSchedule::loadSchedule(xmlFile, this->mac_pib.macShortAddress);
            for(auto &slot : slots) {
                static_cast<StaticScheduling*>(scheduling)->allocateGTS(slot.superframeID, slot.slotID, slot.channelID, (Direction)slot.direction, slot.address);
            }
        }
    } else if(stage == INITSTAGE_LINK_LAYER) {
        radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
        dsme->start();
        dsmeAdaptionLayer.startAssociation();

        updateVisual();
        cModule* mobilityModule = this->getParentModule()->getParentModule()->getSubmodule("mobility");
        IMobility* mobility = dynamic_cast<IMobility*>(mobilityModule);
        if(mobility) {
            Coord currentPosition = mobility->getCurrentPosition();
            LOG_INFO("POSITION: x=" << currentPosition.x << ", y=" << currentPosition.y);
        }
    }
}

void DSMEPlatform::finish() {
    recordScalar("numUpperPacketsForCAP", dsme->getMessageDispatcher().getNumUpperPacketsForCAP());
    recordScalar("numUpperPacketsForGTS", dsme->getMessageDispatcher().getNumUpperPacketsForGTS());
    recordScalar("numUpperPacketsDroppedFullQueue", dsme->getMessageDispatcher().getNumUpperPacketsDroppedFullQueue());
    recordScalar("macChannelOffset", dsme->getMAC_PIB().macChannelOffset);
    recordScalar("numUnusedTxGTS", dsme->getMessageDispatcher().getNumUnusedTxGTS());
}

void DSMEPlatform::handleLowerPacket(inet::Packet* packet) {
    if(!this->transceiverIsOn) {

        DSMEMessage* message = getLoadedMessage(packet);
        message->getHeader().decapsulateFrom(message);

        LOG_DEBUG("Missed frame " << packet->str() << "(" << getSequenceChartInfo(message, false) << ") [" << getErrorInfo(packet) << "]");

        releaseMessage(message);
        return;
    }

    if(packet->hasBitError()) {
        DSMEMessage* message = getLoadedMessage(packet);
        message->getHeader().decapsulateFrom(message);


        LOG_DEBUG("Received corrupted frame " << packet->str() << "(" << getSequenceChartInfo(message, false) << ")");
        emit(sig_corruptedFrameReceived, packet);

        releaseMessage(message);
        return;
    }

    emit(sig_uncorruptedFrameReceived, packet);

    auto fcs = packet->removeAtBack(B(2)); // FCS is not explicitly handled -> hasBitError is used instead
    DSMEMessage* message = getLoadedMessage(packet);
    message->getHeader().decapsulateFrom(message);

//    if(message->getHeader().getFrameType() == IEEE802154eMACHeader::FrameType::DATA){
//        auto chunk = packet->peekDataAsBytes();
//        auto bytes = chunk->getBytes();
//
//        for(uint8_t i = 0; i < bytes.size(); i++){
//            bytes[i] -= dsme->getMAC_PIB().preshared_secret;
//        }
//
//        auto newChunk = makeShared<BytesChunk>();
//        newChunk->(bytes);
//        packet->eraseAll();
//        packet->insertAtFront(newChunk);
//    }

    // Get LQI
    auto errorRateInd = packet->getTag<inet::ErrorRateInd>();
    message->setLQI(PERtoLQI(errorRateInd->getPacketErrorRate()));

    LOG_DEBUG("Received valid frame     " << packet->str() << "(" << getSequenceChartInfo(message, false) << ") [" << getErrorInfo(packet) << "]");

    // Preamble (4) | SFD (1) | PHY Hdr (1) | MAC Payload | FCS (2)
    message->startOfFrameDelimiterSymbolCounter = getSymbolCounter() - message->getTotalSymbols() + 2 * 4 // Preamble
                                                  + 2 * 1;                                                // SFD

    dsme->getAckLayer().receive(message);
}

void DSMEPlatform::handleUpperPacket(inet::Packet* packet) {
    if (auto *tag = packet->findTag<inet::PacketProtocolTag>()) {
        /* 'Smuggle' protocol information across lower layers via par() */
        auto protocol = tag->getProtocol();
        auto protocolId = inet::ProtocolGroup::ethertype.getProtocolNumber(protocol);
        packet->addPar("networkProtocol").setLongValue(protocolId);
    }

    LOG_INFO_PREFIX;
    LOG_INFO_PURE("Upper layer requests to send a message to ");


    auto message = getLoadedMessage(packet);

//    if(message->getHeader().getFrameType() == IEEE802154eMACHeader::FrameType::DATA){
//        auto chunk = packet->peekAllAsBytes();
//        auto bytes = chunk->getBytes();
//
//        for(uint8_t i = 0; i < bytes.size(); i++){
//            //bytes[i] = (bytes[i] + dsme->getMAC_PIB().preshared_secret) %  256;
//        }
//
//        auto newChunk = makeShared<BytesChunk>();
//        newChunk->setBytes(bytes);
//        packet->eraseAll();
//        packet->insertAtFront(newChunk);
//    }

    auto& header = message->getHeader();
    header.setFrameType(IEEE802154eMACHeader::DATA);
    header.setSrcAddr(this->mac_pib.macExtendedAddress);

    auto destinationAddress = packet->getTag<inet::MacAddressReq>()->getDestAddress();
    if(destinationAddress.isMulticast()) {
        // handle multicast as broadcast (TODO ok?)
        destinationAddress = MacAddress::BROADCAST_ADDRESS;
        LOG_INFO_PURE("Broadcast");
    } else {
        LOG_INFO_PURE((destinationAddress.getInt() & 0xFFFF));
    }
    LOG_INFO_PURE("." << std::endl);

    translateMacAddress(destinationAddress, message->getHeader().getDestAddr());

    message->firstTry = true;
    this->dsmeAdaptionLayer.sendMessage(message);
}

void DSMEPlatform::handleSelfMessage(cMessage* msg) {
    if(msg == timer) {
        dsme->getEventDispatcher().timerInterrupt();
    } else if(msg == ccaTimer) {
        bool isIdle = (radio->getReceptionState() == IRadio::RECEPTION_STATE_IDLE) && channelInactive;
        LOG_DEBUG("CCA isIdle " << isIdle);
        dsme->dispatchCCAResult(isIdle);
    } else if(msg == cfpTimer) {
        dsme->handleStartOfCFP();
    } else if(strcmp(msg->getName(), "acktimer") == 0) {
        // LOG_INFO("send ACK")
        bool result = prepareSendingCopy((DSMEMessage*)msg->getParList().get(0), txEndCallback);
        ASSERT(result);
        result = sendNow();
        ASSERT(result);
        // the ACK Message itself will be deleted inside the AckLayer
        delete msg;
    } else if(strcmp(msg->getName(), "receive") == 0) {
        LOG_INFO_PURE("switch to receive");
        radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
        delete msg;
    } else if(strcmp(msg->getName(), "transceiveroff") == 0) {
        turnTransceiverOff();
    } else {
        MacProtocolBase::handleSelfMessage(msg);
    }
}




void DSMEPlatform::receiveSignal(cComponent *source, simsignal_t signalID, intval_t l, cObject *details) {
    Enter_Method_Silent();
    if(signalID == IRadio::transmissionStateChangedSignal) {
        IRadio::TransmissionState newRadioTransmissionState = static_cast<IRadio::TransmissionState>(l);
        if(transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE) {
            // LOG_INFO("Transmission ready")
            scheduleAt(simTime(), new cMessage("receive"));
            txEndCallback(true); // TODO could it be false?
        }
        transmissionState = newRadioTransmissionState;
    } else if(signalID == IRadio::radioModeChangedSignal) {
        IRadio::RadioMode newRadioMode = static_cast<IRadio::RadioMode>(l);
        if(newRadioMode == IRadio::RADIO_MODE_TRANSMITTER) {
            // LOG_INFO("switched to transmit")
            if(pendingSendRequest) {
                // LOG_INFO("sendDown after tx switch")

                sendDown(pendingTxPacket);
                pendingTxPacket = nullptr; // now owned by lower layer
                pendingSendRequest = false;
            }
        } else if(newRadioMode == IRadio::RADIO_MODE_RECEIVER) {
            LOG_INFO("switched to receive 2");
        }
    } else if(signalID == IRadio::receptionStateChangedSignal) {
        // LOG_INFO("receptionStateChanged to " << (uint16_t)value);
        channelInactive = false;
    }
}

/****** IDSMERadio ******/

bool DSMEPlatform::setChannelNumber(uint8_t k) {
    DSME_ASSERT(this->transceiverIsOn);
    DSME_ASSERT(k >= 11 && k <= 26);

    auto r = check_and_cast<NarrowbandRadioBase*>(radio);
    r->setCenterFrequency(MHz(2405 + 5 * (k-11)));
    currentChannel = k;
    return true;
}

uint8_t DSMEPlatform::getChannelNumber() {
    DSME_ASSERT(currentChannel >= 11 && currentChannel <= 26);
    return currentChannel;
}

static std::string extract_type(std::string s) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, '|')) {
        out.push_back(item);
    }
    return out[3];
}

bool DSMEPlatform::prepareSendingCopy(IDSMEMessage* msg, Delegate<void(bool)> txEndCallback) {
    if(msg == nullptr) {
        return false;
    }

    DSMEMessage* message = check_and_cast<DSMEMessage*>(msg);

    std::string printable_info = getSequenceChartInfo(msg, true);
    LOG_DEBUG(printable_info);

    LOG_INFO("sendCopyNow");

    this->txEndCallback = txEndCallback;
    auto packet = message->getSendableCopy();

    const auto& fcs = makeShared<ByteCountChunk>(B(2));
    packet->insertAtBack(fcs);

    packet->addTagIfAbsent<inet::PacketProtocolTag>()->setProtocol(&Protocol::ieee802154);
    if(!msg->getReceivedViaMCPS()) { // do not rewrite upper layer packet names
        DSME_ASSERT(strlen(packet->getName()) == 0);
        packet->setName(extract_type(printable_info).c_str());
    }

    switch(msg->getHeader().getFrameType()) {
        case IEEE802154eMACHeader::BEACON:
            emit(sig_beaconSentDown, packet);
            break;
        case IEEE802154eMACHeader::DATA:
            if(msg->getHeader().getDestAddr().isBroadcast()) {
                emit(sig_broadcastDataSentDown, packet);
            } else {
                emit(sig_unicastDataSentDown, packet);
            }
            break;
        case IEEE802154eMACHeader::ACKNOWLEDGEMENT:
            emit(sig_ackSentDown, packet);
            break;
        case IEEE802154eMACHeader::COMMAND: {
            CommandFrameIdentifier cmd = (CommandFrameIdentifier)message->packet->peekDataAsBytes()->getByte(0);
            if(cmd == CommandFrameIdentifier::DSME_GTS_REQUEST || cmd == CommandFrameIdentifier::DSME_GTS_REPLY || cmd == CommandFrameIdentifier::DSME_GTS_NOTIFY) {
                LOG_INFO("Command frame transmitted with creation time " << (long)msg->getHeader().getCreationTime() << " and dwell time " << (long)(getSymbolCounter() - msg->getHeader().getCreationTime()));
                emit(sig_commandFrameDwellTime, getSymbolCounter() - msg->getHeader().getCreationTime());
                DSME_ASSERT(msg->getHeader().getCreationTime() > 0);
            }
            emit(sig_commandSentDown, packet);
            break; }
        default:
            DSME_ASSERT(false);
    }

    DSME_ASSERT(pendingTxPacket == nullptr);

    pendingTxPacket = packet;

    if(radio->getRadioMode() != IRadio::RADIO_MODE_TRANSMITTER) {
        //JND:
        //DSME_ASSERT(msg->getHeader().getFrameType() != IEEE802154eMACHeader::ACKNOWLEDGEMENT); // switching is handled by ACK routine
        radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    }

    return true;
}

bool DSMEPlatform::sendNow() {
    DSME_ASSERT(transceiverIsOn);
    DSME_ASSERT(pendingTxPacket);
    DSME_ASSERT(!pendingSendRequest);

    if(this->radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER) {
        /*inet::Packet *p = pendingTxPacket->dup();
        p->removeAtBack(B(2));
        DSMEMessage* msg = getLoadedMessage(p);
        msg->getHeader().decapsulateFrom(msg);

        if(msg->getHeader().getFrameType() == IEEE802154eMACHeader::COMMAND) {
            CommandFrameIdentifier cmd = (CommandFrameIdentifier)msg->packet->peekDataAsBytes()->getByte(0);
            if(cmd == CommandFrameIdentifier::DSME_GTS_REQUEST || cmd == CommandFrameIdentifier::DSME_GTS_REPLY || cmd == CommandFrameIdentifier::DSME_GTS_NOTIFY) {
                LOG_INFO("Command frame transmitted with creation time " << (long)msg->getHeader().getCreationTime() << " and dwell time " << (long)(getSymbolCounter() - msg->getHeader().getCreationTime()));
                emit(commandFrameDwellTime, getSymbolCounter() - msg->getHeader().getCreationTime());
                DSME_ASSERT(msg->getHeader().getCreationTime() > 0);
            }
        }
        releaseMessage(msg); */

        // can be sent direct
        sendDown(pendingTxPacket);
        pendingTxPacket = nullptr;
    } else {
        pendingSendRequest = true;
    }
    // otherwise receiveSignal will be called eventually
    return true;
}

void DSMEPlatform::abortPreparedTransmission() {
    DSME_ASSERT(!pendingSendRequest);
    DSME_ASSERT(pendingTxPacket);
    delete pendingTxPacket;
    pendingTxPacket = nullptr;
    scheduleAt(simTime(), new cMessage("receive"));
}

bool DSMEPlatform::sendDelayedAck(IDSMEMessage* ackMsg, IDSMEMessage* receivedMsg, Delegate<void(bool)> txEndCallback) {
    DSME_ASSERT(this->transceiverIsOn);
    DSMEMessage* dsmeAckMsg = dynamic_cast<DSMEMessage*>(ackMsg);
    DSME_ASSERT(dsmeAckMsg != nullptr);

    cMessage* acktimer = new cMessage("acktimer");
    acktimer->getParList().setTakeOwnership(false); // ackMsg is still owned by the AckLayer
    acktimer->getParList().addAt(0, dsmeAckMsg);

    this->txEndCallback = txEndCallback;

    // Preamble (4) | SFD (1) | PHY Hdr (1) | MAC Payload | FCS (2)
    uint32_t endOfReception = receivedMsg->getStartOfFrameDelimiterSymbolCounter() + receivedMsg->getTotalSymbols() - 2 * 4 // Preamble
                              - 2 * 1;                                                                                      // SFD
    uint32_t ackTime = endOfReception + aTurnaroundTime;
    uint32_t now = getSymbolCounter();
    uint32_t diff = ackTime - now;

    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);

    scheduleAt(simTime() + diff * symbolDuration, acktimer);
    return true;
}

void DSMEPlatform::setReceiveDelegate(receive_delegate_t receiveDelegate) {
    this->receiveFromAckLayerDelegate = receiveDelegate;
}

bool DSMEPlatform::startCCA() {
    DSME_ASSERT(this->transceiverIsOn);

    this->channelInactive = true;
    scheduleAt(simTime() + 8 * this->symbolDuration, this->ccaTimer);
    return true;
}

void DSMEPlatform::turnTransceiverOn() {
    if(!this->transceiverIsOn) {
	this->transceiverIsOn = true;
	LOG_INFO("turnedON");
    	this->radio->setRadioMode(inet::physicallayer::IRadio::RADIO_MODE_RECEIVER);
    }
}

void DSMEPlatform::turnTransceiverOff(){
    this->transceiverIsOn = false;
    this->radio->setRadioMode(inet::physicallayer::IRadio::RADIO_MODE_OFF);
}

void DSMEPlatform::delayedTurnTransceiverOff(){
    scheduleAt(simTime(), new cMessage("transceiveroff"));
}

bool DSMEPlatform::isGackEnabled(){
    return gackEnabled;
}

bool DSMEPlatform::isGackBeaconEnabled() {
    return gackBeaconEnabled;
}

bool DSMEPlatform::isGackCAPEnabled(){
    return gackCAPEnabled;
}

bool DSMEPlatform::isGackGTSEnabled(){
    return gackGTSEnabled;
}

/****** IDSMEPlatform ******/

bool DSMEPlatform::isReceptionFromAckLayerPossible() {
    return true;
}

void DSMEPlatform::handleReceivedMessageFromAckLayer(IDSMEMessage* message) {
    DSME_ASSERT(receiveFromAckLayerDelegate);
    receiveFromAckLayerDelegate(message);
}

DSMEMessage* DSMEPlatform::getEmptyMessage() {
    messagesInUse++;
    signalMessagesInUse(messagesInUse);
    DSME_ASSERT(messagesInUse <= MSG_POOL_SIZE); // TODO should return nullptr (and check everywhere!!)
    auto msg = new DSMEMessage();
    msg->receivedViaMCPS = false;
    signalNewMsg(msg);
    return msg;
}

DSMEMessage* DSMEPlatform::getLoadedMessage(inet::Packet* packet) {
    messagesInUse++;
    signalMessagesInUse(messagesInUse);
    DSME_ASSERT(messagesInUse <= MSG_POOL_SIZE); // TODO
    auto msg = new DSMEMessage(packet);
    msg->receivedViaMCPS = false;
    signalNewMsg(msg);
    return msg;
}

void DSMEPlatform::releaseMessage(IDSMEMessage* msg) {
    DSME_ASSERT(messagesInUse > 0);
    DSME_ASSERT(msg != nullptr);
    messagesInUse--;
    signalMessagesInUse(messagesInUse);

#if 1
    DSMEMessage* dsmeMsg = dynamic_cast<DSMEMessage*>(msg);
    DSME_ASSERT(dsmeMsg != nullptr);
    msgsActive.erase(msgMap[dsmeMsg]);
#endif

    delete msg;
}

void DSMEPlatform::startTimer(uint32_t symbolCounterValue) {
    SimTime time = symbolCounterValue * symbolDuration;
    if(timer->isScheduled()) {
        cancelEvent(timer);
    }
    if(simTime() <= time) {
        scheduleAt(time, timer);
    }
}

uint32_t DSMEPlatform::getSymbolCounter() {
    return simTime() / symbolDuration;
}

uint16_t DSMEPlatform::getRandom() {
    return intrand(UINT16_MAX);
}

void DSMEPlatform::updateVisual() {
    std::stringstream s;
    s << this->mac_pib.macShortAddress;

    if(this->mac_pib.macIsCoord) {
        s << " C";
    }
    if(this->mac_pib.macAssociatedPANCoord) {
        s << " A";
    }

    cModule* host = findContainingNode(this);
    while(host->getParentModule() && host->getParentModule()->getId() != 1) {
        host = host->getParentModule();
    }
    cDisplayString& displayString = host->getDisplayString();
    displayString.setTagArg("t", 0, s.str().c_str());
}

void DSMEPlatform::scheduleStartOfCFP() {
    scheduleAt(simTime(), cfpTimer);
}

uint8_t DSMEPlatform::getMinCoordinatorLQI() {
    return minCoordinatorLQI;
}

void DSMEPlatform::handleIndicationFromMCPS(IDSMEMessage* msg) {
    if(msg->getHeader().getDestAddr().isBroadcast() && msg->getLQI() < minBroadcastLQI) {
        releaseMessage(msg);
        return;
    }

    DSMEMessage* dsmeMessage = check_and_cast<DSMEMessage*>(msg);

    auto packet = dsmeMessage->decapsulatePacket();

    inet::MacAddress address;
    translateMacAddress(dsmeMessage->getHeader().getSrcAddr(), address);
    packet->addTagIfAbsent<MacAddressInd>()->setSrcAddress(address);
    packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());

    releaseMessage(msg);

    if (packet->hasPar("networkProtocol")) {
        /* Reattach protocol information from par() */
        auto protocolId = packet->par("networkProtocol").longValue();
        auto protocol = inet::ProtocolGroup::ethertype.getProtocol(protocolId);
        packet->addTagIfAbsent<inet::PacketProtocolTag>()->setProtocol(protocol);
        packet->addTagIfAbsent<inet::DispatchProtocolReq>()->setProtocol(protocol);
    }

    sendUp(packet);
}

void DSMEPlatform::handleConfirmFromMCPS(IDSMEMessage* msg, DataStatus::Data_Status status) {
    releaseMessage(msg);
}

void DSMEPlatform::signalNewMsg(DSMEMessage* msg) {
#if 0
    LOG_INFO_PREFIX;
    LOG_INFO_PURE(msgId << " - ");
    for(auto i : msgsActive) {
        LOG_INFO_PURE(i << " ");
    }
    LOG_INFO_PURE(cometos::endl);
#endif

    msgMap[msg] = msgId;
    msgsActive.insert(msgId);
    msgId++;
}

std::string DSMEPlatform::getDSMEManagement(uint8_t management, DSMESABSpecification& subBlock, CommandFrameIdentifier cmd) {
    std::stringstream ss;

    uint8_t numChannels = this->dsmeAdaptionLayer.getMAC_PIB().helper.getNumChannels();

    ss << " ";
    uint8_t type = management & 0x7;
    switch((ManagementType)type) {
        case DEALLOCATION:
            ss << "DEALLOCATION";
            break;
        case ALLOCATION:
            ss << "ALLOCATION";
            break;
        case DUPLICATED_ALLOCATION_NOTIFICATION:
            ss << "DUPLICATED-ALLOCATION-NOTIFICATION";
            break;
        case REDUCE:
            ss << "REDUCE";
            break;
        case RESTART:
            ss << "RESTART";
            break;
        case EXPIRATION:
            ss << "EXPIRATION";
            break;
        default:
            ss << (uint16_t)management;
    }

    if(subBlock.getSubBlock().count(true) == 1) {
        for(DSMESABSpecification::SABSubBlock::iterator it = subBlock.getSubBlock().beginSetBits(); it != subBlock.getSubBlock().endSetBits(); it++) {
            // this calculation assumes there is always exactly one superframe in the subblock
            GTS gts(subBlock.getSubBlockIndex(), (*it) / numChannels, (*it) % numChannels);

            ss << " " << gts.slotID << " " << gts.superframeID << " " << (uint16_t)gts.channel;
        }
    }

    return ss.str();
}

std::string DSMEPlatform::getSequenceChartInfo(IDSMEMessage* msg, bool outgoing) {
    DSMEMessage* dsmeMsg = dynamic_cast<DSMEMessage*>(msg);
    DSME_ASSERT(dsmeMsg != nullptr);

    std::stringstream ss;

    IEEE802154eMACHeader& header = msg->getHeader();

    if(outgoing) {
        ss << (uint16_t)header.getDestAddr().getShortAddress() << "|";
    } else {
        ss << (uint16_t)header.getSrcAddr().getShortAddress() << "|";
    }

    ss << (uint16_t)header.hasSequenceNumber() << "|";

    ss << (uint16_t)header.getSequenceNumber() << "|";

    switch(header.getFrameType()) {
        case IEEE802154eMACHeader::BEACON:
            ss << "BEACON";
            break;
        case IEEE802154eMACHeader::DATA:
            ss << "DATA";
            break;
        case IEEE802154eMACHeader::ACKNOWLEDGEMENT:
            ss << "ACK";
            break;
        case IEEE802154eMACHeader::COMMAND: {
            //uint8_t cmd = dsmeMsg->frame->getData()[0];
            uint8_t cmd = dsmeMsg->packet->peekDataAsBytes()->getByte(0);

            switch((CommandFrameIdentifier)cmd) {
                case ASSOCIATION_REQUEST:
                    ss << "ASSOCIATION-REQUEST";
                    break;
                case ASSOCIATION_RESPONSE:
                    ss << "ASSOCIATION-RESPONSE";
                    break;
                case DISASSOCIATION_NOTIFICATION:
                    ss << "DISASSOCIATION-NOTIFICATION";
                    break;
                case DATA_REQUEST:
                    ss << "DATA-REQUEST";
                    break;
                case BEACON_REQUEST:
                    ss << "BEACON-REQUEST";
                    break;
                case DSME_ASSOCIATION_REQUEST:
                    ss << "DSME-ASSOCIATION-REQUEST";
                    break;
                case DSME_ASSOCIATION_RESPONSE:
                    ss << "DSME-ASSOCIATION-RESPONSE";
                    break;
                case DSME_BEACON_ALLOCATION_NOTIFICATION:
                    ss << "DSME-BEACON-ALLOCATION-NOTIFICATION";
                    break;
                case DSME_BEACON_COLLISION_NOTIFICATION:
                    ss << "DSME-BEACON-COLLISION-NOTIFICATION";
                    break;
                case DSME_GTS_REQUEST:
                case DSME_GTS_REPLY:
                case DSME_GTS_NOTIFY: {
                    DSMEMessage* m = getLoadedMessage(dsmeMsg->getSendableCopy());
                    m->getHeader().decapsulateFrom(m);

                    MACCommand cmdd;
                    cmdd.decapsulateFrom(m);
                    GTSManagement man;
                    man.decapsulateFrom(m);

                    switch(cmdd.getCmdId()) {
                        case DSME_GTS_REQUEST: {
                            ss << "DSME-GTS-REQUEST";
                            GTSRequestCmd req;
                            req.decapsulateFrom(m);
                            //ss << getDSMEManagement(dsmeMsg->frame->getData()[1], req.getSABSpec(), cmdd.getCmdId());
                            ss << getDSMEManagement(dsmeMsg->packet->peekDataAsBytes()->getByte(1), req.getSABSpec(), cmdd.getCmdId());
                            break;
                        }
                        case DSME_GTS_REPLY: {
                            ss << "DSME-GTS-REPLY";
                            GTSReplyNotifyCmd reply;
                            reply.decapsulateFrom(m);
                            //ss << getDSMEManagement(dsmeMsg->frame->getData()[1], reply.getSABSpec(), cmdd.getCmdId());
                            ss << getDSMEManagement(dsmeMsg->packet->peekDataAsBytes()->getByte(1), reply.getSABSpec(), cmdd.getCmdId());
                            break;
                        }
                        case DSME_GTS_NOTIFY: {
                            ss << "DSME-GTS-NOTIFY";
                            GTSReplyNotifyCmd notify;
                            notify.decapsulateFrom(m);
                            //ss << getDSMEManagement(dsmeMsg->frame->getData()[1], notify.getSABSpec(), cmdd.getCmdId());
                            ss << getDSMEManagement(dsmeMsg->packet->peekDataAsBytes()->getByte(1), notify.getSABSpec(), cmdd.getCmdId());
                            break;
                        }
                        default:
                            break;
                    }

                    this->releaseMessage(m);

                    break;
                }
                default:
                    break;
            }
            break;
        }
        default:
            ss << "UNKNOWN";
            break;
    }

    ss << "|" << msg->getTotalSymbols();

    return ss.str();
}

void DSMEPlatform::signalGTSChange(bool deallocation, IEEE802154MacAddress counterpart, bool gackGTS) {
    if(gackGTS){
        if(deallocation) gackGTSSlots--;
        else gackGTSSlots++;
        emit(sig_gackGTSChange, gackGTSSlots);
    }else{
        if(deallocation) slots--;
        else slots++;
        emit(sig_gtsChange, slots);
    }
}

void DSMEPlatform::signalGTSCount(uint32_t value) {
    emit(sig_GTSCount, value);
}

void DSMEPlatform::signalGackGTSCount(uint32_t value) {
    emit(sig_gackGTSCount, value);
}

void DSMEPlatform::signalQueueLength(uint32_t length) {
    emit(sig_queueLength, length);
}

void DSMEPlatform::signalNumDroppedRetransmissionPackets(uint32_t packets){
    emit(sig_numDroppedRetransmissionPackets, packets);
}

void DSMEPlatform::signalNumDroppedPackets(uint32_t packets){
    emit(sig_numDroppedPackets, packets);
}

void DSMEPlatform::signalAcksInGack(uint32_t packets){
    emit(sig_acksInGack, packets);
}

void DSMEPlatform::signalPacketRetransmissionRate(double rate){
    emit(sig_packetRetransmissionRate, rate);
}

void DSMEPlatform::signalRetransmissionQueueLength(uint32_t length) {
    emit(sig_retransmissionQueueLength, length);
}

void DSMEPlatform::signalMessagesInUse(uint32_t nr) {
    emit(sig_messagesInUse, nr);
}

void DSMEPlatform::signalCFPAckDelay(uint32_t symbols) {
    emit(sig_cfpAckDelay, symbols);
}

void DSMEPlatform::signalCAPAckDelay(uint32_t symbols) {
    emit(sig_capAckDelay, symbols);
}

void DSMEPlatform::signalPacketsTXPerSlot(uint32_t packets) {
    emit(sig_packetsTXPerSlot, packets);
}

void DSMEPlatform::signalPacketsRXPerSlot(uint32_t packets) {
    emit(sig_packetsRXPerSlot, packets);
}

}
