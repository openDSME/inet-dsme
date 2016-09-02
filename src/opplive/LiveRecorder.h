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

#ifndef __INET_LIVERECORDER_H
#define __INET_LIVERECORDER_H

#include <string>
#include "inet/common/INETDefs.h"
#include "wamp_cpp/WAMPServer.h"
#include "wamp_cpp/RPCallable.h"
#include <thread>
#include "wamp_cpp/Topic.h"

namespace inet {

/**
 * Listener for sending events via websocket
 */
class INET_API LiveRecorder : public cResultRecorder,  public RPCallable<LiveRecorder>
{
    protected:
        //std::map<std::string,long> groupcounts;
    int a;

    protected:
        virtual void collect(std::string val);
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, bool b, cObject *details) override;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, long l, cObject *details) override;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, unsigned long l, cObject *details) override;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, double d, cObject *details) override;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const SimTime& v, cObject *details) override;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const char *s, cObject *details) override;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *obj, cObject *details) override;

        static WAMPServer* server;

        void eventLoop();
        std::thread eventThread;
        bool running;

        Topic<int> topic;

    public:
        LiveRecorder();
        ~LiveRecorder();
        virtual void finish(cResultFilter *prev) override;

        int adding(int a, int b);
        void handleEvent1(int a);
};

} // namespace inet

#endif
