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

#include "Routing.h"
#include "App.h"
Define_Module(Routing);

/*
 * In the Topology, the IDs are dynamic, so we
 * first populate the Controller ID.
 */
int Routing::getControllerID() {
    cModule *network = cSimulation::getActiveSimulation()->getSystemModule();
    for (SubmoduleIterator it(network); !it.end(); ++it) {
        cModule * mod = *it;
        if (!strcmp(mod->getName(), "controller")) {
           return (mod->getId());
        }
    }
    return (0);
}

/*
 * We store the Thruput src files as
 * leaf_spine_xxxx
 * fat_tree_xxxxxx
 * tree_xxxxxxxxxx
 * bus_star_xxxxxx
 */
std::string Routing::getTopoString (int topo_type) {

    std::string ret;
    switch (topo_type){
        case 0:
            ret = "leaf_spine";
            break;
        case 1:
            ret = "fat_tree";
            break;
        case 2:
            ret = "tree";
            break;
        case 3:
            ret = "bus_star";
            break;
    }

    return (ret);
}


/*
 * One of the Utility functions to read throughput source files
 * This function creates a dynamic string for the thruput file.
 */
std::string Routing::getFileName(int topo_type, int flows, bool from_ctrl) {
    std::string file_location = "/home/ananya/Documents/ThruputSrcFiles/10Gbps/Stats/";
    std::string dir = from_ctrl ? "cs"  : "sc";
    std::string ext = ".csv";
    std::string file_name = file_location + getTopoString(topo_type) + "_" + std::to_string(flows) + "-" + dir + ext;
    return (file_name);
}

/*
 * One of the Utility functions to read throughput source files
 * This function reads each entry in the thruput src file, once the file name is supplied.
 */
void Routing::calculateDelay(std::string file_name, bool from_ctrl){
    std::string thruEntry;
    std::ifstream thruputDataFile(file_name);
    while (getline (thruputDataFile, thruEntry)) {
        // EV << thruEntry << endl; // one entry looks like this: 64,121722.8689183932
        int pos = thruEntry.find(",");
        int node_id = std::stoi(thruEntry.substr(0, pos));
        EV << node_id <<endl;
        std::string sub = thruEntry.substr(pos + 1);
        double del = std::stod(sub);

        /* fromController stores the thruput values observed
         * from the direction of the controller
         * toController stores the thruput values observed
         * packets that traverse towards the controller. */
        if (from_ctrl) fromController[node_id] = del;
        else toController[node_id] = del;
    }
    thruputDataFile.close();
    return;
}

/*
 * For 10Gbps,
 * We calculate the baseline thruput by using the baseline csv files.
 * We are running two tests -
 * ===== One for FLow installation (with normal csv files, and baseline)
 * ===== One for Stats collection (with normal csv files, and baseline)
 *
 * The thruputs and packetlengths will be the same for one test of normal csv and baseline,
 * but will change with the number of flows, the direction of the flow and the type of test.
 *
 *
 * For 100Mbps, we use Qrates as QUEUE_RATE_PKTS_S_100Mbps_100FLows_SC/CS
 * For 10Gbps, QUEUE_RATE_PKTS_S_10G_100FLows_CS_Stats/SC
 * */
long double Routing::calculateQueuingLatencyToController(int address, int flows)  {
    double qRate = 0.0;
    switch (flows) {
    case 100:
        qRate = (double)QUEUE_RATE_PKTS_S_10G_100FLows_SC_Stats;
        break;
    case 500:
        qRate = (double)QUEUE_RATE_PKTS_S_10G_500FLows_SC_Stats;
        break;
    }

    double thruputVal = (double)toController[address]/ (double)(averagePackLenSC10GbpsStats[flows]*8);
    long double delay = (double)1/(double)(qRate - thruputVal);
    //long double delay = (double)1/(double)(QUEUE_RATE_PKTS_S);
    EV << "\ntoController:" << qRate << ",Thruput:" << thruputVal << ",Delay:" << delay <<endl;
    return (delay);
}

/*
 *  calculateQueuingLatencyFromController(int address, int flows)
 *  params: address of the node, number of flows.
 *  For details on the formula, read README
 */
long double Routing::calculateQueuingLatencyFromController(int address, int flows) {
    double qRate = 0.0;
    switch (flows) {
    case 100:
        qRate = (double)QUEUE_RATE_PKTS_S_10G_100FLows_CS_Stats;
        break;
    case 500:
        qRate = (double)QUEUE_RATE_PKTS_S_10G_500FLows_CS_Stats;
        break;
    }
    double thruputVal = (double)fromController[address]/(double)(averagePackLenCS10GbpsStats[flows]*8);
    EV << "\nQRate:" << ",fromController:" << (double)fromController[address] << endl;
    long double delay = (double)1/(double)(qRate - thruputVal);
    EV << "\nQRate:FromCtlr:" << qRate << ",Thruput:" << thruputVal << ",Delay:" << delay <<endl;
    return (delay);
}

void Routing::initialize()
{
    CONTROLLER_ID = getControllerID();

    if (!CONTROLLER_ID) {
        throw cRuntimeError("Invalid ID for Controller");
    }

    myAddress = getParentModule()->par("address");
    dropSignal = registerSignal("drop");
    outputIfSignal = registerSignal("outputIf");
    topology_type = par("topo");

    cTopology *topo = new cTopology("topo");

    // EV << "\n\n " << getNedTypeName() << "--->" << getParentModule()->getComponentType()->getFullName() << "\n";
    topo->extractByNedTypeName(cStringTokenizer("Node Controller").asVector());
    cTopology::Node *thisNode = topo->getNodeFor(getParentModule());

    /* Create the routing table */
    for (int i = 0; i < topo->getNumNodes(); i++) {
        if (topo->getNode(i) == thisNode)
            continue;  // skip ourselves
        topo->calculateUnweightedSingleShortestPathsTo(topo->getNode(i));
        if (thisNode->getNumPaths() == 0) continue;
        cGate *parentModuleGate = thisNode->getPath(0)->getLocalGate();
        int gateIndex = parentModuleGate->getIndex();
        int address = topo->getNode(i)->getModule()->par("address");
        rtable[address] = gateIndex;
        // EV << "  towards address " << address << " gateIndex is " << gateIndex << endl;
    }

    delete topo;

    if (getParentModule()->getId() != CONTROLLER_ID) {

        /*
         * Queuing delays would differ with the number of flows.
         * We get the number of flows from the parameters.
         */
        int flows = getParentModule()->par("num_flows");


        /*
         * First calculate the delay in the direction of
         * "from the controller to the switch."
         *  [Controller] ---> [Spine]---> [Leaf]
         *  pk ---------------------------->
         * We store these delays in fromController.
         */
        std::string file_name = getFileName(topology_type, flows, true);
        calculateDelay(file_name, true);

        /*
         * Then calculate the delay "TO the controller from the switch."
         *  [Controller] ---> [Spine]---> [Leaf]
         *  <-------------------------------pk
         * We store these delays per switch in toController
         */
        file_name = getFileName(topology_type, flows, false);
        calculateDelay(file_name, false);

        /* We then calculate the final Queuing latencies from fromController
         * and toController, PER switch, for a given number of "flows". */
        queuing_DTC = calculateQueuingLatencyToController(myAddress, flows);
        queuing_DFC = calculateQueuingLatencyFromController(myAddress, flows);
    } else {

        /* For a controller, these delays are 0.*/
        queuing_DTC = 0;
        queuing_DFC = 0;
    }
}

/* This routine is called for EACH message received by the routing module.
 */
void Routing::handleMessage(cMessage *msg)
{
    Packet *pk = check_and_cast<Packet *>(msg);
    int destAddr = pk->getDestAddr();
    int srcAddr = pk->getSrcAddr();
    int pk_id = pk->getPacket_id();
    long double delay_pack = 0;

    EV << "Routing at" <<myAddress << "-"  << pk_id << "-"<< pk->getName() << "-"<< myAddress << "="<< destAddr << endl;
    EV << "Routing at" <<myAddress << "-"  << pk_id << "-"<<"Routing:queuing_DTC:" << queuing_DTC ;
    EV << "Routing at" <<myAddress << "-" << pk_id << "-"<< "Routing:queuing_DFC:" << queuing_DFC << endl;

    /* This message was intended for us, let us process it.
     * We send it to OUR Application module, which is connected to Routing's localOut.
     * Application module will read, what kind of a packet it is, and will send
     * the appropriate packet response.
     */
    if (destAddr == myAddress) {
        send(pk, "localOut");
        emit(outputIfSignal, -1);
        return;
    }

    /* Route/Forward the message as necessary. */
    RoutingTable::iterator it = rtable.find(destAddr);

    /* Could not find next hop. */
    if (it == rtable.end()) {
        emit(dropSignal, (long)pk->getByteLength());
        delete pk;
        return;
    }

    /* The code for forwarding is here: */
    int outGateIndex = (*it).second;
    pk->setHopCount(pk->getHopCount()+1);
    emit(outputIfSignal, outGateIndex);

    /* Here is where we plug in our delay simulation, before forwarding the packet.*/
    if (srcAddr == myAddress) {
        /* If the packet was generated by this node, we will only have transmission delay, not the queuing delay. */
        delay_pack = 0;
        send(pk, "out", outGateIndex);
    } else {
        /*
         * If the packet is forwarded by this node, check which direction it is going to.
         * If it is coming from the controller, we plug in delay fromCtlr
         * If it is going to the controller, we plug in delay toController.
         */
        if (destAddr == CONTROLLER_ADDRS) {
            delay_pack = queuing_DTC;
        } else {
            delay_pack = queuing_DFC;
        }

        /* We then plug in the transmission delay */
        int size = pk->getByteLength();
        delay_pack += (double (size)/double(LINE_RATE));

        /* Omnet lets us send with a delay by using the sendDelayed function */
        sendDelayed(pk, delay_pack, "out", outGateIndex);
    }
}

