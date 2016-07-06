//
// Copyright (C) 2006 Andras Varga
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

#ifndef __INET_DSMESTATIONARYMOBILITY_H
#define __INET_DSMESTATIONARYMOBILITY_H

#include "inet/common/INETDefs.h"

#include "base/DSMEMobilityBase.h"

namespace inet_dsme {

/**
 * This mobility module does not move at all; it can be used for standalone stationary nodes.
 *
 * @ingroup mobility
 * @author Andras Varga
 */
class DSMEStationaryMobility : public DSMEMobilityBase {
protected:
    /** @brief Never happens. */
    virtual void handleSelfMessage(cMessage *message) override;

public:
    DSMEStationaryMobility() {
    }

    DSMEStationaryMobility(Coord position) {
        lastPosition = position;
    }

    /** @brief Returns the current position at the current simulation time. */
    virtual Coord getCurrentPosition() override {
        return lastPosition;
    }

    /** @brief Returns the current speed at the current simulation time. */
    virtual Coord getCurrentSpeed() override {
        return Coord::ZERO;
    }

    virtual double getMaxSpeed() const override {
        return 0;
    }

    virtual Coord getConstraintAreaMax() const override {
        return lastPosition;
    }
    virtual Coord getConstraintAreaMin() const override {
        return lastPosition;
    }
};

} /* inet_dsme */

#endif

