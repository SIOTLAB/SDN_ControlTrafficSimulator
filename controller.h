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

#ifndef __FOGSIMULATION_CONTROLLER_H_
#define __FOGSIMULATION_CONTROLLER_H_

#include <omnetpp.h>

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Controller : public cSimpleModule
{
public:
    protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif
