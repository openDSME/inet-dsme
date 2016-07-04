/* HEADER */

#ifndef PHY_CONSTANTS_H
#define PHY_CONSTANTS_H

namespace dsme {

/*
 * This file contains all PHY constants as defined in IEEE 802.15.4-2011, 9.2, Table 70
 */

/* The maximum PSDU size (in octets) the PHY shall be able to receive. */
constexpr uint8_t aMaxPHYPacketSize = 127;

/* RX-to-TX or TX-to-RX turnaround time (in symbol periods), as defined in 8.2.1 and 8.2.2. */
constexpr uint8_t aTurnaroundTime = 12;

} /* dsme */

#endif /* PHY_CONSTANTS_H */
