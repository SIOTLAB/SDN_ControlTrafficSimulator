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

module Controller
{
    parameters:
        int address = 2000;
        string appType;
        int num_flows;
        @display("i=device/server");
    gates:
        inout port_inout[];

    submodules:
        app: <appType> like IApp {
            parameters:
                address = address;
                @display("p=140,60");
        }
        
        routing: Routing {
            parameters:
                @display("p=286,123");
            gates:
                in[sizeof(port_inout)];
                out[sizeof(port_inout)];
        }

        queue[sizeof(port_inout)]: Qu {
            parameters:
                address_qu = address;
                @display("p=140,261,row");
        }
         
    connections:
         routing.localOut --> app.in;
         routing.localIn <-- app.out;
         
        for i=0..sizeof(port_inout)-1 {
            routing.out[i] --> queue[i].in;
            routing.in[i] <-- queue[i].out;
            queue[i].line <--> port_inout[i];
        }
}