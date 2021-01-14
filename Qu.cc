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

#include "Qu.h"

Define_Module(Qu);


Qu::Qu()
{
    endTransmissionEvent = nullptr;
}

Qu::~Qu()
{
    cancelAndDelete(endTransmissionEvent);
}

void Qu::initialize()
{
    myAddress = getParentModule()->par("address");

    msg_queue.setName("my_queue");

    endTransmissionEvent = new cMessage("endTxEvent");

    frameCapacity = par("frameCapacity");
    qlenSignal = registerSignal("qlen");
    busySignal = registerSignal("busy");
    queueingTimeSignal = registerSignal("queueingTime");
    dropSignal = registerSignal("drop");
    txBytesSignal = registerSignal("txBytes");
    rxBytesSignal = registerSignal("rxBytes");

    emit(qlenSignal, msg_queue.getLength());
    emit(busySignal, false);
    isBusy = false;

    cModule *network = cSimulation::getActiveSimulation()->getSystemModule();
    for (SubmoduleIterator it(network); !it.end(); ++it) {
        cModule *mod = *it;
        if (!strcmp(mod->getName(), "controller")) {
           CONTROLLER_ID = mod->getId();
        }
    }
}

void Qu::startTransmitting(cMessage *msg)
{
    //EV << "Starting transmission of " << msg << endl;
    isBusy = true;
    int64_t numBytes = check_and_cast<cPacket *>(msg)->getByteLength();
    send(msg, "line$o");
    emit(txBytesSignal, (long)numBytes);

    // Schedule an event for the time when last bit will leave the gate.
    simtime_t endTransmission = gate("line$o")->getTransmissionChannel()->getTransmissionFinishTime();
    // Schedule this now
    scheduleAt(endTransmission, endTransmissionEvent);
}

void Qu::handleMessage(cMessage *msg)
{
    if (!msg) return;

    // If the message came on the line, we just pass it up
    if (msg->arrivedOn("line$i")) {
        // We received it from the line
        emit(rxBytesSignal, (long)check_and_cast<cPacket *>(msg)->getByteLength());
        send(msg, "out");
        return;
    }

    if (msg->isSelfMessage()) {
        /* Below part is Transmission and should be executed for all kinds of nodes */
        if (msg == endTransmissionEvent) {
            // Transmission finished, we can start next one.
            isBusy = false;
            if (msg_queue.isEmpty()) {
                emit(busySignal, false);
            }
            else {
                //EV << "WE have more packets Queued up:" << msg_queue.getLength() <<" \n";
                while(!msg_queue.isEmpty()) {
                    cMessage *new_msg = (cMessage *)msg_queue.pop();
                    send(new_msg, "out");
                }
            }
        }
        return;
    }
    // arrived on gate "in"
    if (endTransmissionEvent->isScheduled()) {
        // We are currently busy, so just queue up the packet.
        if (frameCapacity && msg_queue.getLength() >= frameCapacity) {
            //EV << "Qu:" << myAddress << "Received " << msg << " but transmitter busy and queue full: discarding\n";
            emit(dropSignal, (long)check_and_cast<cPacket *>(msg)->getByteLength());
            delete msg;
        }
        else {
            //EV << "Qu:" << myAddress << "queuing " << msg << ": queuing up\n";
            msg->setTimestamp();
            msg_queue.insert(msg);
            emit(qlenSignal, msg_queue.getLength());
        }
    } else {
        //EV << "Qu:" << myAddress << "idle, so we can start transmitting right away" << msg << endl;
        emit(queueingTimeSignal, SIMTIME_ZERO);
        startTransmitting(msg);
        emit(busySignal, true);
    }
}
