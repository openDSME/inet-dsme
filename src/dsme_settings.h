/* HEADER */

#ifndef DSME_SETTINGS_H
#define DSME_SETTINGS_H

#include <stdint.h>

namespace dsme {

constexpr uint8_t aNumSuperframeSlots = 16; // fixed value, see Table 8-80 of IEEE 802.15.4-2015, this includes beacon, CAP and GTS
constexpr uint8_t macLIFSPeriod = 40; // fixed value, see 8.1.3 of IEEE 802.15.4-2011 (assuming no UWB PHY)
constexpr uint8_t macSIFSPeriod = 12; // fixed value, see 8.1.3 of IEEE 802.15.4-2011 (assuming no UWB PHY)
constexpr uint8_t PRE_EVENT_SHIFT = macSIFSPeriod;
constexpr uint8_t MIN_CSMA_SLOTS = 0; // 0 for CAP reduction
constexpr uint8_t MAX_GTSLOTS = aNumSuperframeSlots - MIN_CSMA_SLOTS - 1;
constexpr uint8_t MAX_CHANNELS = 16;
constexpr uint8_t MIN_CHANNEL = 11;
constexpr uint8_t MAX_NEIGHBORS = 25;

constexpr uint8_t MIN_SO = 1;
constexpr uint8_t MAX_BO = 10;
constexpr uint8_t MAX_MO = 9;
constexpr uint16_t MAX_SLOTS_PER_SUPERFRAMES = 1 << (uint16_t)(MAX_BO - MIN_SO);
constexpr uint16_t MAX_TOTAL_SUPERFRAMES = 1 << (uint16_t)(MAX_BO - MIN_SO);
constexpr uint16_t MAX_SUPERFRAMES_PER_MULTI_SUPERFRAME = 1 << (uint16_t)(MAX_MO - MIN_SO);
constexpr uint16_t MAX_OCCUPIED_SLOTS = MAX_SUPERFRAMES_PER_MULTI_SUPERFRAME*MAX_GTSLOTS*MAX_CHANNELS;

constexpr uint8_t MAX_SAB_UNITS = 1;

constexpr uint16_t CAP_QUEUE_SIZE = 8;
constexpr uint16_t TOTAL_GTS_QUEUE_SIZE = 30-CAP_QUEUE_SIZE;
constexpr uint16_t MSG_POOL_SIZE = CAP_QUEUE_SIZE+TOTAL_GTS_QUEUE_SIZE+5;

}

#endif
