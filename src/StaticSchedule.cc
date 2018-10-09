#include "StaticSchedule.h"

#include <omnetpp.h>

#include <iomanip>
#include <stdlib.h>

#include <inet/common/ModuleAccess.h>
#include <inet/linklayer/common/InterfaceTag_m.h>
#include <inet/linklayer/common/MacAddressTag_m.h>
#include <inet/common/ProtocolTag_m.h>
#include <inet/common/ProtocolGroup.h>
#include <inet/common/packet/chunk/ByteCountChunk.h>
#include <inet/physicallayer/base/packetlevel/FlatRadioBase.h>
#include <inet/physicallayer/common/packetlevel/SignalTag_m.h>
#include <inet/physicallayer/contract/packetlevel/IRadio.h>

#include "openDSME/dsmeLayer/DSMELayer.h"
#include "openDSME/dsmeLayer/messages/MACCommand.h"
#include "openDSME/dsmeAdaptionLayer/scheduling/PIDScheduling.h"
#include "openDSME/dsmeAdaptionLayer/scheduling/TPS.h"
#include "openDSME/dsmeAdaptionLayer/scheduling/StaticScheduling.h"
#include "openDSME/mac_services/pib/dsme_phy_constants.h"

namespace dsme {


std::vector<StaticSlot> StaticSchedule::loadSchedule(omnetpp::cXMLElement *xmlFile, uint16_t address) {
    char idString[5];
    sprintf(idString, "%d", address);  
    omnetpp::cXMLElement *node = xmlFile->getFirstChildWithAttribute("node", "id", idString);
  
 
    std::vector<StaticSlot> slots; 
    omnetpp::cXMLElementList slotList = node->getChildrenByTagName("slot");    
    for(auto &slot : slotList) {
        StaticSlot s;
        s.slotID = atoi(slot->getAttribute("slotID"));
        s.superframeID = atoi(slot->getAttribute("superframeID"));
        s.channelID = atoi(slot->getAttribute("channelID"));
        s.direction = atoi(slot->getAttribute("direction"));
        s.address = atoi(slot->getAttribute("address"));

        slots.push_back(s);      
    }

    return slots;
}

}
