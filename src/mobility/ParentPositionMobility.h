/*
 * Copyright (C) 2014 Florian Meier <florian.meier@koalo.de>
 *
 * Based on:
 * Copyright (C) 2006 Isabel Dietrich <isabel.dietrich@informatik.uni-erlangen.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#ifndef __INET_PARENTPOSITIONMOBILITY_H
#define __INET_PARENTPOSITIONMOBILITY_H

#include "inet/common/INETDefs.h"

#include "DSMEStationaryMobility.h"


namespace inet_dsme {

/**
 * @brief Mobility model which places a host at the position of its parent module
 *
 * @ingroup mobility
 * @author Maximilian Köstler
 */
class ParentPositionMobility : public DSMEStationaryMobility
{
  protected:
    /** @brief Initializes the position according to the mobility model. */
    virtual void setInitialPosition() override;

  public:
    ParentPositionMobility() {};

    virtual void updatePosition();
};

}

#endif
