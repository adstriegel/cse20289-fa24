

#ifndef __PCAP_PROCESS_H
#define __PCAP_PROCESS_H

#include <stdint.h>

#include "packet.h"

#define DEFAULT_TABLE_SIZE  512
#define MIN_PKT_SIZE        128

/* Global Counters for Summary */

/* How many packets have we seen? */
extern uint32_t        gPacketSeenCount;

/* How many total bytes have we seen? */
extern uint64_t        gPacketSeenBytes;        

/* How many hits have we had? */
extern uint32_t        gPacketHitCount;

/* How much redundancy have we seen? */
extern uint64_t        gPacketHitBytes;

/* Simple data structure for tracking redundancy */
struct PacketEntry
{
    struct Packet * ThePacket;

    /* How many times has this been a hit? */
    uint32_t        HitCount;

    /* How much data would we have saved? */
    uint32_t        RedundantBytes;
};

/* Our big table for recalling packets */
extern struct PacketEntry *    BigTable; 
extern int    BigTableSize;
extern int    BigTableNextToReplace;

char initializeProcessing (int TableSize);

void processPacket (struct Packet * pPacket);

void tallyProcessing ();

#endif
