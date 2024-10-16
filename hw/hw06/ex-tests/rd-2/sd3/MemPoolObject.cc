/* MemPoolObject.cc
 **********************************************************
 * This code is part of the Scalability in a Box (ScaleBox)
 * system developed at the University of Notre Dame. 
 * 
 * For additional information, please visit the NetScale
 * laboratory webpage at:
 * 
 *    http://netscale.cse.nd.edu/
 *
 * PI:       Dr. Aaron Striegel    striegel@nd.edu
 * 
 * Students: Dave Salyers (lead)   Yingxin Jiang
 *           Xiaolong Li           Jeff Smith
 * 
 **********************************************************
 * $Revision: 1.4 $  $Date: 2008/04/11 22:03:19 $
 **********************************************************
 */

#include "MemPoolObject.h"
#include "MemPoolCustom.h"
#include "MemoryPool.h"

// Derived types - for the purpose of allocating new objects when the
//  initial memory pool is exhausted.  Not exactly as clean as one
//  would desire but functional
#include "../pkt/Packet.h"

#include "../pkt/PktSigXOR.h"
#include "../pkt/PktSigMD5.h"

#include "../xml/NodeDOM.h"
#include "../xml/NodeElem.h"

#include "../mon/Thread_Timer.h"

#include "../pkt/address/NetAddress.h"
#include "../pkt/address/NetAddressEthernet.h"
#include "../pkt/address/NetAddressIPv4.h"
#include "../pkt/address/NetAddressIPv4Subnet.h"



MemPoolObject::MemPoolObject () {
	m_nType = MEMPOOL_OBJ_NONE;
	
	// By default when created, the object is not immediately part of
	//  the pool
	m_bInPool = false;

	m_pOwner = NULL;
}
		
MemPoolObject::MemPoolObject (int nType) {
	m_nType = nType;	
}

int MemPoolObject::getType () {
	return m_nType;
}

void MemPoolObject::setOwner (void * pOwner) {
	m_pOwner = pOwner;
}

void * MemPoolObject::getOwner ()
{
	return m_pOwner;
}

void MemPoolObject::cleanBase ()
{
	m_pOwner = NULL;
}

bool MemPoolObject::release () {
	if(isAlreadyInPool())
	{
		cerr << "* Error: Memory Pool: Object is already in the pool and is attempted to be released again into the pool" << endl;
		cerr << "    Type = " << m_nType << "  " << getClassInfo() << endl;
		return false;
	}

	return g_MemPool.releaseObject(this);
}

bool  MemPoolObject::isAlreadyInPool ()
{
	return m_bInPool;
}

void MemPoolObject::setType (int newType) {
	m_nType = newType;
}
	
string MemPoolObject::getClassInfo () {
	string		theString;
	
	theString = "MemPoolObject (Base Class)";
	return theString;
}

void MemPoolObject::markAllocated () {
	m_bInPool = false;
}

void MemPoolObject::markInPool () {
	m_bInPool = true;
}

string MemPoolObject::mapIDtoClassType (int nType) { 
	
	switch(nType) {
		 case MEMPOOL_OBJ_PACKET:
		 	return "Packet";
		 case MEMPOOL_OBJ_PKTSIGXOR:
		 	return "PktSigXOR";
		 case MEMPOOL_OBJ_PKTSIGMD5:
		 	return "PktSigMD5";
		 case MEMPOOL_OBJ_DOMNODE:
		 	return "NodeDOM";
		 case MEMPOOL_OBJ_DOMELEM:
		 	return "NodeElem";
		 case MEMPOOL_OBJ_TIMEREVENT:
		 	return "TimerEvent";
		 case MEMPOOL_OBJ_NETADDRESS_BASE:
			return "NetAdress";
		 case MEMPOOL_OBJ_NETADDRESS_ETHERNET:
			return "NetAddressEthernet";
		 case MEMPOOL_OBJ_NETADDRESS_IPV4:
			return "NetAddressIPv4";
		 case MEMPOOL_OBJ_NETADDRESS_IPV4SUBNET:
			return "NetAddressIPv4Subnet";
					 	
		 default:
		 	return mapCustomObjectNameToID(nType);	
	}	
	
}


MemPoolObject * 	allocateManagedMemObject (int nType) {	
	if(nType >= MEMPOOL_OBJ_LAST) {
		return allocateManagedCustomMemObject(nType);	
	} else if(nType <= MEMPOOL_OBJ_NONE) {
		return NULL;
	}
	
	// Yes, this is kind of ugly but it functions
	
	switch(nType) {
		 case MEMPOOL_OBJ_PACKET:
		 	return new Packet();
		 case MEMPOOL_OBJ_PKTSIGXOR:
		 	return new PktSigXOR();
		 case MEMPOOL_OBJ_PKTSIGMD5:
		 	return new PktSigMD5();
		 case MEMPOOL_OBJ_DOMNODE:
		 	return new NodeDOM();
		 case MEMPOOL_OBJ_DOMELEM:
		 	return new NodeElem();
		 case MEMPOOL_OBJ_TIMEREVENT:
		 	return new TimerEvent();
		 case MEMPOOL_OBJ_NETADDRESS_BASE:
			return new NetAddress(NETADDRESS_TYPE_NONE,MEMPOOL_OBJ_NETADDRESS_BASE);
		 case MEMPOOL_OBJ_NETADDRESS_ETHERNET:
			return new NetAddressEthernet();
		 case MEMPOOL_OBJ_NETADDRESS_IPV4:
			return new NetAddressIPv4();
		 case MEMPOOL_OBJ_NETADDRESS_IPV4SUBNET:
			return new NetAddressIPv4Subnet();
	}	
	
	return NULL;
}

char		checkValidMemObjectType (int nType) {
	if(nType <= MEMPOOL_OBJ_NONE) {
		return 0;
	}
	else if(nType >= MEMPOOL_OBJ_CUSTLAST) {
		return 0;
	} 
	else {
		return 1;
	}
}

