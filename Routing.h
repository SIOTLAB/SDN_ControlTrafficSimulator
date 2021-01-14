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

#include <map>
#include <omnetpp.h>
#include "Packet_m.h"
#include <iostream>
#include <fstream>
#include <string>

typedef std::map<int, double> ThroughputTable;
typedef std::map<int, int> RoutingTable;
/* VAlues for 15 Mbps*/
static std::map<int, int> averagePackLenSC = {{100, 860}, {200, 1030}, {300, 1130}, {400, 1199}, {500, 1225}};
static std::map<int, double> averagePackLenCS = {{100, 85}, {200, 82.8}, {300, 80.8}, {400, 78.7}, {500, 77.3}};

/* Values for 100 Mbps*/
static std::map<int, double> averagePackLenSC100 = {{100, 206.8}, {500, 322.32}};
static std::map<int, double> averagePackLenCS100 = {{100, 169.77}, {500, 169.77}};

/* Values for 10Gbps*/
static std::map<int, double> averagePackLenSC10GbpsStats = {{100, 1499.38}, {500, 1499.37}};
static std::map<int, double> averagePackLenCS10GbpsStats = {{100, 1496.255}, {500, 1496.255}};

static std::map<int, double> averagePackLenSC10GbpsInstall = {{100, 1498.38}, {500, 1498.38}};
static std::map<int, double> averagePackLenCS10GbpsInstall = {{100, 1492.9855}, {500, 1492.9855}};
using namespace omnetpp;

/**
 * Demonstrates static routing, utilizing the cTopology class.
 */
class Routing : public cSimpleModule
{
    friend class App;
  private:
    int myAddress;
    int CONTROLLER_ID = 0;
    RoutingTable rtable;
    simsignal_t dropSignal;
    simsignal_t outputIfSignal;
    bool amIcontroller;
    long double queuing_DTC = 0;
    long double queuing_DFC = 0;
    int topology_type;
    ThroughputTable toController;
    ThroughputTable fromController;
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    std::string getFileName(int topo_type, int flows, bool toCtrl);
    std::string getTopoString (int topo_type);
    void calculateDelay(std::string file_name, bool from_ctrl);
    double getThroughputFromCtlr(int x);
    long double calculateQueuingLatencyToController(int, int) ;
    long double calculateQueuingLatencyFromController(int, int);
    int getControllerID();
};
