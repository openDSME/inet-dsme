#include "DSMEPlatform.h"

#include <iomanip>

#include "INETMath.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/FindModule.h"
#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"
#include "inet/physicallayer/base/packetlevel/FlatRadioBase.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "openDSME/dsmeLayer/DSMELayer.h"
#include "openDSME/dsmeLayer/messages/MACCommand.h"
#include "openDSME/dsmeAdaptionLayer/scheduling/TPSQ.h"
#include "openDSME/dsmeAdaptionLayer/scheduling/STAS.h"
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

simsignal_t DSMEPlatform::broadcastDataSentDown = registerSignal("broadcastDataSentDown");
simsignal_t DSMEPlatform::unicastDataSentDown = registerSignal("unicastDataSentDown");
simsignal_t DSMEPlatform::commandSentDown = registerSignal("commandSentDown");
simsignal_t DSMEPlatform::beaconSentDown = registerSignal("beaconSentDown");
simsignal_t DSMEPlatform::ackSentDown = registerSignal("ackSentDown");
simsignal_t DSMEPlatform::uncorruptedFrameReceived = registerSignal("uncorruptedFrameReceived");
simsignal_t DSMEPlatform::corruptedFrameReceived = registerSignal("corruptedFrameReceived");

void translateMacAddress(MACAddress& from, IEEE802154MacAddress& to) {
    // TODO correct translation
    if(from.isBroadcast()) {
        to = IEEE802154MacAddress(IEEE802154MacAddress::SHORT_BROADCAST_ADDRESS);
    } else {
        to.setShortAddress((from.getAddressByte(4) << 8) | from.getAddressByte(5));
    }
}

uint8_t PERtoLQI(double per) {
    // inverse function of the graph given in the ATmega256RFR2 datasheet
    double lqi = -22.2222 * log(0.00360656 * (-1 + (1 / (1 - per))));
    if(lqi > 255) {
        lqi = 255;
    } else if(lqi < 0) {
        lqi = 0;
    }
    return (uint8_t)(lqi + 0.5);
}

std::string getErrorInfo(DSMEFrame* macPkt) {
    std::stringstream ss;

    inet::physicallayer::ReceptionIndication* control = check_and_cast<inet::physicallayer::ReceptionIndication*>(macPkt->getControlInfo());
    ss << control->getBitErrorCount() << ", ";
    ss << std::setprecision(3) << control->getBitErrorRate() * 100.0 << "%, ";
    ss << control->getPacketErrorRate() * 100.0 << "%, ";
    ss << "LQI " << PERtoLQI(control->getPacketErrorRate()) << ", ";
    ss << "SNIR: " << inet::math::fraction2dB(control->getMinSNIR()) << " dB, ";
    ss << "RSSI: " << inet::math::mW2dBm(control->getMinRSSI().get() * 1000) << " dBm ";

    return ss.str();
}

DSMEPlatform::DSMEPlatform()
    : phy_pib(),
      mac_pib(phy_pib),

      dsme(new DSMELayer()),
      mcps_sap(*dsme),
      mlme_sap(*dsme),
      dsmeAdaptionLayer(*dsme) {
}

DSMEPlatform::~DSMEPlatform() {
    delete dsme;
    delete scheduling;

    cancelAndDelete(ccaTimer);
    cancelAndDelete(cfpTimer);
    cancelAndDelete(timer);
}

/****** INET ******/

InterfaceEntry* DSMEPlatform::createInterfaceEntry() {
    InterfaceEntry* e = new InterfaceEntry(this);

    // data rate
    e->setDatarate(bitrate);

    // generate a link-layer address to be used as interface token for IPv6
    e->setMACAddress(addr);
    e->setInterfaceToken(addr.formInterfaceIdentifier());

    // capabilities
    e->setMtu(par("mtu").longValue());
    e->setMulticast(true);
    e->setBroadcast(true);

    return e;
}

/****** OMNeT++ ******/

void DSMEPlatform::initialize(int stage) {
    MACProtocolBase::initialize(stage);

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
        //scheduling = new TPSQ(this->dsmeAdaptionLayer);
        scheduling = new STAS(this->dsmeAdaptionLayer);
        this->dsmeAdaptionLayer.initialize(scanChannels,scheduling);

        /* Initialize Address */
        IEEE802154MacAddress address;
        const char* addrstr = par("address");

        if(!strcmp(addrstr, "auto")) {
            // assign automatic address
            addr = MACAddress::generateAutoAddress();

            // change module parameter from "auto" to concrete address
            par("address").setStringValue(addr.str().c_str());
        } else {
            addr.setAddress(addrstr);
        }

        translateMacAddress(addr, this->mac_pib.macExtendedAddress);
        registerInterface();

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
        this->mac_pib.macResponseWaitTime = 32;

        this->mac_pib.macIsPANCoord = par("isPANCoordinator");

        this->mac_pib.macIsCoord = (par("isCoordinator") || this->mac_pib.macIsPANCoord);

        this->phy_pib.phyCurrentChannel = par("commonChannel");

        if(strcmp(par("allocationScheme").stringValue(), "random") == 0) {
            this->dsmeAdaptionLayer.settings.allocationScheme = DSMEAdaptionLayerSettings::ALLOC_RANDOM;
        } else {
            this->dsmeAdaptionLayer.settings.allocationScheme = DSMEAdaptionLayerSettings::ALLOC_CONTIGUOUS_SLOT;
        }

        this->dsmeAdaptionLayer.setIndicationCallback(DELEGATE(&DSMEPlatform::handleIndicationFromMCPS, *this));
        this->dsmeAdaptionLayer.setConfirmCallback(DELEGATE(&DSMEPlatform::handleConfirmFromMCPS, *this));

        this->dsme->initialize(this);
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
}

void DSMEPlatform::handleLowerPacket(cPacket* pkt) {
    DSMEFrame* macPkt;
    if(nullptr == (macPkt = dynamic_cast<DSMEFrame*>(pkt))) {
        DSME_ASSERT(false);
        return;
    }

    if(!this->transceiverIsOn) {
        DSMEMessage* dsmemsg = getLoadedMessage(macPkt);
        dsmemsg->getHeader().decapsulateFrom(dsmemsg);

        LOG_DEBUG("Missed frame " << macPkt->detailedInfo() << "(" << getSequenceChartInfo(dsmemsg, false) << ") [" << getErrorInfo(macPkt) << "]");

        releaseMessage(dsmemsg);
        return;
    }

    if(macPkt->hasBitError()) {
        DSMEMessage* dsmemsg = getLoadedMessage(macPkt);
        dsmemsg->getHeader().decapsulateFrom(dsmemsg);

        LOG_DEBUG("Received corrupted frame " << macPkt->detailedInfo() << "(" << getSequenceChartInfo(dsmemsg, false) << ")");
        emit(corruptedFrameReceived, macPkt);

        releaseMessage(dsmemsg);
        return;
    }

    emit(uncorruptedFrameReceived, macPkt);

    DSMEMessage* dsmemsg = getLoadedMessage(macPkt);
    dsmemsg->getHeader().decapsulateFrom(dsmemsg);

    // Get LQI
    inet::physicallayer::ReceptionIndication* control = check_and_cast<inet::physicallayer::ReceptionIndication*>(macPkt->getControlInfo());
    dsmemsg->setLQI(PERtoLQI(control->getPacketErrorRate()));

    LOG_DEBUG("Received valid frame     " << macPkt->detailedInfo() << "(" << getSequenceChartInfo(dsmemsg, false) << ") [" << getErrorInfo(macPkt) << "]");

    // Preamble (4) | SFD (1) | PHY Hdr (1) | MAC Payload | FCS (2)
    dsmemsg->startOfFrameDelimiterSymbolCounter = getSymbolCounter() - dsmemsg->getTotalSymbols() + 2 * 4 // Preamble
                                                  + 2 * 1;                                                // SFD

    dsme->getAckLayer().receive(dsmemsg);
}

void DSMEPlatform::handleUpperPacket(cPacket* pkt) {
    IMACProtocolControlInfo* const cInfo = check_and_cast<IMACProtocolControlInfo*>(pkt->removeControlInfo());
    LOG_INFO_PREFIX;
    LOG_INFO_PURE("Upper layer requests to send a message to ");

    DSMEFrame* macPkt = new DSMEFrame();
    macPkt->setNetworkProtocol(cInfo->getNetworkProtocol());
    macPkt->encapsulate(pkt);

    DSMEMessage* dsmemsg = getLoadedMessage(macPkt);

    auto& hdr = dsmemsg->getHeader();
    hdr.setFrameType(IEEE802154eMACHeader::DATA);
    hdr.setSrcAddr(this->mac_pib.macExtendedAddress);
    MACAddress dest = cInfo->getDestinationAddress();
    if(dest.isMulticast()) {
        // handle multicast as broadcast (TODO ok?)
        dest = MACAddress::BROADCAST_ADDRESS;
        LOG_INFO_PURE("Broadcast");
    } else {
        LOG_INFO_PURE((cInfo->getDestinationAddress().getInt() & 0xFFFF));
    }
    LOG_INFO_PURE("." << std::endl);

    translateMacAddress(dest, dsmemsg->getHeader().getDestAddr());

    delete cInfo;

    dsmemsg->firstTry = true;
    this->dsmeAdaptionLayer.sendMessage(dsmemsg);
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
        // LOG_INFO("switch to receive")
        radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
        delete msg;
    } else {
        MACProtocolBase::handleSelfMessage(msg);
    }
}

void DSMEPlatform::receiveSignal(cComponent* source, simsignal_t signalID, long value DETAILS_ARG) {
    Enter_Method_Silent();
    if(signalID == IRadio::transmissionStateChangedSignal) {
        IRadio::TransmissionState newRadioTransmissionState = static_cast<IRadio::TransmissionState>(value);
        if(transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE) {
            // LOG_INFO("Transmission ready")
            txEndCallback(true); // TODO could it be false?
            scheduleAt(simTime(), new cMessage("receive"));
        }
        transmissionState = newRadioTransmissionState;
    } else if(signalID == IRadio::radioModeChangedSignal) {
        IRadio::RadioMode newRadioMode = static_cast<IRadio::RadioMode>(value);
        if(newRadioMode == IRadio::RADIO_MODE_TRANSMITTER) {
            // LOG_INFO("switched to transmit")
            if(pendingSendRequest) {
                // LOG_INFO("sendDown after tx switch")

                sendDown(pendingTxFrame);
                pendingTxFrame = nullptr; // now owned by lower layer
                pendingSendRequest = false;
            }
        } else if(newRadioMode == IRadio::RADIO_MODE_RECEIVER) {
            // LOG_INFO("switched to receive")
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

    k -= 11;
    auto r = check_and_cast<FlatRadioBase*>(radio);
    r->setCarrierFrequency(MHz(2405 + 5 * k));
    return true;
}

bool DSMEPlatform::prepareSendingCopy(IDSMEMessage* msg, Delegate<void(bool)> txEndCallback) {
    if(msg == nullptr) {
        return false;
    }

    DSMEMessage* dsmeMsg = dynamic_cast<DSMEMessage*>(msg);
    DSME_ASSERT(dsmeMsg != nullptr);

    LOG_DEBUG(getSequenceChartInfo(msg, true));

    LOG_INFO("sendCopyNow " << (uint64_t)msg);

    this->txEndCallback = txEndCallback;
    DSMEFrame* frame = dsmeMsg->getSendableCopy();

    switch(msg->getHeader().getFrameType()) {
        case IEEE802154eMACHeader::BEACON:
            emit(beaconSentDown, frame);
            break;
        case IEEE802154eMACHeader::DATA:
            if(msg->getHeader().getDestAddr().isBroadcast()) {
                emit(broadcastDataSentDown, frame);
            } else {
                emit(unicastDataSentDown, frame);
            }
            break;
        case IEEE802154eMACHeader::ACKNOWLEDGEMENT:
            emit(ackSentDown, frame);
            break;
        case IEEE802154eMACHeader::COMMAND:
            emit(commandSentDown, frame);
            break;
        default:
            DSME_ASSERT(false);
    }

    DSME_ASSERT(pendingTxFrame == nullptr);

    pendingTxFrame = frame;

    if(radio->getRadioMode() != IRadio::RADIO_MODE_TRANSMITTER) {
        DSME_ASSERT(msg->getHeader().getFrameType() != IEEE802154eMACHeader::ACKNOWLEDGEMENT); // switching is handled by ACK routine
        radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    }

    return true;
}

bool DSMEPlatform::sendNow() {
    DSME_ASSERT(this->transceiverIsOn);
    DSME_ASSERT(this->pendingTxFrame);
    DSME_ASSERT(!this->pendingSendRequest);

    if(this->radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER) {
        // can be sent direct
        sendDown(this->pendingTxFrame);
        this->pendingTxFrame = nullptr;
    } else {
        this->pendingSendRequest = true;
    }
    // otherwise receiveSignal will be called eventually
    return true;
}

void DSMEPlatform::abortPreparedTransmission() {
    DSME_ASSERT(!pendingSendRequest);
    DSME_ASSERT(pendingTxFrame);
    delete pendingTxFrame;
    pendingTxFrame = nullptr;
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
    this->transceiverIsOn = true;
    this->radio->setRadioMode(inet::physicallayer::IRadio::RADIO_MODE_RECEIVER);
}

void DSMEPlatform::turnTransceiverOff(){
    this->transceiverIsOn = false;
    this->radio->setRadioMode(inet::physicallayer::IRadio::RADIO_MODE_OFF);
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
    DSME_ASSERT(messagesInUse <= MSG_POOL_SIZE); // TODO should return nullptr (and check everywhere!!)
    DSMEMessage* msg = new DSMEMessage();
    msg->receivedViaMCPS = false;
    signalNewMsg(msg);
    return msg;
}

DSMEMessage* DSMEPlatform::getLoadedMessage(DSMEFrame* frame) {
    messagesInUse++;
    DSME_ASSERT(messagesInUse <= MSG_POOL_SIZE); // TODO
    DSMEMessage* msg = new DSMEMessage(frame);
    msg->receivedViaMCPS = false;
    signalNewMsg(msg);
    return msg;
}

void DSMEPlatform::releaseMessage(IDSMEMessage* msg) {
    DSME_ASSERT(messagesInUse > 0);
    DSME_ASSERT(msg != nullptr);
    messagesInUse--;

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
    return 150; // corresponds roughly to 20% PER
}

void DSMEPlatform::handleIndicationFromMCPS(IDSMEMessage* msg) {
    DSMEMessage* dsmeMsg = dynamic_cast<DSMEMessage*>(msg);
    DSME_ASSERT(dsmeMsg != nullptr);

    DSMEFrame* macPkt = dsmeMsg->decapsulateFrame();
    releaseMessage(msg);
    cPacket* packet = macPkt->decapsulate();

    SimpleLinkLayerControlInfo* const controlInfo = new SimpleLinkLayerControlInfo();
    controlInfo->setSrc(macPkt->getSrcAddr());
    controlInfo->setInterfaceId(interfaceEntry->getInterfaceId());
    controlInfo->setNetworkProtocol(macPkt->getNetworkProtocol());
    packet->setControlInfo(controlInfo);

    delete macPkt;
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
    uint8_t numGTSlots = this->dsmeAdaptionLayer.getMAC_PIB().helper.getNumGTSlots();
    uint8_t numSuperFramesPerMultiSuperframe = this->dsmeAdaptionLayer.getMAC_PIB().helper.getNumberSuperframesPerMultiSuperframe();

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
            GTS gts = GTS::GTSfromAbsoluteIndex((*it) + subBlock.getSubBlockIndex() * numGTSlots * numChannels, numGTSlots, numChannels,
                                                numSuperFramesPerMultiSuperframe);

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
            uint8_t cmd = dsmeMsg->frame->getData()[0];

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
                            ss << getDSMEManagement(dsmeMsg->frame->getData()[1], req.getSABSpec(), cmdd.getCmdId());
                            break;
                        }
                        case DSME_GTS_REPLY: {
                            ss << "DSME-GTS-REPLY";
                            GTSReplyNotifyCmd reply;
                            reply.decapsulateFrom(m);
                            ss << getDSMEManagement(dsmeMsg->frame->getData()[1], reply.getSABSpec(), cmdd.getCmdId());
                            break;
                        }
                        case DSME_GTS_NOTIFY: {
                            ss << "DSME-GTS-NOTIFY";
                            GTSReplyNotifyCmd notify;
                            notify.decapsulateFrom(m);
                            ss << getDSMEManagement(dsmeMsg->frame->getData()[1], notify.getSABSpec(), cmdd.getCmdId());
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
}
