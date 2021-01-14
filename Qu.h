// SDN Control Traffic Simulator: simulate control traffic throughput
// and latencies in a variety of topologies
// Copyright (C) 2020  Ananya Gopal, Jesse Chen

// This program is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.

// You should have received a copy of the
// GNU General Public License along with this program.
// If not, see <https://www.gnu.org/licenses/>.

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include<stdio.h>
#include<string.h>
#include<omnetpp.h>


#include "Packet_m.h"
#include "packet_types.h"

#define CONTROLLER_ADDRS 2000

using namespace omnetpp;

class Qu: public cSimpleModule
{
private:
    int myAddress, CONTROLLER_ID = 0;

    cQueue msg_queue;

    /* Frame signals*/
    long frameCapacity;
    bool isBusy;
    simsignal_t qlenSignal;
    simsignal_t busySignal;
    simsignal_t queueingTimeSignal;
    simsignal_t dropSignal;
    simsignal_t txBytesSignal;
    simsignal_t rxBytesSignal;

    cMessage *endTransmissionEvent;

public:
    Qu();
    virtual ~Qu();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void startTransmitting(cMessage *msg);
};
