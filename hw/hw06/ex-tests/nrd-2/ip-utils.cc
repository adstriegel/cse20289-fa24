/* ip-utils.cc
 **********************************************************
 * This code is part of the Scalability in a Box (ScaleBox)
 * system developed at the University of Notre Dame. 
 * 
 * For additional information, please visit the NetScale
 * laboratory webpage at:
 *    http://gipse.cse.nd.edu/NetScale
 *
 * PI:       Dr. Aaron Striegel    striegel@nd.edu
 * 
 * Students: Dave Salyers (lead)   Yingxin Jiang
 *           Xiaolong Li           Jeff Smith
 * 
 **********************************************************
 * $Revision: 1.3 $  $Date: 2008/04/09 02:53:17 $
 **********************************************************
 */

#include <netinet/in.h>
#include <iostream>
using namespace std;

#include <vector>
using namespace std;

#include <stdio.h>

#include "ip-utils.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////
// char		isPacketIP (char * pPacket, int nLength)
//
// See header file / doxygen docs
//

char isPacketIP (char * pPacket, int nLength) {
	if(pPacket[OFFSET_ETH_TYPELEN] == ETH_TYPELEN_IP_B1 && pPacket[OFFSET_ETH_TYPELEN+1] == ETH_TYPELEN_IP_B2) {
		return 1;
	} else {
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

char isPacketIPv4IPsec (char * pPacket, int nLength) {
	if(pPacket[L2_OFFSET+OFFSET_IPV4_PROTOCOL] == IP_PROTOCOL_IPSEC_ESP ||
	   pPacket[L2_OFFSET+OFFSET_IPV4_PROTOCOL] == IP_PROTOCOL_IPSEC_AH ) {
		return 1;
	} else {
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// char		isPacketARP (char * pPacket, int nLength)
//
// See header file / doxygen docs
//

char isPacketARP (char * pPacket, int nLength) {
	if(pPacket[OFFSET_ETH_TYPELEN] == ETH_TYPELEN_ARP_B1 && pPacket[OFFSET_ETH_TYPELEN+1] == ETH_TYPELEN_ARP_B2) {
		return 1;
	} else {
		return 0;
	}
}

string getEtherMac_Dst (char* pPacket) {
    string  sResult;
    char    szTemp[18];
    char* pData = pPacket + OFFSET_ETH_DSTMAC;

    snprintf(szTemp, sizeof(szTemp), "%02x:%02x:%02x:%02x:%02x:%02x", 
	     (unsigned char)pData[0], (unsigned char)pData[1], (unsigned char)pData[2], (unsigned char)pData[3], (unsigned char)pData[4], (unsigned char)pData[5]);
    sResult = szTemp;

    return sResult;
}


string getEtherMac_Src (char* pPacket) {
    string  sResult;
    char    szTemp[18];
    char* pData = pPacket + OFFSET_ETH_SRCMAC;

    snprintf(szTemp, sizeof(szTemp), "%02x:%02x:%02x:%02x:%02x:%02x",
	     (unsigned char)pData[0], (unsigned char)pData[1], (unsigned char)pData[2], (unsigned char)pData[3], (unsigned char)pData[4], (unsigned char)pData[5]);
    sResult = szTemp;

    return sResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// char		getIPVersion (char * pPacket, int nLength) 
//
// See header file / doxygen docs
//

char getIPVersion (char * pPacket, int nLength) {
	char		theVer;

	theVer = (pPacket[L2_OFFSET] & 0xF0) >> 4;
	theVer = theVer & 0x0F;
	
	return theVer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// char		isTCP (char * pPacket, int nLength)
//
// See header file / doxygen docs
//

char isTCP (char * pPacket, int nLength) {
	if(pPacket[ETHERNET_LENGTH+OFFSET_IPV4_PROTOCOL] == IP_PROTOCOL_TCP) {
		return 1;
	} else {
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// char		isUDP (char * pPacket, int nLength)
//
// See header file / doxygen docs
//

char isUDP (char * pPacket, int nLength) {
	if(pPacket[ETHERNET_LENGTH+OFFSET_IPV4_PROTOCOL] == IP_PROTOCOL_UDP) {
		return 1;
	} else {
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

char * getIPv4_Address_Dst (char * pPacket)
{
	return pPacket+ETHERNET_LENGTH+OFFSET_IPV4_PROTOCOL_DSTIP;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

char * getIPv4_Address_Src (char * pPacket)
{
	return pPacket+ETHERNET_LENGTH+OFFSET_IPV4_PROTOCOL_SRCIP;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned char getIPv4Protocol (char * pPacket) {
	return (unsigned char) pPacket[ETHERNET_LENGTH+OFFSET_IPV4_PROTOCOL];	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// unsigned short	getPacketLength (char * pPacket, int nLength) 
//
// See header file / doxygen docs
//

unsigned short getPacketLength (char * pPacket, int nLength) {
	unsigned short	nLen;
	
	// Typecast magic to get the 16 bit length out of the IPv4 header
	nLen = ntohs( *((unsigned short *) (pPacket+L2_OFFSET+OFFSET_IPV4_LENGTH)));

	return nLen;
}

unsigned short getIPv4_ID (char * pPacket, int nLength)
{
	unsigned short	nID;

	// Typecast magic to get the 16 bit length out of the IPv4 header
	nID = ntohs( *((unsigned short *) (pPacket+L2_OFFSET+OFFSET_IPV4_ID)));

	return nID;
}

void setPacketLength (char * pPacket, unsigned short nLength) {
	unsigned short	* pIPLength;
	
	pIPLength = (unsigned short *) (pPacket+L2_OFFSET+OFFSET_IPV4_LENGTH);
	
	*pIPLength = htons(nLength);	
}

void setIP_ID (char * pPacket, unsigned short nID) {
	unsigned short * pIPID;
	
	pIPID = (unsigned short *) (pPacket+L2_OFFSET+OFFSET_IPV4_ID);	
	
	*pIPID = htons(nID);	
}

unsigned short getTCP_SrcPort (char * pPacket) {
	unsigned short	nPort;
	int		nOffset;

	if(hasIPOptions(pPacket, -1)) {		
		nOffset = getIP_HeaderSize(pPacket, -1);
	} else {
		nOffset = L3_IPV4_OFFSET;
	}

	memcpy(&nPort, pPacket+L2_OFFSET+nOffset+OFFSET_TCP_SRCPORT, 2);
	nPort = ntohs(nPort);	
	
	return nPort;
}

unsigned short getTCP_DstPort (char * pPacket) {
	unsigned short	nPort;

	int		nOffset;

	if(hasIPOptions(pPacket, -1)) {		
		nOffset = getIP_HeaderSize(pPacket, -1);
	} else {
		nOffset = L3_IPV4_OFFSET;
	}

	
	memcpy(&nPort, pPacket+L2_OFFSET+nOffset+OFFSET_TCP_DSTPORT, 2);
	nPort = ntohs(nPort);	
	
	return nPort;	
	
}

unsigned int getTCP_SeqNumber (char * pPacket, int nLength) {
	unsigned int		lSeqNum;

	int		nOffset;

	if(hasIPOptions(pPacket, nLength)) {		
		nOffset = getIP_HeaderSize(pPacket, nLength);
	} else {
		nOffset = L3_IPV4_OFFSET;
	}
	
	lSeqNum = ntohl( *((unsigned int *) (pPacket+L2_OFFSET+nOffset+OFFSET_TCP_SEQNUM)));
	
	return lSeqNum;
}

unsigned int getTCP_AckNumber (char * pPacket, int nLength) {
	unsigned int		lAckNum;

	int		nOffset;

	if(hasIPOptions(pPacket, nLength)) {		
		nOffset = getIP_HeaderSize(pPacket, nLength);
	} else {
		nOffset = L3_IPV4_OFFSET;
	}
	
	lAckNum = ntohl( *((unsigned int *) (pPacket+L2_OFFSET+nOffset+OFFSET_TCP_ACKNUM)));
	
	return lAckNum;
}

uint32_t getTCP_Ts (char * pData, int nLength) {
	uint32_t Ts;

	int		nOffset, nDiff;

	nDiff = getTCP_HeaderSize(pData,nLength) - 20;

	// Any TCP options present?
	if(nDiff == 0) {
		//cout << "    ** No options!" << endl;
		return 0;
	}
	if(hasIPOptions(pData, nLength)) {		
		nOffset = getIP_HeaderSize(pData, nLength);
	} else {
		nOffset = L3_IPV4_OFFSET;
	}

    //for(int i=0;i<nDiff;i++){
    //        printf("%d->%d ",i,pData[L2_OFFSET+getIP_HeaderSize(pData, nLength)+20+i] & 0xFF);
    //}
    Ts =      pData[L2_OFFSET+getIP_HeaderSize(pData, nLength)+20+4] << 24
            |(pData[L2_OFFSET+getIP_HeaderSize(pData, nLength)+20+5] & 0xFF) << 16
            |(pData[L2_OFFSET+getIP_HeaderSize(pData, nLength)+20+6] & 0xFF) << 8
            |(pData[L2_OFFSET+getIP_HeaderSize(pData, nLength)+20+7] & 0xFF);
	
	return Ts;
}
void writeTCP_SeqNumber (unsigned int nSeqNum, char * pPacket, int nLength) {
	int		nOffset;

	if(hasIPOptions(pPacket, nLength)) {		
		nOffset = getIP_HeaderSize(pPacket, nLength);
	} else {
		nOffset = L3_IPV4_OFFSET;
	}
	
	nSeqNum = htonl(nSeqNum);
	
	memcpy(pPacket+L2_OFFSET+nOffset+OFFSET_TCP_SEQNUM, &nSeqNum, 4);		
}

void writeTCP_TimestampOption(unsigned int nTSval, unsigned int nTSecr, char * pPacket, int nLength ){
	unsigned short 		nDiff;
	
	nDiff = getTCP_HeaderSize(pPacket,nLength) - 32;

	// Any TCP options present?
	if(nDiff < 0) {
		cerr << "Error: ** No options exist when write Timestamp option with HeaderSize of "<< nDiff + 32 << endl;
		return ;
	}

	int		nOffset;
	if(hasIPOptions(pPacket, nLength)) {		
		nOffset = getIP_HeaderSize(pPacket, nLength);
	} else {
		nOffset = L3_IPV4_OFFSET;
	}
	
	nTSval = htonl(nTSval);
	nTSecr = htonl(nTSecr);
	
            /* printf("\nThe options: before writeTCP_TimestampOption\n"); */
            /* for(int i = 0;i<10;i++) */
            /*         printf("%x ",pPacket[L2_OFFSET+nOffset+OFFSET_TCP_TIMESTAMP_TSVAL - 2 +i]); */
	memcpy(pPacket+L2_OFFSET+nOffset+OFFSET_TCP_TIMESTAMP_TSVAL, &nTSval, 4);		
	memcpy(pPacket+L2_OFFSET+nOffset+OFFSET_TCP_TIMESTAMP_TSECR, &nTSecr, 4);		
            /* printf("\nThe options: after writeTCP_TimestampOption\n"); */
            /* for(int i = 0;i<10;i++) */
            /*         printf("%x ",pPacket[L2_OFFSET+nOffset+OFFSET_TCP_TIMESTAMP_TSVAL - 2 +i]); */

}

void writeTCP_AckNumber (unsigned int nSeqNum, char * pPacket, int nLength) {
	int		nOffset;

	if(hasIPOptions(pPacket, nLength)) {		
		nOffset = getIP_HeaderSize(pPacket, nLength);
	} else {
		nOffset = L3_IPV4_OFFSET;
	}
	
	nSeqNum = htonl(nSeqNum);
	
	memcpy(pPacket+L2_OFFSET+nOffset+OFFSET_TCP_ACKNUM, &nSeqNum, 4);		
}

void dumpData_ASCII (char * pData, char bySeparator, int nLength)
{
	int			j;

	for(j=0; j<nLength; j++) {
		if(j != 0 && bySeparator >= ' ') {
			printf("%c", bySeparator);
		}

		printf("%c", (unsigned char) pData[j]);
	}
}

void dumpHex (char * pData, char bySeparator, int nLength) {
	int			j;
	
	for(j=0; j<nLength; j++) {
		if(j != 0) {
			printf("%c", bySeparator);
		}
		
		printf("%02X", (unsigned char) pData[j]);		
	}
}

typedef unsigned short u16;
typedef unsigned long u32;

u16 ip_sum_calc_v2(u16 len_ip_header, u16 buff[])
{
	u16 word16;
	u32 sum=0;
	u16 i;

	// make 16 bit words out of every two adjacent 8 bit words in the packet
	// and add them up
	for (i=0;i<len_ip_header;i=i+2){
		word16 =((buff[i]<<8)&0xFF00)+(buff[i+1]&0xFF);
		sum = sum + (u32) word16;
	}

	// take only 16 bits out of the 32 bit sum and add up the carries
	while (sum>>16)
	  sum = (sum & 0xFFFF)+(sum >> 16);

	// one's complement the result
	sum = ~sum;

return ((u16) sum);
}


void	updateIPv4_Checksum (char * pData, int nLength) {
	unsigned short *	pChecksum;
	unsigned short	nCksum;

	// Zero out the checksum field
	pChecksum = (unsigned short *) (pData+L2_OFFSET+OFFSET_IPV4_CHECKSUM);

	//printf(" Previous IP Checksum: %04X\n", *pChecksum);

	*pChecksum = 0;

	nCksum = ip_sum_calc(getIP_HeaderSize(pData, nLength), (unsigned short *) (pData+L2_OFFSET));

//	nCksum = ip_sum_calc_v2(getIP_HeaderSize(pData, nLength), (unsigned short *) (pData+L2_OFFSET));
//	nCksum =ip_cksum(pData+L2_OFFSET,20);

	*pChecksum = nCksum;

	//printf(" New IP Checksum: %04X\n", *pChecksum);

}



unsigned short ip_sum_calc (unsigned short len_ip_header, unsigned short * pBuff)
{
	unsigned short		word16;
	unsigned int			sum=0;
	unsigned short		i;

	// get the sum in an integer (note: assumes int > 16 bits)
	for(i=0;i <len_ip_header/2;i++)
	   sum += pBuff[i];
	
	// now you have to fold in carries
	sum = (sum >> 16) + (sum & 0xffff);
	sum += sum>>16;
	return ~(sum);
}

void	updateTCP_Checksum (char * pData, int nLength) {
	unsigned short *	pChecksum;
	unsigned short	nCksum;
	int		nOffset;

	//cout << "TCP Checksum Update: Estimated length of " << nLength << endl;

	if(hasIPOptions(pData, nLength)) {		
		nOffset = getIP_HeaderSize(pData, nLength);
	} else {
		nOffset = L3_IPV4_OFFSET;
	}
	
	// Zero out the checksum field	
	pChecksum = (unsigned short *) (pData+L2_OFFSET+nOffset+OFFSET_TCP_CKSUM);
	
	//printf(" Previous TCP Checksum: %04X\n", *pChecksum);
	
	*pChecksum = 0;
	
	// From http://www.netfor2.com/tcpsum.htm
	//uint16_t tcp_sum_calc_alt(uint16_t len_tcp, uint16_t src_addr[], uint16_t dest_addr[], bool padding, uint16_t buff[])


	nCksum = tcp_sum_calc(getPacketLength(pData, nLength)-getIP_HeaderSize(pData, nLength), (unsigned short*) (pData+L2_OFFSET+OFFSET_IPV4_PROTOCOL_SRCIP),
						  (unsigned short*) (pData+L2_OFFSET+OFFSET_IPV4_PROTOCOL_DSTIP), (unsigned short *) (pData+L2_OFFSET+nOffset));
	
	*pChecksum = nCksum;
	
	//printf(" New Checksum: %04X\n", *pChecksum);
}


// Borrowed from http://www.netfor2.com/tcpsum.htm

unsigned short tcp_sum_calc(unsigned short len_tcp, unsigned short src_addr [], unsigned short dest_addr[],  unsigned short buff[])
{
	u16 prot_tcp=6;
	u16 padd=0;
	u16 word16;
	u32 sum;	
	u16 i;
	
	//cout << " Validating address computations" << endl;
	
	//dumpIPv4((char *) src_addr);
	//cout << "    ";
	//dumpIPv4((char *) dest_addr);
	//cout << endl;
	
	// Find out if the length of data is even or odd number. If odd,
	// add a padding byte = 0 at the end of packet
	if (len_tcp % 2){
		padd=1;
		buff[len_tcp]=0;
	}
	
	//initialize sum to zero
	sum=0;
	
	for(i=0;i<(len_tcp+padd)/2;i++)
	   sum += buff[i];
	
	sum += src_addr[0];
	sum += src_addr[1];
	
	sum += dest_addr[0];
	sum += dest_addr[1];

	sum += ntohs(prot_tcp);
	sum += ntohs(len_tcp);

	
	// now you have to fold in carries
	sum = (sum >> 16) + (sum & 0xffff);
	sum += sum>>16;
	return ~(sum);
}

// From http://www.netfor2.com/tcpsum.htm
uint16_t tcp_sum_calc_alt(uint16_t len_tcp, uint16_t src_addr[], uint16_t dest_addr[], bool padding, uint16_t buff[])
{
	uint16_t prot_tcp=6;
	uint16_t padd=0;
	uint16_t word16;
	uint32_t sum;
	int			i;

	// Find out if the length of data is even or odd number. If odd,
	// add a padding byte = 0 at the end of packet
	if (padding == true){
		padd=1;
		buff[len_tcp]=0;
	}

	//initialize sum to zero
	sum=0;

	// make 16 bit words out of every two adjacent 8 bit words and
	// calculate the sum of all 16 vit words
	for (i=0;i<len_tcp+padd;i=i+2){
		word16 =((buff[i]<<8)&0xFF00)+(buff[i+1]&0xFF);
		sum = sum + (unsigned long)word16;
	}
	// add the TCP pseudo header which contains:
	// the IP source and destinationn addresses,
	for (i=0;i<4;i=i+2){
		word16 =((src_addr[i]<<8)&0xFF00)+(src_addr[i+1]&0xFF);
		sum=sum+word16;
	}
	for (i=0;i<4;i=i+2){
		word16 =((dest_addr[i]<<8)&0xFF00)+(dest_addr[i+1]&0xFF);
		sum=sum+word16;
	}
	// the protocol number and the length of the TCP packet
	sum = sum + prot_tcp + len_tcp;

	// keep only the last 16 bits of the 32 bit calculated sum and add the carries
    	while (sum>>16)
		sum = (sum & 0xFFFF)+(sum >> 16);

	// Take the one's complement of sum
	sum = ~sum;

	return ((unsigned short) sum);
}

string IPv4toDomainName (char* ipAddr) {
	struct hostent *he;
    struct in_addr ipv4addr;

    memset(&ipv4addr, 0, sizeof(struct in_addr));
    inet_pton(AF_INET, ipAddr, &ipv4addr);

    if ((he = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET)) == NULL) {
        return "Unknown";
    }

    string host = he->h_name;
	char* ht = strdup(host.c_str());

	vector<string> output;
	output.clear();

    char* pch = strtok(ht, ".");
    while (pch != NULL) {
	    output.push_back(pch);
        pch = strtok(NULL, ".");
    }

    free(ht);

	string ret = "Unknown";

	int len = output.size();
    if (len >= 2) {
        ret = output[len-2] + "." + output[len-1];
    }

    return ret;
}

string IPv4toString (char* pData) {
	int		j;
	string	sResult;
	char	szTemp[21];
	
	sprintf(szTemp, "%d.%d.%d.%d", (unsigned char) pData[0], (unsigned char) pData[1], (unsigned char) pData[2], (unsigned char) pData[3]);
	sResult = szTemp;
	
	return sResult;	
}

string IPv4toString_24 (char* pData) {
    string  sResult;
    char    szTemp[21];

    sprintf(szTemp, "%d.%d.%d.0/24", (unsigned char) pData[0], (unsigned char) pData[1], (unsigned char) pData[2]);
    sResult = szTemp;

    return sResult;
}

void dumpIPv4 (char * pData) {
	int		j;
	
	for(j=0; j<4; j++) {
		if(j != 0) {
			printf(".");
		}
		
		printf("%d", (unsigned char) pData[j]);
	}
}

void dumpParseARP (char * pData) {
	// Whoot, if we got this far, we should be good
	cout << "  Breaking down the ARP" << endl;
	
	printf ("     H/W Type:   %02X%02X\n", (unsigned char) pData[L2_OFFSET+0], (unsigned char) pData[L2_OFFSET+1]);

	printf ("     Proto Type: %02X%02X\n", (unsigned char) pData[L2_OFFSET+2], (unsigned char) pData[L2_OFFSET+3]);

	printf ("     H/W , Proto Size: %02X ,  %02X\n", (unsigned char) pData[L2_OFFSET+4], (unsigned char) pData[L2_OFFSET+5]);

	printf ("     Operation:  %02X%02X\n", (unsigned char) pData[L2_OFFSET+6], (unsigned char) pData[L2_OFFSET+7]);

	
	cout << "     Src MAC: ";
	dumpHex(pData+L2_OFFSET+8, ':', DEFAULT_MAC_LEN);
	cout << endl;
	
	cout << "     Src IP: ";
	dumpIPv4(pData+8+L2_OFFSET+DEFAULT_MAC_LEN);
	cout << endl;

	cout << "     Dst MAC: ";
	dumpHex(pData+8+L2_OFFSET+DEFAULT_MAC_LEN+DEFAULT_IP_LEN, ':', DEFAULT_MAC_LEN);
	cout << endl;
	
	cout << "     Dst IP: ";
	dumpIPv4(pData+8+L2_OFFSET+DEFAULT_MAC_LEN*2+DEFAULT_IP_LEN);
	cout << endl;	
}	

unsigned char getTCP_HeaderSize (char * pData, int nLength) {	
	unsigned char			nVal;

	// The math is a bit tricky but think of it like this
	//	  The offset is in the upper four bits (upper nibble)
	//    The offset is a multiple of 4 bytes (i.e. 32 bits)
	//	  The four position in binary starts 2 spots over
	//	  If we shift it over two, that puts the value now starting at Bit 2 (aka starting it at multiples of 4)
	//	  We bit-wise AND with 0x3C to whack any sign bits that carried over

	if(hasIPOptions(pData, nLength)) {
		int		nOffset;
		
		nOffset = getIP_HeaderSize(pData, nLength);
		
		nVal = (pData[L2_OFFSET+nOffset+OFFSET_TCP_DATAOFF] >> 2) & 0x3C;
		return nVal;  		
	} else {	
		nVal = (pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_DATAOFF] >> 2) & 0x3C;
		//printf("  TCP Header size is %d\n", nVal);
		return nVal;  		
	}	
}

void setTCPFlag_PSH (char * pData, int nLength, char bVal) {
	if(hasIPOptions(pData, nLength)) {
		int		nOffset;
		
		nOffset = getIP_HeaderSize(pData, nLength);
		
		if(bVal) {
			pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] = pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] | BITMASK_TCP_FLAG_PSH;
		} else {
			pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] = pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] & BITMASK_TCP_FLAG_INV_PSH;	
		}
	} else {
		if(bVal) {
			pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] = pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] | BITMASK_TCP_FLAG_PSH;
		} else {
			pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] = pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] & BITMASK_TCP_FLAG_INV_PSH;	
		}		
	}		
}


char getTCPFlag_PSH (char * pData, int nLength) {
	// This is split up intentionally for the underlying branch prediction mechanism to have
	// a fast path and to cut down on pipeline stalls
	
	if(hasIPOptions(pData, nLength)) {
		int		nOffset;
		
		nOffset = getIP_HeaderSize(pData, nLength);
		
		return pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] & BITMASK_TCP_FLAG_PSH;			
	} else {
		return pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] & BITMASK_TCP_FLAG_PSH;		
	}	
}

void setTCPFlag_FIN (char * pData, int nLength, char bVal)
{
	if(hasIPOptions(pData, nLength)) {
		int		nOffset;

		nOffset = getIP_HeaderSize(pData, nLength);

		if(bVal) {
			pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] = pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] | BITMASK_TCP_FLAG_FIN;
		} else {
			pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] = pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] & BITMASK_TCP_FLAG_INV_FIN;
		}
	} else {
		if(bVal) {
			pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] = pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] | BITMASK_TCP_FLAG_FIN;
		} else {
			pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] = pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] & BITMASK_TCP_FLAG_INV_FIN;
		}
	}
}

void setTCPFlag_RST (char * pData, int nLength, char bVal)
{
	if(hasIPOptions(pData, nLength)) {
		int		nOffset;

		nOffset = getIP_HeaderSize(pData, nLength);

		if(bVal) {
			pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] = pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] | BITMASK_TCP_FLAG_RST;
		} else {
			pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] = pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] & BITMASK_TCP_FLAG_INV_RST;
		}
	} else {
		if(bVal) {
			pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] = pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] | BITMASK_TCP_FLAG_RST;
		} else {
			pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] = pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] & BITMASK_TCP_FLAG_INV_RST;
		}
	}
}

void setTCPFlag_ACK (char * pData, int nLength, char bVal) {
	if(hasIPOptions(pData, nLength)) {
		int		nOffset;
		
		nOffset = getIP_HeaderSize(pData, nLength);
		
		if(bVal) {
			pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] = pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] | BITMASK_TCP_FLAG_ACK;
		} else {
			pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] = pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] & BITMASK_TCP_FLAG_INV_ACK;	
		}
	} else {
		if(bVal) {
			pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] = pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] | BITMASK_TCP_FLAG_ACK;
		} else {
			pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] = pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] & BITMASK_TCP_FLAG_INV_ACK;	
		}		
	}	
}

char getTCPFlag_ACK (char * pData, int nLength) {
	// This is split up intentionally for the underlying branch prediction mechanism to have
	// a fast path and to cut down on pipeline stalls
	
	if(hasIPOptions(pData, nLength)) {
		int		nOffset;
		
		nOffset = getIP_HeaderSize(pData, nLength);
		
		return pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] & BITMASK_TCP_FLAG_ACK;			
	} else {
		return pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] & BITMASK_TCP_FLAG_ACK;		
	}			
}

char getTCPFlag_RST (char * pData, int nLength) {
	// This is split up intentionally for the underlying branch prediction mechanism to have
	// a fast path and to cut down on pipeline stalls
	
	if(hasIPOptions(pData, nLength)) {
		int		nOffset;
		
		nOffset = getIP_HeaderSize(pData, nLength);
		
		return pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] & BITMASK_TCP_FLAG_RST;			
	} else {
		return pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] & BITMASK_TCP_FLAG_RST;		
	}	
}


char getTCPFlag_FIN (char * pData, int nLength) {
	// This is split up intentionally for the underlying branch prediction mechanism to have
	// a fast path and to cut down on pipeline stalls
	
	if(hasIPOptions(pData, nLength)) {
		int		nOffset;
		
		nOffset = getIP_HeaderSize(pData, nLength);
		
		return pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] & BITMASK_TCP_FLAG_FIN;			
	} else {
		return pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] & BITMASK_TCP_FLAG_FIN;		
	}	
}

char getTCPFlag_SYN (char * pData, int nLength) {
	// This is split up intentionally for the underlying branch prediction mechanism to have
	// a fast path and to cut down on pipeline stalls
	
	if(hasIPOptions(pData, nLength)) {
		int		nOffset;
		
		nOffset = getIP_HeaderSize(pData, nLength);
		
		return pData[L2_OFFSET+nOffset+OFFSET_TCP_CONTROL] & BITMASK_TCP_FLAG_SYN;			
	} else {
		return pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL] & BITMASK_TCP_FLAG_SYN;		
	}	
}

uint8_t getTCP_Flags (char * pData)
{
	if(hasIPOptions(pData, -1)) {
		int		nOffset;

		nOffset = getIP_HeaderSize(pData, -1);

		return (uint8_t) pData[L2_OFFSET+nOffset+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL];
	} else {
		return (uint8_t) pData[L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_CONTROL];
	}
}

char hasIPOptions (char * pData, int nLength) {
	if(getIP_HeaderSize(pData, nLength) != L3_IPV4_OFFSET) {
		return 1;
	} else {
		return 0;
	}	
}

unsigned char getIP_HeaderSize (char * pData, int nLength) {
	unsigned char		nVal;
	
	nVal = (pData[L2_OFFSET+OFFSET_IPV4_VERLEN] & 0x0F) << 2;

	//printf("  IP Header size is %d\n", nVal);
	
	return nVal;	
}

uint16_t 	getOffset_TCP_Payload (char * pData, int nLength)
{
	uint16_t 	nOffset;

	if(hasIPOptions(pData, nLength)) {
		nOffset = getIP_HeaderSize(pData, nLength);
	} else {
		nOffset = L3_IPV4_OFFSET;
	}

	nOffset += getTCP_HeaderSize(pData,nLength);
	nOffset += L2_OFFSET;

	return nOffset;
}

unsigned short getTCP_Checksum (char * pData, int nLength) {
	unsigned short *	pChecksum;
	unsigned short	nCksum;
	int		nOffset;

	if(hasIPOptions(pData, nLength)) {		
		nOffset = getIP_HeaderSize(pData, nLength);
	} else {
		nOffset = L3_IPV4_OFFSET;
	}
	
	// Zero out the checksum field	
	pChecksum = (unsigned short *) (pData+L2_OFFSET+nOffset+OFFSET_TCP_CKSUM);
	
	return *pChecksum;	
}

void dumpIP_brief (char * pData) {
	cout << " IP Pkt  "; 
	cout << "   Hdr: " << (int) getIP_HeaderSize(pData, -1) << " Len: " << getPacketLength(pData, -1) << endl; 	
}

void parseIP (char * pData)
{
	printf("IP Packet \n");
	printf("  Hdr Size: %d\n   Version: %d\n", (int) getIP_HeaderSize(pData,-1), (int) getIPVersion(pData, -1));
	printf("  Length: %d\n", getPacketLength(pData, -1));
}

uint16_t	getTCP_WindowSize 	(char * pData)
{
	uint16_t 	nWindowSize;

	// Typecast magic to get the 16 bit length out of the IPv4 header
	nWindowSize = ntohs( *((uint16_t *) (pData+L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_WINDOW)));

	return nWindowSize;
}

uint16_t	getIP_PacketID (char * pPacket)
{
	uint16_t 	nID;

	// Typecast magic to get the 16 bit length out of the IPv4 header
	nID = ntohs( *((uint16_t *) (pPacket+L2_OFFSET+OFFSET_IPV4_ID)));

	return nID;
}


void		setIP_PacketID (char * pPacket, uint16_t nNewID)
{
	uint16_t	* pIP_ID;

	pIP_ID = (uint16_t *) (pPacket+L2_OFFSET+OFFSET_IPV4_ID);

	*pIP_ID = htons(nNewID);
}


void dumpTCP_brief (char * pData) {
	cout << " TCP Header (Hdr Size = ";

	printf("%d", (uint8_t) getTCP_HeaderSize(pData,-1));

	cout << ")" << endl;
	
	cout << "    SP: " << getTCP_SrcPort(pData);
	cout << "  DP: " << getTCP_DstPort(pData) << endl;

	cout << "    SN #:    " << getTCP_SeqNumber(pData, -1);

	if(getTCPFlag_ACK(pData,-1))
	{
		cout << "   AN #:    " << getTCP_AckNumber(pData, -1);
	}
	cout << endl;
	cout << "      Flags: ";
	
	if(getTCPFlag_SYN(pData, -1)) {
		cout << "S";
	} else {
		cout << " ";
	}
	
	if(getTCPFlag_FIN(pData, -1)) {
		cout << "F";
	} else {
		cout << " ";
	}
	
	if(getTCPFlag_RST(pData, -1)) {
		cout << "R";
	} else {
		cout << " ";
	}
	
	if(getTCPFlag_ACK(pData, -1)) {
		cout << "A";
	} else {
		cout << " ";
	}
	
	if(getTCPFlag_PSH(pData, -1)) {
		cout << "P";
	} else {
		cout << " ";
	}
		
	cout << endl;

	cout << "      Window: " << getTCP_WindowSize(pData) << endl;


	//printf("   Checksum: 0x%04X\n", getTCP_Checksum(pData, -1));	
	
}

/*************************************/

unsigned short getTCP_PayloadSize (char * pData) {
	return getPacketLength(pData, -1) - (getIP_HeaderSize(pData, -1) + getTCP_HeaderSize(pData, -1));		
}

/*************************************/

char	isPureAck (char * pData, int nLength) {
	if(getTCPFlag_ACK(pData,nLength) && getPacketLength(pData, nLength) == (getIP_HeaderSize(pData, nLength) + getTCP_HeaderSize(pData, nLength))) {
		return 1;
	}	else {
		return 0;
	}	
}

unsigned int calcTimeDiff (struct timeval * pComp1, struct timeval * pComp2) {
	unsigned int		nDiffSec;
	unsigned int		nDiffUS;
	char				bT1;
	
	nDiffSec = 0;
	nDiffUS = 0;
	
//	cout << "CTD (pC1): " << pComp1->tv_sec << "." << pComp1->tv_usec << endl;
//	cout << "CTD (pC2): " << pComp2->tv_sec << "." << pComp2->tv_usec << endl;
		
	if(pComp1->tv_sec >= pComp2->tv_sec) {
		nDiffSec = pComp1->tv_sec - pComp2->tv_sec; 
		bT1 = 1;
	} else {
		nDiffSec = pComp2->tv_sec - pComp1->tv_sec;
		bT1 = 0;
	}

	if(bT1 == 1) {		
		if(nDiffSec == 0) {	
			nDiffUS = abs(pComp1->tv_usec - pComp2->tv_usec);
		} else {
			nDiffUS += pComp1->tv_usec;
			nDiffUS += (1000000 - pComp2->tv_usec);	
			nDiffSec -= 1;
		}
	} else {
		if(nDiffSec == 0) {	
			nDiffUS = abs(pComp2->tv_usec - pComp1->tv_usec);
		} else {
			nDiffUS += pComp2->tv_usec;
			nDiffUS += (1000000 - pComp1->tv_usec);	
			nDiffSec -= 1;
		}
	}		
	
	return nDiffSec * 1000000 + nDiffUS;	
}

char	compareTime (struct timeval * pComp1, struct timeval * pComp2) {
	if(pComp1->tv_sec > pComp2->tv_sec) {
		return 1;
	} 
	
	if(pComp1->tv_sec == pComp2->tv_sec) {
		if(pComp1->tv_usec >= pComp2->tv_usec) {
			return 1;
		} else {
			return 0;
		}
	} else {
		// Must be less than
		return 0;
	}
	
}

void adjustTime (struct timeval * pAdj, unsigned int nAddUS) {
    // Revised to directly compute seconds rather than
    // iterate on via a while loop

    unsigned int nSec;

    nSec = nAddUS / 1000000;
    nAddUS = nAddUS - nSec * 1000000;

    pAdj->tv_sec += nSec;
    pAdj->tv_usec += nAddUS;
	
    while(pAdj->tv_usec >= 1000000) {
	    pAdj->tv_usec -= 1000000;
	    pAdj->tv_sec++;
    }
}

void displayTime (struct timeval * pTime) {
	time_t		theTime;
	struct tm	 *  loctime;

	char		szOutBuffer[256];
	
	theTime = pTime->tv_sec;
	loctime = localtime(&theTime);

	strftime (szOutBuffer, 256, "%d %b %y %H:%M:%S", loctime);
	cout << szOutBuffer;	
	
	printf(".%06d", (unsigned int) pTime->tv_usec);
}	

unsigned short  getTCPOptions_Size (char * pData)
{
	unsigned short 		nDiff;

	nDiff = getTCP_HeaderSize(pData,0) - 20;

	return nDiff;
}

unsigned short	purgeTCPOptions (char * pData, int nLength) {
	unsigned short 		nDiff;
	
	nDiff = getTCP_HeaderSize(pData,nLength) - 20;

	// Any TCP options present?
	if(nDiff == 0) {
		//cout << "    ** No options!" << endl;
		return 0;
	}
	// Force the TCP header size to 20 bytes -> 32
	pData[L2_OFFSET+getIP_HeaderSize(pData,nLength)+OFFSET_TCP_DATAOFF] &= 0x0F;
	pData[L2_OFFSET+getIP_HeaderSize(pData,nLength)+OFFSET_TCP_DATAOFF] |= 0x50;
		
	int			nBytesToCopy;
	
	nBytesToCopy = getPacketLength(pData,nLength) - getIP_HeaderSize(pData,nLength) - 20;	
	
    memcpy(pData+L2_OFFSET+getIP_HeaderSize(pData, nLength)+20,pData+L2_OFFSET+getIP_HeaderSize(pData, nLength)+20+nDiff,
                    nBytesToCopy);

    // Update the IP length
	//cout << "  Setting the packet length to " << getPacketLength(pData,nLength)- (unsigned short) nDiff << endl;
	setPacketLength(pData, getPacketLength(pData,nLength)- (unsigned short) nDiff);

	updateIPv4_Checksum  (pData, nLength);				
	updateTCP_Checksum (pData, nLength);
	
	return nDiff;
}
unsigned short	purgeTCPOptions_TsOptOn (char * pData, int nLength) {
	unsigned short 		nDiff;
	
	nDiff = getTCP_HeaderSize(pData,nLength) - 32;

	// Any TCP options present?
	if(nDiff < 0) {
		cerr << "Error: ** No options when purge TCP options with header size "<< nDiff + 32 << endl;
		return 0;
	}
    /* else{ */
    /*         //printf("The options: \n"); */
    /*         //for(int i = 0;i<nDiff;i++) */
    /*         //        printf("%x ",pData[L2_OFFSET+getIP_HeaderSize(pData, nLength)+20+i]); */
    /* } */
    unsigned char myOpt[12]={0x08,0x0a,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00};
		
	//cout << "  Difference is " << nDiff << " bytes!" << endl;

	// We can safely ignore the MAC, IP, and TCP headers to start
		
    memcpy(pData+L2_OFFSET+getIP_HeaderSize(pData, nLength)+20, myOpt, 12);
    //cout<<"Before Check the copy bytes:"<<endl;
    //for(int i = 0;i<nDiff;i++)
    //        printf("%d:%x ",i,pData[L2_OFFSET+getIP_HeaderSize(pData, nLength)+20+i]);
	// Force the TCP header size to 20 bytes -> 32
    //printf("Before header size : %x\t",pData[L2_OFFSET+getIP_HeaderSize(pData,nLength)+OFFSET_TCP_DATAOFF]);
	pData[L2_OFFSET+getIP_HeaderSize(pData,nLength)+OFFSET_TCP_DATAOFF] &= 0x0F;
	pData[L2_OFFSET+getIP_HeaderSize(pData,nLength)+OFFSET_TCP_DATAOFF] |= 0x80;
    //printf("After header size : %x\t",pData[L2_OFFSET+getIP_HeaderSize(pData,nLength)+OFFSET_TCP_DATAOFF]);
		
	int			nBytesToCopy;
	
	nBytesToCopy = getPacketLength(pData,nLength) - getIP_HeaderSize(pData,nLength) - 32;	
    //cout<<"Check the copy bytes:"<<endl;
    //for(int i = 0;i<nBytesToCopy;i++)
    //        printf("%d:%x ",i,pData[L2_OFFSET+getIP_HeaderSize(pData, nLength)+20+i]);
	
    memcpy(pData+L2_OFFSET+getIP_HeaderSize(pData, nLength)+32,pData+L2_OFFSET+getIP_HeaderSize(pData, nLength)+32+nDiff,
                    nBytesToCopy);

    // Update the IP length
	//cout << "  Setting the packet length to " << getPacketLength(pData,nLength)- (unsigned short) nDiff << endl;
	setPacketLength(pData, getPacketLength(pData,nLength)- (unsigned short) nDiff);

	updateIPv4_Checksum  (pData, nLength);				
	updateTCP_Checksum (pData, nLength);
	
	return nDiff;
}

void setTCP_WindowSize (char * pData, unsigned short nWindow) {
	unsigned short	* pTCPWin;
	int		nOffset;
	
	if(nWindow == 0) {
		return;
	}

	if(hasIPOptions(pData, -1)) {		
		nOffset = getIP_HeaderSize(pData, -1);
	} else {
		nOffset = L3_IPV4_OFFSET;
	}
		
	pTCPWin = (unsigned short *) (pData+L2_OFFSET+nOffset+OFFSET_TCP_WINDOW);	
	*pTCPWin = htons(nWindow);		
}

void clearTCP_Flags (char * pData)
{
	pData[OFFSET_TCP_CONTROL+L2_OFFSET+L3_IPV4_OFFSET] = 0;
}


