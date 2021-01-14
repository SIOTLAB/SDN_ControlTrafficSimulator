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

#include "App.h"
#include "Routing.h"
Define_Module(App);

App::App()
{
    sendPacketIn = nullptr;
}

App::~App()
{
    cancelAndDelete(sendPacketIn);
}

/*
 * App module has a two-stage initialization.
 * */
int App::numInitStages() const {
    return (APP_STAGES);
}
/*
 * In the Topology, the IDs are dynamic, so we
 * first populate the Controller ID.
 */
int App::getControllerID() {
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
 * Param explanation to understand the init fn:
 * mode: We can specify the mode of the init fn.
 *       mode MODE_QUEUING_LATENCY - We simulate the 4 packet sequence between each node and controller.
 *       mode MODE_275_FLOWS: This is to simulate a realistic 275 messages from each node to the controller, per second.
 *                            ALso gives us a realistic latency graph
 *
 *
 * */
void App::initialize(int stage)
{
    mode = par("mode");
    EV << "\n\n\n" << "Initialized:" << getModuleType() << "-Ports:"
                << numPorts<< ", Parent:" << getParentModule() << ",mode:" << mode << "\n";

    if (stage == 1) {
        cModule *targetModule = getParentModule()->getSubmodule("routing");
        Routing *target = check_and_cast<Routing *>(targetModule);
        EV << "Myaddress:" << myAddress << "-" << target->queuing_DFC;
        EV << "Myaddress:" << myAddress << "-" << target->queuing_DTC;
        queuing_DFC = target->queuing_DFC;
        queuing_DTC = target->queuing_DTC;
    }

    if (stage == 0) {
        myAddress = par("address");
        numFlows = getParentModule()->par("num_flows");
        numQueues = par("num_queues");
        numPorts = par("num_ports");
        total_nodes = par("total_nodes");
        sendPollt = &par("sendPollTime");
        sendPacketInTime = &par("sendPacketInTime");
        sendPacketInTime275 = &par("sendPacketInTime275");

        WATCH(myAddress);

        endToEndDelaySignal = registerSignal("endToEndDelay");
        hopCountSignal = registerSignal("hopCount");
        sourceAddressSignal = registerSignal("sourceAddress");


        CONTROLLER_ID = getControllerID();

        if (!CONTROLLER_ID) {
            throw cRuntimeError("Invalid ID for Controller");
        }
        /* Send the entire Sequence */
        if (getParentModule()->getId() == CONTROLLER_ID) {
            amIcontroller = true;
            /* We calculated the thruput by sending an entire polling sequence between
             * the switch and the controller. Please check README.
             * This poll message was part of that. */
            //pollMessage = new cMessage("pollMessage");
            //scheduleAt(simTime(), pollMessage);
        }

        switch (mode) {
        case MODE_QUEUING_LATENCY:
            /* Send 4-packet entire Sequence */
            msg_id = MSG_ID_START;
            if (!amIcontroller) {
                /* Switch schedules a MSG every second.*/
                /* Schedule a POLL message for packet_in */
                sendPacketIn = new cMessage("sendPacketIn");
                scheduleAt(simTime() + myAddress*0.5, sendPacketIn);
            }
            break;
        case MODE_275_FLOWS:
            if (!amIcontroller) {
                /* Switch schedules a MSG every second.*/
                /* Schedule a POLL message for packet_in */
                sendPacketIn = new cMessage("sendPacketIn");
                scheduleAt(simTime(), sendPacketIn);
            }
            break;
        }
    }
}

/*
 * For a STATS_FLOW_REPLY Packet, we simulate the length with the
 * following formula:
 *       STATS_FLOW_REPLY length in bytes = 68 + (104 * num_flows)
 */
int App::calculateLenFlowStatsResponsePacket(int num_flows) {
    return (68 + (104*num_flows));
}

/*
 * For a STATS_PORT_STATS_REPLY Packet, we simulate the length with the
 * following formula:
 *       STATS_PORT_STATS_REPLY length in bytes = 68 + (112*num_ports)
 */
int App::calculateLenPortStatsResponsePacket(int num_ports) {
    return (68 + (112*num_ports));
}

/*
 * For a STATS_QUEUE_REPLY Packet, we simulate the length with the
 * following formula:
 *       STATS_QUEUE_REPLY length in bytes = 108 * (40*num_queues)
 */
int App::calculateLenQueueStatsResponsePacket(int num_queues) {
    return (108 + (40*num_queues));
}

Packet* App::sendToController(int src, int len, char pk_name[], pk_kind_t pk_type, int id) {
    Packet *new_packet = new Packet(pk_name);
    new_packet->setByteLength(len);
    new_packet->setKind(pk_type);
    new_packet->setSrcAddr(src);
    new_packet->setDestAddr(CONTROLLER_ADDRS);
    new_packet->setPacket_id(id);
    return new_packet;
}

Packet* App::sendPacketToNodes(int dest, int len, char pk_name[], pk_kind_t pk_type, int id) {
    Packet *pk = new Packet(pk_name);
    pk->setByteLength(len);
    pk->setKind(pk_type);
    pk->setSrcAddr(CONTROLLER_ADDRS);
    pk->setDestAddr(dest);
    pk->setPacket_id(id);
    return pk;
}

bool App::isPacketFromController(Packet* pk) {
    if (pk->getSrcAddr() != CONTROLLER_ADDRS)
    {
        EV << "Packet not received from Controller, discarding" << endl;
        return (false);
    }
    return (true);
}
/*
 *
 *  Packet routine:
 *  1. Switch sends to controller
 *  2. controller sends to Switch
 *  3. controller sends a bigger packet to Switch
 *  4. Switch sends reply to controller.
 * */
void App::handleMessage(cMessage *msg)
{

    Packet    *pk, *pk_2;
    int       size = 0;
    pk_kind_t reply_type, incoming_packet_type;
    char      pk_out_msg[40];
    double    tx_delay = 0.0;
    int       src_addr, msg_id_send;

    bool should_return = false;
    bool isParentController = getParentModule()->getId() == CONTROLLER_ID;
    bool isSelfMessage = msg->isSelfMessage();

    // Every time period, send a packet_in to the controller
    if (isSelfMessage) {
        if (isParentController) {
            delete msg;
            return;
        }

        if ((msg == sendPacketIn) && (mode == MODE_QUEUING_LATENCY)) {
            sprintf(pk_out_msg, "%d_%d_%d", msg_id, myAddress, CONTROLLER_ADDRS);
            size = PACKET_IN_LEN;
            reply_type = PACKET_IN;
            tx_delay = (double (size)/double(LINE_RATE)) +  queuing_DTC;
            //EV << "App: " << msg_id << "generating packet from:" << myAddress << "-" << pk_out_msg << ": Qdelay is" << tx_delay << endl;
            //EV << "App: " << msg_id << ":Sending A PACKET of type: " << reply_type << "to controller "<< size << "--delay" << tx_delay;
            pk = sendToController(myAddress, size, pk_out_msg, reply_type, msg_id);
            msg_id++;
            sendDelayed(pk, tx_delay, "out");
            // The chain of events should happen every sendPacketInTime seconds
            if (sendPacketIn->isScheduled())
                cancelEvent(sendPacketIn);
            scheduleAt(simTime() + sendPacketInTime->doubleValue(), sendPacketIn);

        }

        if ((msg == sendPacketIn) && (mode==MODE_275_FLOWS)) {
            msg_id = 0;
            while (msg_id < NUM_POLLING_SEQ) {
                sprintf(pk_out_msg, "%d_%d_%d", msg_id, myAddress, CONTROLLER_ADDRS);
                size = PACKET_IN_LEN;
                reply_type = PACKET_IN;
                Packet *packet = sendToController(myAddress, size, pk_out_msg, reply_type, msg_id);
                msg_id++;
                send(packet, "out");
            }
            // The chain of events should happen every sendPacketInTime275 seconds
            if (sendPacketIn->isScheduled())
                cancelEvent(sendPacketIn);
            scheduleAt(simTime() + sendPacketInTime275->doubleValue(), sendPacketIn);
        }
        return;
    }

    // If we reach this point, we have already scheduled the self messages.
    // We have a received a new message, and the processing of that starts here.

    incoming_packet_type = static_cast<pk_kind_t> (msg->getKind());
    pk = check_and_cast<Packet *>(msg);

    src_addr = pk->getSrcAddr();
    msg_id_send = pk->getPacket_id();

    //EV << "App: " << msg_id << "App:Received x message at" << myAddress << "-"<< msg->getDisplayString() << msg->getKind() << endl;
    // Processing all messages at the Nodes
    if (!isParentController){
        //EV << "App: " << msg_id << "App:Received x message at" << myAddress << "-"<< msg->getDisplayString() << msg->getKind() << endl;
        if (!isPacketFromController(pk)) {
            //EV << " Packet not from Controller."<< endl;
            delete msg;
            return;
        }

        switch (incoming_packet_type){
            case PACKET_FLOW_MOD:
                //EV <<"No reply needed";
                should_return = true;
                break;
            case STATS_FLOW_REQ:
                size = this->calculateLenFlowStatsResponsePacket(numFlows);
                reply_type = STATS_FLOW_REPLY;
                sprintf(pk_out_msg, "%d_%d_%d", msg_id_send, myAddress, CONTROLLER_ADDRS);
                if (mode==MODE_QUEUING_LATENCY) {
                    tx_delay = (double (size)/double(LINE_RATE)) +  queuing_DTC;
                    EV << "App: " << msg_id << "App:Sending A PACKET of type: " << reply_type << "to controller "<< size << "--delay" << tx_delay;
                    pk = sendToController(myAddress, size, pk_out_msg, reply_type, msg_id_send);
                    sendDelayed(pk, tx_delay, "out");
                } else if (mode==MODE_275_FLOWS && msg_id_send==0) {
                    //EV << "App: " << msg_id << "App:Sending A PACKET of type: " << reply_type << "to controller "<< size << "--delay" << tx_delay;
                    pk = sendToController(myAddress, size, pk_out_msg, reply_type, msg_id_send);
                    send(pk, "out");
                }
                should_return = true;
                break;
            default:
                should_return = true;
                break;
        }

        if (should_return) {
            delete msg;
            return;
        }
    } else { //  The Controller's processing
        if (mode == MODE_QUEUING_LATENCY) {
            switch (incoming_packet_type){
                case PACKET_IN:
                    size = PACKET_FLOW_MOD_LEN;
                    reply_type =  PACKET_FLOW_MOD;
                    sprintf(pk_out_msg, "%d_%d_%d", msg_id_send, myAddress, src_addr);

                    tx_delay = double (size)/double(LINE_RATE);
                    //EV << "App: " << msg_id <<"-tx-delay" << tx_delay;
                    pk = sendPacketToNodes(src_addr, size, pk_out_msg, reply_type, msg_id_send);
                    sendDelayed(pk, tx_delay, "out");
                    //send(pk, "out");
                    // Once first packet is sent, we will send another one, right after.
                    size = STATS_FLOW_REQ_LEN;
                    reply_type =  STATS_FLOW_REQ;

                    tx_delay += double (size)/double(LINE_RATE);
                    //EV << "App: " << msg_id << "-tx-delay" << tx_delay;
                    sprintf(pk_out_msg, "%d_%d_%d", msg_id_send, myAddress, src_addr);
                    pk_2 = sendPacketToNodes(src_addr, size, pk_out_msg, reply_type, msg_id_send);
                    sendDelayed(pk_2, tx_delay, "out");
                    break;
                case STATS_FLOW_REPLY:
                    //EV << "Polling sequence over.";
                    break;

                default:
                    //EV << "Invalid Packet... Dropping";
                    should_return = true;
                    break;
            }
        } else if (mode==MODE_275_FLOWS) {
            switch (incoming_packet_type){
                case PACKET_IN:
                    size = PACKET_FLOW_MOD_LEN;
                    reply_type =  PACKET_FLOW_MOD;
                    sprintf(pk_out_msg, "%d_%d_%d", msg_id_send, myAddress, src_addr);
                    pk = sendPacketToNodes(src_addr, size, pk_out_msg, reply_type, msg_id_send);
                    send(pk, "out");
                    if (msg_id_send==0) {
                        size = STATS_FLOW_REQ_LEN;
                        reply_type =  STATS_FLOW_REQ;
                        sprintf(pk_out_msg, "%d_%d_%d", msg_id_send, myAddress, src_addr);
                        pk_2= sendPacketToNodes(src_addr, size, pk_out_msg, reply_type, msg_id_send);
                        send(pk_2, "out");
                    }
                    break;
                case STATS_FLOW_REPLY:
                    //EV << "Polling sequence over.";
                    break;

                default:
                    //EV << "Invalid Packet... Dropping";
                    should_return = true;
                    break;
            }
        }

        if (should_return) {
            delete msg;
            return;
        }
        delete msg;
    }
}

