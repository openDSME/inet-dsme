//
// Copyright (C) 2013 Opensim Ltd
// Author: Levente Meszaros
// Author: Institute of Telematics, Hamburg University of Technology (SP_GPSR)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __INET_DSME_GPSR_H
#define __INET_DSME_GPSR_H

#include "inet/routing/gpsr/Gpsr.h"

namespace inet_dsme {

class SP_GPSR : public inet::Gpsr
{
    virtual inet::GpsrOption *createGpsrOption(inet::L3Address destination);
    virtual inet::L3Address findGreedyRoutingNextHop(const inet::Ptr<const inet::NetworkHeaderBase>& networkHeader, const inet::L3Address& destination);
    virtual inet::L3Address findPerimeterRoutingNextHop(const inet::Ptr<const inet::NetworkHeaderBase>& networkHeader, const inet::L3Address& destination);
};

} // namespace inet

#endif // ifndef __INET_GPSR_H

