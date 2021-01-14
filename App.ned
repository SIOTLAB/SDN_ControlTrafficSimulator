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

//
// Application model to generate traffic for the network.
//
simple App like IApp
{
    parameters:
        int address;  // local App node address
       
        // Controller'a message time
        volatile double sendPollTime @unit(s) = default(3s); // time between generating packets
        volatile double sendPacketInTime @unit(s) = default(100s);
        volatile double sendPacketInTime275 @unit(s) = default(1s); // 275 messages
       	int num_queues;
       	int num_ports;
       	int total_nodes;
    	int mode;
        @display("i=block/browser");
        
    gates:
        input in;
        output out;
}


