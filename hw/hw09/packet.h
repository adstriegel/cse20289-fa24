/* packet.h : Definition file for relevant packet structers and helper functions */

#ifndef __PACKET_H
#define __PACKET_H

#include <stdint.h>
#include <sys/time.h>

#define PKT_SIZE_LIMIT  1500

/* A data packet as parsed from a pcap file 
 * 
 *  Data is a pointer to an allocated array of bytes
 *  SizeDataMax is the size of the Data buffer
 *  TimeCapture is when the packet was captured
 *  LengthIncluded is how big the packet is as captured
 *  LengthOriginal is how big the packet actually was when seen on the network
 *  
 *  LengthOriginal will always be equal or greater than LengthIncluded 
*/
struct Packet 
{
    /* A byte-wise block of data that has been allocated */
    uint8_t *   Data;

    /* Size of the allocated buffer in Data */
    uint16_t    SizeDataMax;

    /* The time when this packet was recorded */
    struct timeval      TimeCapture;

    /* The length of this packet as captured */
    uint32_t    LengthIncluded;

    /* The length as seen by the packet capture */
    uint32_t    LengthOriginal;

    /* Offset to the relevant payload */
    uint32_t    PayloadOffset;

    /* Size of the payload */
    uint32_t    PayloadSize;
};

/* Helper to do the endian magic fix */
#define endianfixs(A) ((((uint16_t)(A) & 0xff00) >> 8))
#define endianfixl(A) ((((uint32_t)(A) & 0xff000000) >> 24))

/* Allocate a new packet structure with the specified data buffer size */
struct Packet * allocatePacket (uint16_t DataSize);

/* Discard the packet and free back up the memory */
void discardPacket (struct Packet * pPacket);

#endif