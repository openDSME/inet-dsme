//
// Copyright (C) 2014 Florian Meier <florian.meier@koalo.de>
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

#include <LiveRecorder.h>
#include <sstream>

namespace inet {

Register_ResultRecorder("live", LiveRecorder);

void LiveRecorder::collect(std::string value) {
    ASSERT(1 == 4);
    int* a = 0;
    *a = 17;
    if(value == "1") {
        value = "@";
    }
}

void LiveRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, bool b DETAILS_ARG) {
    collect(b ? "true" : "false");
}

void LiveRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, long l DETAILS_ARG) {
    std::stringstream s;
    s << l;
    collect(s.str());
}

void LiveRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, unsigned long l DETAILS_ARG) {
    std::stringstream s;
    s << l;
    collect(s.str());
}

void LiveRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, double d DETAILS_ARG) {
    std::stringstream s;
    s << d;
    collect(s.str());
}

void LiveRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, const SimTime& v DETAILS_ARG) {
    collect(v.str());
}

void LiveRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, const char *s DETAILS_ARG) {
    collect(s);
}

void LiveRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *obj DETAILS_ARG) {
    collect(obj->getFullPath());
}

void LiveRecorder::finish(cResultFilter *prev) {
/*    opp_string_map attributes = getStatisticAttributes();

    for(auto & elem : groupcounts) {
        std::stringstream name;
        name << getResultName().c_str() << ":" << elem.first;
        getEnvir()->recordScalar(getComponent(), name.str().c_str(), elem.second, &attributes); // note: this is NaN if count==0
    }
    */
}

} // namespace inet

