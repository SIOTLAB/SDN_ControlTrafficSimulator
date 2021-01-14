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

network TwoNode_test {
    int controller_port  = 1;
    int num_switches  = 1;
    int total_nodes = 1;
    submodules:
        // Right Side
        Leaf[num_switches] : Node {
            parameters:
                @display("i=device/router, p=,,m,1,5,5");
                address = index;
                app.total_nodes = total_nodes;
                app.num_ports = 1;
                app.num_queues = 1;
            gates:
            	port[num_switches];
        }
        controller : Controller {
            parameters:
                app.total_nodes = total_nodes;
				app.num_ports = 1;
                app.num_queues = 1;
            gates:
				port_inout[controller_port];
        }

    connections allowunconnected:
    	// Spine-controller
    	Leaf[0].port[0] <--> {  datarate = 10Gbps; } <-->  controller.port_inout[0];
    	//Leaf[1].port[0] <--> {  datarate = 10Gbps; } <-->  controller.port_inout[1];

}

network Spine1_leaf1_2nodes {
    int controller_port  = 1;
    int num_spines = 1; 
    int num_leafs = 1;
    
    int total_nodes =   num_spines + num_leafs;
    submodules:
        // Right Side
        Leaf[num_leafs] : Node {
            parameters:
                @display("i=device/router, p=,,m,1,5,5");
                address = index;
                app.total_nodes = total_nodes;
                app.num_ports = total_nodes;
                app.num_queues = total_nodes;               
            gates:
            	port[num_spines];
        }
                
        Spine[num_spines] : Node {
            parameters:
                address = num_leafs + index;
                app.total_nodes = total_nodes;
                app.num_ports = 1;
                app.num_queues = 1;  
                @display("i=device/router");
            gates:
            	port[num_spines + num_leafs];
        }
        
        controller : Controller {
            parameters:
                app.total_nodes = total_nodes;
                app.num_ports = total_nodes;
                app.num_queues = total_nodes;
            gates:
				port_inout[controller_port];
        }

    connections allowunconnected:
    	// Spine-controller
    	Spine[0].port[num_spines] <--> {  datarate = 10Gbps; } <-->  controller.port_inout[0];
  	  	// Connect the Spines to the leafs.
  	  	for x = 0..num_spines-1, for y=0..num_leafs-1 {
  	 		Spine[x].port[y] <--> {  datarate = 10 Gbps; } <-->  Leaf[y].port[x];
  	  	}
}

network spine_leaf_dynamic {
    int controller_port  = 1;
    int num_spines  @prompt("Enter the number of spines"); // 3
    int num_leafs  @prompt("Enter the number of Leaf nodes"); // 9
    int total_nodes = num_spines + num_leafs;
    submodules:
        // Right Side
        Leaf[num_leafs] : Node {
            parameters:
                @display("i=device/router, p=,,m,1,5,5");
                address = index;
                app.total_nodes = total_nodes;
                app.num_ports = num_spines;
              	app.num_queues = num_spines;
            gates:
            	port[num_spines];
        }
                
        Spine[num_spines] : Node {
            parameters:
                address = num_leafs + index; // Address will  start from last leaf
                app.total_nodes = total_nodes;
                @display("i=device/router");
                app.num_ports = num_spines + num_leafs;
                app.num_queues = num_spines + num_leafs;
            gates:
            	port[num_spines + num_leafs];
        }
        
        controller : Controller {
            parameters:
                app.total_nodes = total_nodes;
                app.num_ports = 1;
                app.num_queues = 1;
            gates:
				port_inout[controller_port];
        }
        
    connections allowunconnected:
    	// Spine-controller
    	Spine[0].port[num_leafs] <--> {  datarate = 10Gbps; } <-->  controller.port_inout[0];
  	  	// Connect the Spines to the leafs.
  	  	for x = 0..num_spines-1, for y=0..num_leafs-1 {
  	 		Spine[x].port[y] <--> {  datarate = 10Gbps; } <-->  Leaf[y].port[x];
  	  	}
}

network tree_graph {
	int depth = 7;
	int branch = 2;

	// calclulate number of total nodes
	int total_nodes = 126;

	submodules:
		Leaf[total_nodes] : Node {
			parameters:
				@display("i=device/router");
				 address = index;
				 app.total_nodes = total_nodes;
				 app.num_ports = branch + 1;
                 app.num_queues = branch + 1;
			gates:
				port[branch + 1];
		}

		controller : Controller {
			parameters:
			    app.total_nodes = total_nodes;
			   	app.num_ports = branch;
                app.num_queues = branch;
			gates:
				port_inout[branch];
		}


	connections allowunconnected:

		// connects controller to switches
		for i = 0..branch-1 {
			Leaf[i].port[0] <--> { datarate = 10 Gbps; } <--> controller.port_inout[i];
		}

		// connects each node do its parent 
		for i = branch..total_nodes-1 {
			Leaf[i].port[0] <--> { datarate = 10 Gbps; } <--> Leaf[(int (i/branch))-1].port[i%branch+1];
		}
}

network fat_tree { 

	// THIS IS FOR SWITCH DEGREE=6
	// CAN SERVICE 54 END HOSTS
	// K = 2, core=1, leaf=4
	//
	int k @prompt("Enter the value of k");
	// TODO check if can use k, as a user param, and then build the formulas.
	// num_switches = 1.2 * k^2
	// num_core_switches = 0.25 * k^2;
	int num_core_switches = int (0.25*(k^2));

	// num_leaf_switches = k^2
	int num_leaf_switches = k^2;
	
	int total_num_nodes = num_core_switches + num_leaf_switches; //controller
	submodules:
		Leaf[num_leaf_switches] : Node {
			parameters:
			    @display("i=device/router, p=,,m,40,100,100");
				address = index;
				app.total_nodes = total_num_nodes;
				app.num_ports = k;
              	app.num_queues = k;
			gates:
				port[k];
		}

		// each core switch has an extra port for potentially attaching the controller]
		// port[k] is for attaching to controller
		Spine[num_core_switches] : Node {
			parameters:
			    @display("i=device/router, p=,,m,10,50,50");
			    address = num_leaf_switches + index;
			    app.total_nodes = total_num_nodes;
				app.num_ports = k+1;
              	app.num_queues = k+1;			    
			gates:
				port[k+1];
		}

		controller : Controller {
			parameters:
			    app.total_nodes = total_num_nodes;
				app.num_ports = 1;
              	app.num_queues = 1;
			gates:
				port_inout[1];
				
		}

		connections allowunconnected:
			// attach controller to controller_spine
			controller.port_inout[0] <--> { datarate = 10 Gbps; } <--> Spine[0].port[k];

			// connect spines to pods
			for i=0..num_core_switches-1, for j=0..k-1 { 
				// j = pod index
				Spine[i].port[j] <--> { datarate = 10 Gbps; } <--> Leaf[j*k + int (i/int(k/2))].port[i%int(k/2)];
			}

			// connect top of pods to bottoms of pods
			for i=0..k-1, for j=0.. int (k/2)-1, for m=0.. (int (k/2))-1 { // i = pod index
				  //j = top node index in pod
				  // m = bottom node index
						// leaf_1_index = i*k + j
						// port_1_index = k/2 + m
						// leaf_2_index = i*k + k/2 + m
						// port_2_index = j
				Leaf[i*k + j].port[int (k/2) + m] <--> { datarate = 10 Gbps; } <--> Leaf[i*k + int (k/2) + m].port[j];	
			}

}

network bus_star {
	int tree_points @prompt("Enter a value for tree-points");
	int bus_stops @prompt("Enter a value for bus-stops");

	int total_tree_points = tree_points * bus_stops;

	submodules:
		Leaf[total_tree_points] : Node {
			parameters:
			    address = index;
			    app.total_nodes = total_tree_points;
			    app.num_ports = 1;
              	app.num_queues = 1;
			gates:
				port[1];
		}
		Spine[bus_stops] : Node {
			parameters:
			    app.total_nodes = total_tree_points;
			    address = total_tree_points + index;
			    app.num_ports = tree_points + 2;
              	app.num_queues = tree_points + 2;
			    
			gates:
				port[tree_points + 2];
		}
		controller : Controller {
			parameters:
				app.total_nodes = total_tree_points + bus_stops;
				app.num_ports = 1;
              	app.num_queues = 1;
			gates:
				port_inout[1];
		}

		connections allowunconnected:
			// controller to first bus_stop
			controller.port_inout[0] <--> { datarate = 10 Gbps; } <--> Spine[0].port[0];

			// connect bus stops together
			// port 0 is towards the controller
			// port 1 is away from controller
			for i = 0..bus_stops-2 {
				Spine[i].port[1] <--> { datarate = 10 Gbps; } <--> Spine[i+1].port[0];
			}

			// connects tree_points to bus_stops
			for i = 0..total_tree_points-1 {
				Leaf[i].port[0] <--> { datarate = 10 Gbps; } <--> Spine[int(i/tree_points)].port[i%tree_points+2];
			}
}	


