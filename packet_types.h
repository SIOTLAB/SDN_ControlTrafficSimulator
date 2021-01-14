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

/*
 * Author: Ananya Gopal, Jesse Chen
 * File name: packet_types.h
 * This file will have the enum types declared for different kinds of messages
 */

#ifndef __PACKET_TYPES_H
#define __PACKET_TYPES_H
#endif

#define LINE_RATE 10000000000
#define MSG_DELAY(x) x/LINE_RATE
/* First module */
typedef enum {
    STATS_TABLE_REQ = 100,
    STATS_TABLE_REPLY,

    STATS_GROUP_DESC_REQ,
    STATS_GROUP_DESC_REPLY,

    STATS_GROUP_REQ,
    STATS_GROUP_REPLY,

    STATS_OF_METER_CONFIG_REQ,
    STATS_OF_METER_CONFIG_REPLY,

    STATS_OF_METER_REQ,
    STATS_OF_METER_REPLY,

    STATS_FLOW_REQ,
    STATS_FLOW_REPLY,

    STATS_PORT_STATS_REQ,
    STATS_PORT_STATS_REPLY,

    STATS_QUEUE_REQ,
    STATS_QUEUE_REPLY,

    PACKET_IN,
    PACKET_FLOW_MOD,
} pk_kind_t;

/* Custom packet lengths*/
#define STATS_TABLE_REQ_LEN 68
#define STATS_GROUP_DES_REQ_LEN 68
#define STATS_GROUP_REQ_LEN 76
#define STATS_OF_METER_CONFIG_REQ_LEN 76
#define STATS_OF_METER_REQ_LEN 76


#define STATS_FLOW_REQ_LEN 108
#define STATS_PORT_REQ_LEN 76
#define STATS_QUEUE_REQ_LEN 76


#define STATS_TABLE_REPLY_LEN 6372
#define STATS_GROUP_DES_REPLY_LEN 68
#define STATS_GROUP_REPLY_LEN 68
#define STATS_OF_METER_CONFIG_REPLY_LEN 68
#define STATS_OF_METER_REPLY_LEN 68

/* Second module */
typedef enum {
    BUS_STAR = 0,
    LEAF_SPINE,
    FAT_TREE,
    TREE_GRAPH
} topo_t;


#define PACKET_IN_LEN 174
#define PACKET_FLOW_MOD_LEN 170

#define QUEUE_RATE_PKTS_S 2180
/* Values for 100 Mbps */
#define QUEUE_RATE_PKTS_S_100Mbps_100FLows_SC 60444.87427
#define QUEUE_RATE_PKTS_S_100Mbps_500FLows_SC 38781.33532
#define QUEUE_RATE_PKTS_S_100Mbps_100FLows_CS 73629.02751
#define QUEUE_RATE_PKTS_S_100Mbps_500FLows_CS 73629.02751

/* Values for 10Gbps */
// Files:Flow Install: /home/ananya/Documents/ThruputSrcFiles/10Gbps/Flow/*
// PAcketLen used:averagePackLenSC10GbpsStats, averagePackLenCS10GbpsStats
#define QUEUE_RATE_PKTS_S_10G_100FLows_SC_Stats 833677.9202
#define QUEUE_RATE_PKTS_S_10G_100FLows_CS_Stats 835419.0963
#define QUEUE_RATE_PKTS_S_10G_100FLows_SC_FlowInstall 834234.3064
#define QUEUE_RATE_PKTS_S_10G_100FLows_CS_FlowInstall 837248.5868

// Files:Flow Install: /home/ananya/Documents/ThruputSrcFiles/10Gbps/Stats/*
// PAcketLen averagePackLenSC10GbpsInstall, averagePackLenCS10GbpsInstall
#define QUEUE_RATE_PKTS_S_10G_500FLows_SC_Stats 833683.4804
#define QUEUE_RATE_PKTS_S_10G_500FLows_CS_Stats 835419.0963
#define QUEUE_RATE_PKTS_S_10G_500FLows_SC_FlowInstall 834234.3064
#define QUEUE_RATE_PKTS_S_10G_500FLows_CS_FlowInstall 837248.5868

#define MSG_ID_START 500

