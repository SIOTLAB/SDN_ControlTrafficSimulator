# SDN_ControlTrafficSimulator

Steps to run:

~# After installing Omnetpp, clone the repo. 

~# make, and build
fogSimulation$ opp_makemake -f 

fogSimulation$ ls 

App.cc App.ned controller.h fogNetLeafSpine.ned Makefile omnetpp.ini Packet_m.h packet_types.h Qu.h README.md Routing.h App.h controller.cc controller.ned IApp.ned Node.ned Packet_m.cc Packet.msg Qu.cc Qu.ned Routing.cc Routing.ned 

fogSimulation$ make 

MSGC: 
Packet.msg 
App.cc 
controller.cc 
Qu.cc 
Routing.cc 
Packet_m.cc 
Creating executable: out/gcc-release//fogSimulation

~# Then run the fogSImulation exe 
fogSimulation$ ./fogSimulation 

OMNeT++ Discrete Event Simulation (C) 1992-2019 Andras Varga, OpenSim Ltd. Version: 5.6.2, 
build: 200518-aa79d0918f, edition: Academic Public License -- NOT FOR COMMERCIAL USE 
See the license for distribution terms and warranty disclaimer

Setting up Qtenv...

Loading NED files from .: 7

Loading images from './bitmaps': : 0 Loading images from './images': : 0 Loading images from '/home/ananya/Documents/omnetpp-5.6.2/images': : 0 abstract/: 90 background/: 4 block/: 320 device/: 195 logo/: 1 maps/: 9 misc/: 70 msg/: 55 old/: 111 status/*: 28

~# This will open a GUI. 
~# Input the topology: spine-leaf/Fat-Tree/Tree/Bus-Star 
~# Input the number of flows to initiated by each node (Click on the "Use this value for all similar parameters")

