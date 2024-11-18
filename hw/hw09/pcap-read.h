
#ifndef __PCAP_READ_H
#define __PCAP_READ_H

/* Note: Increased this to accommodate raw WiFi packets 
     Make sure to test later */
#define DEFAULT_READ_BUFFER     2500

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct FilePcapInfo 
{
	char * 		FileName;
	char 		EndianFlip;

	/* Zero means read until the end, non-zero if limited */
	uint32_t 	MaxPackets;

	uint32_t 	Packets;
	uint32_t 	BytesRead;
};

/** Parse the start of a pcap file to determine if this is a valid pcap file 
 * and to appropriately identify the endian-ness of the file
 * @param pTheFile   A valid C-style file pointer at the start of a pcap file
 * @param pFileInfo  A valid pointer to information about the file 
 * @returns 1 if successful, 0 if unsuccessful
*/
char parsePcapFileStart (FILE * pTheFile, struct FilePcapInfo * pFileInfo);

/** Given a file that is pointing to another pcap data record, read out and extra
 * one packet from the file 
 * 
 * Note that the caller assumes responsibility for the recent memory allocation
 * that results in a packet being successfully read from the file 
 *   
 * @param pTheFile  A valid C-style file pointer pointing to a pcap record
 * @param pFileInfo A valid pointer to information about the file 
 * @returns Non-NULL allocated Packet struct pointer, NULL if unsuccessful
 */
struct Packet * readNextPacket (FILE * pTheFile, struct FilePcapInfo * pFileInfo);

/** Read a pcap file and process the packets contained within the file 
 * @param pFileInfo  Information about the file to read
 * @returns 1 if successful, 0 otherwise
*/
char readPcapFile (struct FilePcapInfo * pFileInfo);


#endif

