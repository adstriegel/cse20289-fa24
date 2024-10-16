/* NetAddressIPv4.cc
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

#include <vector>
using namespace std;

#include "NetAddressIPv4.h"

#include "../../util/tokenize.h"

#include "../../core/LogEvent.h"
#include "../../mem/MemoryPool.h"

///////////////////////////////////////////////////////

NetAddressIPv4::NetAddressIPv4 () : NetAddress (NETADDRESS_TYPE_IPV4,MEMPOOL_OBJ_NETADDRESS_IPV4) {
	allocate(ADDRESS_IPV4_SIZE);
}

///////////////////////////////////////////////////////

NetAddressIPv4 * NetAddressIPv4::create () {
	NetAddressIPv4 	*	pAddr;
	pAddr = (NetAddressIPv4 *) g_MemPool.getObject(MEMPOOL_OBJ_NETADDRESS_IPV4);
	return pAddr;			
}

///////////////////////////////////////////////////////

void NetAddressIPv4::clean () {
	// Nothing to do here for this one
}

///////////////////////////////////////////////////////

string NetAddressIPv4::toString () {
	string		sTemp;
	char 		szTemp[30];
	unsigned char *		pAddress;
	
	pAddress = getAddress();
	
	if(pAddress == NULL) {
		logFaultMediumEvent("Warning: Attempting to convert an IPv4 address to a string but address data is NULL", "NetAddressIPv4", "toString");
		return "NULL address";
	}
	
	sprintf(szTemp, "%d.%d.%d.%d", (unsigned char) pAddress[0], (unsigned char) pAddress[1], (unsigned char) pAddress[2], (unsigned char) pAddress[3]);

	sTemp = szTemp;
	return sTemp;
}

///////////////////////////////////////////////////////

string NetAddressIPv4::toDebugString () {
	string		sTemp;
	char		szTemp[20];
	
	sTemp = "Type=NetAddressIPv4,";
	
	sprintf(szTemp,"Size=%d",getSize());
	
	sTemp += szTemp;
	
	sTemp += toString();
	
	return sTemp;
}

///////////////////////////////////////////////////////

bool NetAddressIPv4::isBroadcast () {
	unsigned char * pAddress;
	
	pAddress = getAddress();
	
	if(pAddress == NULL) {
		logFaultMediumEvent("Warning: Attempting to assess if an IPv4 address is broadcast but the address data is NULL", "NetAddressIPv4", "isBroadcast");
		return false;
	}
	
	if(pAddress[3] == 255) {
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////

bool NetAddressIPv4::isMulticast () {
	unsigned char * pAddress;
	
	pAddress = getAddress();
	
	if(pAddress == NULL) {
		logFaultMediumEvent("Warning: Attempting to assess if an IPv4 address is multicast but the address data is NULL", "NetAddressIPv4", "isMulticast");
		return false;
	}
	
	if(pAddress[0] >= 224) {
		return true;
	}
	
	return false;
}

///////////////////////////////////////////////////////

bool NetAddressIPv4::isPrivateAddress () {
	unsigned char * pAddress;
	
	pAddress = getAddress();
	
	if(pAddress == NULL) {
		logFaultMediumEvent("Warning: Attempting to assess if an IPv4 address is private but address data is NULL", "NetAddressIPv4", "isPrivateAddress");
		return false;
	}
	
	// Are we in 10 net, 10.*?
	if(pAddress[0] == 10) {
		return true;
	}
	
	// Are we in 192.168.*?
	if(pAddress[0] == 192 && pAddress[1] == 168) {
		return true;
	}
	
	return false;
}

///////////////////////////////////////////////////////

bool NetAddressIPv4::setAddressFromString (string sAddress) {
	vector<string>		listDots;
	int				j;	
	unsigned char * pAddress;
	
	Tokenize(sAddress,listDots,".");
	
	if(listDots.size() != ADDRESS_IPV4_SIZE) {
		logFaultLowEvent("Fault: Input IPv4 address (" + sAddress + ") does not contain the full three periods.", "NetAddressIPv4", "createFromString");
		return false;
	}
	
	pAddress = getAddress();
	
	for(j=0; j<ADDRESS_IPV4_SIZE; j++) {
		pAddress[j] = (unsigned char) atoi(listDots[j].c_str()); 
	}
		
	return true;
}

///////////////////////////////////////////////////////

NetAddressIPv4 * NetAddressIPv4::createFromString (string sAddress) {
	NetAddressIPv4 *		pAddrIP;
	
	pAddrIP = NetAddressIPv4::create();
	
	if(pAddrIP->setAddressFromString(sAddress)) {
		return pAddrIP;
	} else {
		pAddrIP->release();
		return NULL;
	}
}

///////////////////////////////////////////////////////

