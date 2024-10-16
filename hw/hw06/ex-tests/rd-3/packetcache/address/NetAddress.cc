/* NetAddress.cc
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

#include "NetAddress.h"
#include "../../mem/MemoryPool.h"
#include "../../core/LogEvent.h"


///////////////////////////////////////////////////////

NetAddress::NetAddress (unsigned short nType, unsigned short nPoolType) : MemPoolObject (nPoolType) {
	m_pAddress = NULL;
	m_nType = nType;
	m_nSize = 0;	
}
	
///////////////////////////////////////////////////////

NetAddress::~NetAddress () {
	if(m_pAddress != NULL) {
		delete m_pAddress;
		m_nSize = 0;
	}
}

///////////////////////////////////////////////////////

NetAddress * NetAddress::create () {
	NetAddress 	*	pAddr;
	pAddr = (NetAddress *) g_MemPool.getObject(MEMPOOL_OBJ_NETADDRESS_BASE);
	return pAddr;			
}

///////////////////////////////////////////////////////

void NetAddress::clean () {
	// By default, we will just let everything stand in the generic class.  This might get
	//  us into trouble later but it is generally recommended to use the derived classes
	//  rather than the base class
}

///////////////////////////////////////////////////////

string 	NetAddress::toString () {
	return "";
}

///////////////////////////////////////////////////////

string	NetAddress::toHexString () {
	return "";
}

///////////////////////////////////////////////////////

string	NetAddress::toDebugString () {
	return "";
}

///////////////////////////////////////////////////////
	
unsigned char * NetAddress::getAddress () {
	return m_pAddress;
}

///////////////////////////////////////////////////////

bool NetAddress::setAddress (unsigned char * pAddr, unsigned int nSize) {
	if(nSize > m_nSize) {
		logFaultHighEvent("Warning: Attempted to set an address that would exceed size boundary");
		return false;
	}
	
	memcpy(m_pAddress, pAddr, nSize);
	return true;
}

///////////////////////////////////////////////////////
		
bool NetAddress::setAddressWithGrow (unsigned char * pAddr, unsigned int nSize) {
	if(nSize > MAX_ADDRESS_LENGTH) {
		logFaultHighEvent("Warning: Attempted to set an address (with growth) that would exceed size boundary");
		return false;
	}
	
	if(nSize > m_nSize) {
		if(m_pAddress != NULL) {
			delete m_pAddress;
		}
		
		m_pAddress = new unsigned char[nSize];
		m_nSize = nSize;
	}
	
	return true;
}

///////////////////////////////////////////////////////

unsigned short NetAddress::getAddressType () {
	return m_nType;
}

///////////////////////////////////////////////////////
		
NetAddress & NetAddress::operator = (const NetAddress rhs) {
	m_nSize = rhs.m_nSize;
	m_nType = rhs.m_nType;
	
	if(m_pAddress != NULL) {
		m_pAddress = new unsigned char [m_nSize];
	}
	
	if(m_pAddress == NULL) {
		m_nSize = 0;
	} else {
		memcpy(m_pAddress, rhs.m_pAddress, m_nSize);
	}

	return *this;
}
		
bool NetAddress::operator == (NetAddress rhs) {
	return false;
}

bool NetAddress::operator == (unsigned char * pComp) {	
	checkIntegrity();
	
	if(memcmp(pComp, m_pAddress, m_nSize) == 0) {
		return true;
	} else {
		return false;
	}
}

bool NetAddress::operator == (char * pComp) {
	// Typecast and defer to the base function to avoid repetition of behavior
	return *this == (unsigned char *) pComp;	
}

bool NetAddress::operator != (NetAddress rhs) {
	return false;
}

bool NetAddress::operator < (NetAddress rhs) {
	return false;
}

bool NetAddress::operator > (NetAddress rhs) {
	return false;
}

bool NetAddress::operator >= (NetAddress rhs) {
	return false;
}

bool NetAddress::operator <= (NetAddress rhs) {
	return false;
}
		
bool NetAddress::setAddressType (unsigned short nType) {
	return false;
}
		
bool NetAddress::allocate (unsigned int nSize) {
	if(nSize > MAX_ADDRESS_LENGTH) {
		return false;
	}
	
	if(m_pAddress != NULL) {
		delete m_pAddress;
	}
	
	m_pAddress = new unsigned char [nSize];
	m_nSize = nSize;
	
	if(m_pAddress != NULL) {
		return true;
	} else {
		logFailureEvent("Warning: Unable to allocate memory in NetAddress::allocate");
		m_nSize = 0;
		return false;
	}
}
	
bool NetAddress::grow (unsigned int nIncrease) {
	unsigned char * 	pTempAddr;
	
	if(nIncrease + m_nSize > MAX_ADDRESS_LENGTH) {
		return false;
	}
		
	pTempAddr = new unsigned char [m_nSize+nIncrease];
	m_nSize += nIncrease;
	
	if(pTempAddr != NULL) {
		return true;
	} else {
		logFailureEvent("Warning: Unable to allocate memory in NetAddress::grow");
		return false;
	}	
	
	memcpy(pTempAddr, m_pAddress, m_nSize-nIncrease);
	
	if(m_pAddress != NULL) {
		delete m_pAddress;
	}
	
	m_pAddress = pTempAddr;
	return true;
}
	
string NetAddress::convertTypeToString () {
	switch(m_nType) {
		case NETADDRESS_TYPE_NONE:
			return "None";
		case NETADDRESS_TYPE_ETHERNET:
			return "Ethernet (802.3)";
		case NETADDRESS_TYPE_IPV4:
			return "IPv4";
		case NETADDRESS_TYPE_IPV6:			
			return "IPv6";
		case NETADDRESS_TYPE_PORT:
			return "Layer 4 Port";
		case NETADDRESS_TYPE_DNS:
			return "DNS Hostname";
		default:
			return "Illegal value";
	}
}

unsigned int NetAddress::getSize () {
	return m_nSize;
}

unsigned int NetAddress::getAllocatedSize () {
	return m_nSize;
}

bool NetAddress::setAddressFromString (string sAddress) {
	return false;
}

NetAddress * NetAddress::createFromString (string sAddress) {
	logInformEvent("Warning: create for NetAddress not yet implemented", "NetAddress", "createFromString");
	return NULL;
}

bool NetAddress::checkIntegrity () {
	if(m_pAddress == NULL) {
		return false;
	}
	
	if(m_nSize == 0) {
		return false;
	}
	
	if(m_nSize > MAX_ADDRESS_LENGTH) {
		return false;
	}
	
	return true;
}

void NetAddress::release () {
	delete this;
}
