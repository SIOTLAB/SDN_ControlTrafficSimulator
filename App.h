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

#include <vector>
#include <omnetpp.h>
#include "Packet_m.h"
#include "packet_types.h"

#include <iostream>
#include <fstream>

#define CONTROLLER_ADDRS 2000
#define MODE_QUEUING_LATENCY 1
#define MODE_275_FLOWS 2
#define APP_STAGES 2
#define NUM_POLLING_SEQ 1
typedef std::map<int, double> ThroughputTable;


using namespace omnetpp;

/**
 * Generates traffic for the network.
 */
class App : public cSimpleModule
{


  private:
    // configuration
    int myAddress, total_nodes;
    int numFlows, numQueues, numPorts;
    int msg_id;
    int mode;

    int CONTROLLER_ID = 0;

    long double queuing_DTC = 0;
    long double queuing_DFC = 0;

    bool amIcontroller = false;
    // signals
    simsignal_t endToEndDelaySignal;
    simsignal_t hopCountSignal;
    simsignal_t sourceAddressSignal;

    /* Poll messages */
    cPar     *sendPollt;
    cPar     *sendPacketInTime;
    cPar     *sendPacketInTime275;
    // cMessage *pollMessage;
    cMessage *sendPacketIn;


  public:
    App();
    virtual ~App();
    double getThroughputFromCtlr(int x);
    ThroughputTable toController;
    ThroughputTable fromController;
     long double calculateQueuingLatencyToController(int) ;
     long double calculateQueuingLatencyFromController(int);
  protected:
    virtual void initialize(int) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual int numInitStages() const;
    Packet* sendToController(int, int, char*,pk_kind_t, int);
    Packet* sendPacketToNodes(int, int, char*, pk_kind_t, int);
    int calculateLenFlowStatsResponsePacket(int);
    int calculateLenPortStatsResponsePacket(int);
    int calculateLenQueueStatsResponsePacket(int);
    bool isPacketFromController(Packet*);
    int getControllerID();
    std::string getFileName(int topo_type, int flows, bool toCtrl);
    std::string getTopoString (int topo_type);
    void calculateDelay(std::string file_name, bool from_ctrl);
};
