#include "ExtCSMA.h"

using namespace inet;

Define_Module(inet::ExtCSMA);

void ExtCSMA::packetDropped(CSMAFrame* macPkt, PacketResult result) {
    uint16_t short_address = macPkt->getSrcAddr().getAddressByte(5)+(((uint16_t)macPkt->getSrcAddr().getAddressByte(4))<<8);

    if(macPkt->hasEncapsulatedPacket()) {
        cPacket *netPkt = macPkt->decapsulate();

        if(netPkt->hasEncapsulatedPacket()) {
            cPacket *appPkt = netPkt->decapsulate();
            LiveTrafGen* trafGen = dynamic_cast<LiveTrafGen*>(appPkt->getSenderModule());
            if(trafGen != nullptr) {
                trafGen->handleDroppedPacket(appPkt, short_address, result);
            }

            delete appPkt;
        }

        delete netPkt;
    }

    delete macPkt;
}

void ExtCSMA::manageMissingAck(t_mac_event    /*event*/, cMessage *    /*msg*/)
{
    if (txAttempts < macMaxFrameRetries) {
        // increment counter
        txAttempts++;
        EV_DETAIL << "I will retransmit this packet (I already tried "
                  << txAttempts << " times)." << endl;
    }
    else {
        // drop packet
        EV_DETAIL << "Packet was transmitted " << txAttempts
                  << " times and I never got an Ack. I drop the packet." << endl;
        cMessage *mac = macQueue.front();
        macQueue.pop_front();
        txAttempts = 0;
        // TODO: send dropped signal
        // emit(packetDropped, mac);
        //emit(NF_LINK_BREAK, mac);
        packetDropped((CSMAFrame*)mac,PacketResult::NO_ACK);
//        delete mac;
    }
    manageQueue();
}

void ExtCSMA::updateStatusCCA(t_mac_event event, cMessage *msg)
{
    switch (event) {
        case EV_TIMER_CCA: {
            EV_DETAIL << "(25) FSM State CCA_3, EV_TIMER_CCA" << endl;
            bool isIdle = radio->getReceptionState() == IRadio::RECEPTION_STATE_IDLE;
            if (isIdle) {
                EV_DETAIL << "(3) FSM State CCA_3, EV_TIMER_CCA, [Channel Idle]: -> TRANSMITFRAME_4." << endl;
                updateMacState(TRANSMITFRAME_4);
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                CSMAFrame *mac = check_and_cast<CSMAFrame *>(macQueue.front()->dup());
                attachSignal(mac, simTime() + aTurnaroundTime);
                //sendDown(msg);
                // give time for the radio to be in Tx state before transmitting
                sendDelayed(mac, aTurnaroundTime, lowerLayerOutGateId);
                nbTxFrames++;
            }
            else {
                // Channel was busy, increment 802.15.4 backoff timers as specified.
                EV_DETAIL << "(7) FSM State CCA_3, EV_TIMER_CCA, [Channel Busy]: "
                          << " increment counters." << endl;
                NB = NB + 1;
                //BE = std::min(BE+1, macMaxBE);

                // decide if we go for another backoff or if we drop the frame.
                if (NB > macMaxCSMABackoffs) {
                    // drop the frame
                    EV_DETAIL << "Tried " << NB << " backoffs, all reported a busy "
                              << "channel. Dropping the packet." << endl;
                    cMessage *mac = macQueue.front();
                    macQueue.pop_front();
                    txAttempts = 0;
                    nbDroppedFrames++;
                    emit(packetFromUpperDroppedSignal, mac);
                    packetDropped((CSMAFrame*)mac,PacketResult::CHANNEL_BUSY);

                    //delete mac;
                    manageQueue();
                }
                else {
                    // redo backoff
                    updateMacState(BACKOFF_2);
                    startTimer(TIMER_BACKOFF);
                }
            }
            break;
        }

        case EV_DUPLICATE_RECEIVED:
            EV_DETAIL << "(26) FSM State CCA_3, EV_DUPLICATE_RECEIVED:";
            if (useMACAcks) {
                EV_DETAIL << " setting up radio tx -> WAITSIFS." << endl;
                // suspend current transmission attempt,
                // transmit ack,
                // and resume transmission when entering manageQueue()
                transmissionAttemptInterruptedByRx = true;
                cancelEvent(ccaTimer);

                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                updateMacState(WAITSIFS_6);
                startTimer(TIMER_SIFS);
            }
            else {
                EV_DETAIL << " Nothing to do." << endl;
            }
            //sendUp(decapsMsg(static_cast<MacPkt*>(msg)));
            delete msg;
            break;

        case EV_FRAME_RECEIVED:
            EV_DETAIL << "(26) FSM State CCA_3, EV_FRAME_RECEIVED:";
            if (useMACAcks) {
                EV_DETAIL << " setting up radio tx -> WAITSIFS." << endl;
                // suspend current transmission attempt,
                // transmit ack,
                // and resume transmission when entering manageQueue()
                transmissionAttemptInterruptedByRx = true;
                cancelEvent(ccaTimer);
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                updateMacState(WAITSIFS_6);
                startTimer(TIMER_SIFS);
            }
            else {
                EV_DETAIL << " Nothing to do." << endl;
            }
            sendUp(decapsMsg(static_cast<CSMAFrame *>(msg)));
            delete msg;
            break;

        case EV_BROADCAST_RECEIVED:
            EV_DETAIL << "(24) FSM State BACKOFF, EV_BROADCAST_RECEIVED:"
                      << " Nothing to do." << endl;
            sendUp(decapsMsg(static_cast<CSMAFrame *>(msg)));
            delete msg;
            break;

        default:
            fsmError(event, msg);
            break;
    }
}

void ExtCSMA::updateStatusIdle(t_mac_event event, cMessage *msg)
{
    switch (event) {
        case EV_SEND_REQUEST:
            if (macQueue.size() <= queueLength) {
                macQueue.push_back(static_cast<CSMAFrame *>(msg));
                EV_DETAIL << "(1) FSM State IDLE_1, EV_SEND_REQUEST and [TxBuff avail]: startTimerBackOff -> BACKOFF." << endl;
                updateMacState(BACKOFF_2);
                NB = 0;
                //BE = macMinBE;
                startTimer(TIMER_BACKOFF);
            }
            else {
                // queue is full, message has to be deleted
                EV_DETAIL << "(12) FSM State IDLE_1, EV_SEND_REQUEST and [TxBuff not avail]: dropping packet -> IDLE." << endl;
                emit(packetFromUpperDroppedSignal, msg);
                packetDropped((CSMAFrame*)msg,PacketResult::QUEUE_FULL);
                updateMacState(IDLE_1);
            }
            break;

        case EV_DUPLICATE_RECEIVED:
            EV_DETAIL << "(15) FSM State IDLE_1, EV_DUPLICATE_RECEIVED: setting up radio tx -> WAITSIFS." << endl;
            //sendUp(decapsMsg(static_cast<MacSeqPkt *>(msg)));
            delete msg;

            if (useMACAcks) {
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                updateMacState(WAITSIFS_6);
                startTimer(TIMER_SIFS);
            }
            break;

        case EV_FRAME_RECEIVED:
            EV_DETAIL << "(15) FSM State IDLE_1, EV_FRAME_RECEIVED: setting up radio tx -> WAITSIFS." << endl;
            sendUp(decapsMsg(static_cast<CSMAFrame *>(msg)));
            nbRxFrames++;
            delete msg;

            if (useMACAcks) {
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                updateMacState(WAITSIFS_6);
                startTimer(TIMER_SIFS);
            }
            break;

        case EV_BROADCAST_RECEIVED:
            EV_DETAIL << "(23) FSM State IDLE_1, EV_BROADCAST_RECEIVED: Nothing to do." << endl;
            nbRxFrames++;
            sendUp(decapsMsg(static_cast<CSMAFrame *>(msg)));
            delete msg;
            break;

        default:
            fsmError(event, msg);
            break;
    }
}
