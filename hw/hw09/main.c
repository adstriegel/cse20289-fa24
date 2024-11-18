/* main.c : Main file for redextract */

#include <stdio.h>
#include <stdlib.h>

/* for strdup due to C99 */
char * strdup(const char *s);

#include <string.h>

#include "pcap-read.h"
#include "pcap-process.h"


int main (int argc, char *argv[])
{ 
    if(argc < 2)
    {
        printf("Usage: redextract file.pcap\n");
        return -1;
    }

    printf("MAIN: Initializing the table for redundancy extraction\n");
    initializeProcessing(DEFAULT_TABLE_SIZE);
    printf("MAIN: Initializing the table for redundancy extraction ... done\n");

    /* Note that the code as provided below does only the following 
     *
     * - Reads in a single file twice
     * - Reads in the file one packet at a time
     * - Process the packets for redundancy extraction one packet at a time
     * - Displays the end results
     */

    struct FilePcapInfo     theInfo;

    theInfo.FileName = strdup(argv[1]);
    theInfo.EndianFlip = 0;
    theInfo.BytesRead = 0;
    theInfo.Packets = 0;
    theInfo.MaxPackets = 0;

    printf("MAIN: Attempting to read in the file named %s\n", theInfo.FileName);
    readPcapFile(&theInfo);

    printf("Summarizing the processed entries\n");
    tallyProcessing();

    /* Output the statistics */

    printf("Parsing of file %s complete\n", argv[1]);

    printf("  Total Packets Parsed:    %d\n", gPacketSeenCount);
    printf("  Total Bytes   Parsed:    %lu\n", (unsigned long) gPacketSeenBytes);
    printf("  Total Packets Duplicate: %d\n", gPacketHitCount);
    printf("  Total Bytes   Duplicate: %lu\n", (unsigned long) gPacketHitBytes);

    float fPct;

    fPct = (float) gPacketHitBytes / (float) gPacketSeenBytes * 100.0;

    printf("  Total Duplicate Percent: %6.2f%%\n", fPct);


    return 0;
}
