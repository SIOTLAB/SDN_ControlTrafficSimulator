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

simple Qu
{
    parameters:
        int address_qu;  // local Qu node address

 		int frameCapacity = default(0); // max number of packets; 0 means no limit
        bool useCutThroughSwitching = default(false);  // use cut-through switching instead of store-and-forward
        
        // Controller'a message time

        @display("i=block/routing");

        gates:
            input in;
            output out;
            inout line;
}