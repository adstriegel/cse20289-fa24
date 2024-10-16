/* NetAddressIPv4Subnet.cc
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

#include <vector>
using namespace std;

#include <stdlib.h>

#include "NetAddressIPv4Subnet.h"

#include "../../util/tokenize.h"
#include "../../core/LogEvent.h"
#include "../../mem/MemoryPool.h"

///////////////////////////////////////////////////////

NetAddressIPv4Subnet::NetAddressIPv4Subnet () : NetAddress (NETADDRESS_TYPE_IPV4SUBNET, MEMPOOL_OBJ_NETADDRESS_IPV4SUBNET) {
	allocate(ADDRESS_IPV4_SIZE);
	m_nMaskLength = ADDRESS_IPV4SUBNET_MASK_DEFAULT;
}

///////////////////////////////////////////////////////

NetAddressIPv4Subnet * NetAddressIPv4Subnet::create () {
	NetAddressIPv4Subnet 	*	pAddr;
	pAddr = (NetAddressIPv4Subnet *) g_MemPool.getObject(MEMPOOL_OBJ_NETADDRESS_IPV4SUBNET);
	return pAddr;	
}

///////////////////////////////////////////////////////

void NetAddressIPv4Subnet::clean () {
	m_nMaskLength = ADDRESS_IPV4SUBNET_MASK_DEFAULT;
}

///////////////////////////////////////////////////////

string NetAddressIPv4Subnet::toString () {
	string		sTemp;
	char 		szTemp[30];
	unsigned char *		pAddress;
	
	pAddress = getAddress();
	
	if(pAddress == NULL) {
		logFaultMediumEvent("Warning: Attempting to convert an IPv4 address to a string but address data is NULL", "NetAddressIPv4", "toString");
		return "NULL address";
	}
	
	sprintf(szTemp, "%d.%d.%d.%d/%d", (unsigned char) pAddress[0], (unsigned char) pAddress[1], (unsigned char) pAddress[2], (unsigned char) pAddress[3], (unsigned char) m_nMaskLength);

	sTemp = szTemp;
	return sTemp;
}

///////////////////////////////////////////////////////

string NetAddressIPv4Subnet::toDebugString () {
	string		sTemp;
	char		szTemp[20];
	
	sTemp = "Type=NetAddressIPv4Subnet,";
	
	sprintf(szTemp,"Size=%d",getSize());
	
	sTemp += szTemp;
	
	sTemp += toString();
	
	sprintf(szTemp, ",Mask=%d", m_nMaskLength);
	sTemp += szTemp;
	
	return sTemp;
}

///////////////////////////////////////////////////////

bool NetAddressIPv4Subnet::setAddressFromString (string sAddress) {
	vector<string>		listDots;
	int				j;	
	unsigned char * pAddress;
	
	Tokenize(sAddress,listDots,"./");
	
	if(listDots.size() < ADDRESS_IPV4_SIZE) {
		logFaultLowEvent("Fault: Input IPv4 subnet (" + sAddress + ") does not contain the full three periods.", "NetAddressIPv4Subnet", "setAddressFromString");
		return false;
	}

	if(listDots.size() != ADDRESS_IPV4_SIZE+1) {
		logFaultLowEvent("Fault: Input IPv4 subnet (" + sAddress + ") does not contain a bitmask length via a /.", "NetAddressIPv4Subnet", "setAddressFromString");
		return false;
	}
		
	// TODO - Add support for * notation, i.e. 10.*
	
	pAddress = getAddress();
	
	for(j=0; j<ADDRESS_IPV4_SIZE; j++) {
		pAddress[j] = (unsigned char) atoi(listDots[j].c_str()); 
	}
	
	return setMaskLength((unsigned char) atoi(listDots[ADDRESS_IPV4_SIZE+1].c_str()));	
}

///////////////////////////////////////////////////////

NetAddressIPv4Subnet * NetAddressIPv4Subnet::createFromString (string sAddress) {
	vector<string>		listDots;
	int				j;
	NetAddressIPv4Subnet *		pAddrIP;
	
	pAddrIP = NetAddressIPv4Subnet::create();
	
	if(pAddrIP->setAddressFromString(sAddress)) {
		return pAddrIP;
	} else {
		pAddrIP->release();
		return NULL;
	}
}

///////////////////////////////////////////////////////

unsigned char	NetAddressIPv4Subnet::getMaskLength () {
	return m_nMaskLength;
}

///////////////////////////////////////////////////////

bool NetAddressIPv4Subnet::setMaskLength (unsigned char nMask) {
	if(nMask == 0 || nMask > 32) {
		logFaultLowEvent("Fault: Illegal value for mask length.", "NetAddressIPv4Subnet", "setMaskLength");		
		return false;
	} else {
		m_nMaskLength = nMask;
		return true;
	}
}

///////////////////////////////////////////////////////
