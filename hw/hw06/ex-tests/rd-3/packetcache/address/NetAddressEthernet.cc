/* NetAddressEthernet.cc
 **********************************************************
 * This code is part of the Scalability in a Box (ScaleBox)
 * system developed at the University of Notre Dame. 
 * 
 * For additional information, please visit the NetScale
 * laboratory webpage at:
 *    http://netscale.cse.nd.edu/
 *
 * PI:       Dr. Aaron Striegel    striegel@nd.edu
 * 
 **********************************************************
 * Development and equipment support has been provided in
 *  part by the following sponsors:
 * 
 *    National Science Foundation    CNS03-47392
 *    Sun Microsystems Academic Excellence Grant
 *    Intel IXP Development Grant 
 *    University of Notre Dame Faculty Grant
 * 
 **********************************************************
 */

#include <iostream>
using namespace std;

#include <cstdio>
using namespace std;

#include "NetAddressEthernet.h"

#include "../../util/tokenize.h"
#include "../../core/LogEvent.h"
#include "../../mem/MemoryPool.h"

///////////////////////////////////////////////////////

NetAddressEthernet::NetAddressEthernet () : NetAddress (NETADDRESS_TYPE_ETHERNET,MEMPOOL_OBJ_NETADDRESS_ETHERNET) {
	allocate(ADDRESS_ETHERNET_SIZE);
}

///////////////////////////////////////////////////////

NetAddressEthernet * NetAddressEthernet::create () {
	NetAddressEthernet 	*	pAddr;
	pAddr = (NetAddressEthernet *) g_MemPool.getObject(MEMPOOL_OBJ_NETADDRESS_ETHERNET);
	return pAddr;			
}

///////////////////////////////////////////////////////

void NetAddressEthernet::clean () {
	// Nothing to do here for this one
}

///////////////////////////////////////////////////////

string NetAddressEthernet::toString () {
	string		sTemp;
	char 		szTemp[30];
	unsigned char *		pAddress;
	
	pAddress = getAddress();
	
	if(pAddress == NULL) {
		logFaultMediumEvent("Warning: Attempting to convert an Ethernet address to a string but address data is NULL", "NetAddressEthernet", "toString");
		return "NULL address";
	}
	
	sprintf(szTemp, "%02X:%02X:%02X:%02X:%02X:%02X", (unsigned char) pAddress[0], (unsigned char) pAddress[1], (unsigned char) pAddress[2], (unsigned char) pAddress[3], (unsigned char) pAddress[4], (unsigned char) pAddress[5]);
	
	sTemp = szTemp;
	return sTemp;
}

///////////////////////////////////////////////////////

string NetAddressEthernet::toDebugString () {
	string		sTemp;
	char		szTemp[20];
	
	sTemp = "Type=NetAddressEthernet,";
	
	sprintf(szTemp,"Size=%d",getSize());
	
	sTemp += szTemp;
	
	sTemp += toString();
	
	return sTemp;
}

///////////////////////////////////////////////////////

bool NetAddressEthernet::isBroadcast () {
	unsigned char * pAddress;
	int			j;
	
	pAddress = getAddress();
	
	if(pAddress == NULL) {
		logFaultMediumEvent("Warning: Attempting to assess if an Ethernet address is broadcast but the address data is NULL", "NetAddressEthernet", "isBroadcast");
		return false;
	}

	for(j=0; j<ADDRESS_ETHERNET_SIZE; j++) {
		if(pAddress[j] != 0xFF) {
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////

bool NetAddressEthernet::setAddressFromString (string sAddress) {
	vector<string>		listDots;
	int				j;	
	int				nTemp;
	unsigned char * pAddress;
	
	Tokenize(sAddress,listDots,":");
	
	if(listDots.size() != ADDRESS_ETHERNET_SIZE) {
		logFaultLowEvent("Fault: Input Ethernet address (" + sAddress + ") does not contain the full five colons.", "NetAddressEthernet", "setAddressFromString");
		return false;
	}
	
	pAddress = getAddress();
	
	for(j=0; j<ADDRESS_ETHERNET_SIZE; j++) {
		sscanf(listDots[j].c_str(),"%x", &nTemp);
		pAddress[j] = (unsigned char) nTemp;
	}	
		
	return true;
}

///////////////////////////////////////////////////////

NetAddressEthernet * NetAddressEthernet::createFromString (string sAddress) {
	NetAddressEthernet *		pAddrEth;
	
	pAddrEth = NetAddressEthernet::create();
	
	if(pAddrEth->setAddressFromString(sAddress)) {
		return pAddrEth;
	} else {
		pAddrEth->release();
		return NULL;
	}
}

///////////////////////////////////////////////////////

