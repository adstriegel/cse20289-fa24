/* AdapterPCap.h
 **********************************************************
 * This code is part of the Scalability in a Box (ScaleBox)
 * system developed at the University of Notre Dame. 
 * 
 * For additional information, please visit the NetScale
 * laboratory webpage at:
 * 
 * 	 http://netscale.cse.nd.edu/
 *
 * PI:       Prof. Aaron Striegel    striegel@nd.edu
 */

#include <iostream>
using namespace std;


#include "AdapterPCap.h"
#include "Packet.h"

#include "../mem/MemoryPool.h"
#include "Thread_IO.h"
#include "../xml/NodeDOM.h"
#include "../xml/NodeElem.h"


/** The libpcap callback function that is provided by the input thread
 * for grabbing packets from libpcap. 
 * @param args   A typecast AdapterPCap object for returning the packet
 * @param header The pcap structure read in by libpcap denoting size and time
 * @param packet The actual data in a byte array
 */
void pkt_callback (u_char *args, const struct pcap_pkthdr *header, const u_char *packet) 
{
	AdapterPCap		* pAdapter;
	Packet		   * pPacket;
	
	pAdapter = (AdapterPCap *) args;
	
	pPacket = (Packet *) g_MemPool.getObject(MEMPOOL_OBJ_PACKET);
	
	if(pPacket == NULL) {
		cerr << "Packet callback failed, no valid packet in the memory pool" << endl;
		return;
	}
	
	if(header->caplen != header->len) {
		cerr << "Warning: libpcap burped and didn't get us the whole packet, bailing on this packet" << endl;
		pAdapter->setReadPacket(NULL);	
		pPacket->release();	
//		g_theMonitor.addStat(MONITOR_STAT_PCAP_ERR,1);
		return;
	}

	// Set the arrival time of the packet to denote when libpcap got it	
	pPacket->setArrivalTime((timeval *) &(header->ts));
	pPacket->setData(header->caplen, (char *) packet);	
	pAdapter->addInputQueue(pPacket);						
}

AdapterPCap::AdapterPCap () : Adapter () {
	m_bBatchRead = 1;
}

AdapterPCap::~AdapterPCap () {
	
}

int AdapterPCap::openDevice () {
	char 	errbuf[PCAP_ERRBUF_SIZE];
	int	nResult;

	if(strcmp(m_szDevName, "UNKNOWN") == 0) {
		cerr << "No name specified for adapter, exiting...." << endl;
		exit(-1);
	}
	
	cout << "Device Name: " << m_szDevName << endl;

	m_pDevice = pcap_open_live(m_szDevName,MAX_PKT_SIZE,1,1,errbuf);
	
	if(m_pDevice == NULL) {
		cerr << "Libpcap open live device failed for " << m_szDevName << endl;
		cerr << "  " << errbuf << endl;
		return -1;
	}

	// Force the adapter to record only traffic inbound to the adapter (Rx)
	//  and not to monitor any outbound traffic
	if(pcap_setdirection(m_pDevice, PCAP_D_IN)) {
		cerr << "Warning: Issue setting direction for pcap device" << endl;
  	}

	// For Mac OS X < 10.5 (Leopard), you will need to comment out the
	//  setdirection line

	cout << "Opened up device " << m_szDevName << " successfully..." << endl;
	setName(m_szDevName);

	startThreads();	
	
/*	if(pcap_setnonblock(m_pDevice, 1, errbuf) <= 0) {
		cerr << "Unable to set " << m_szDevName << " into non-blocking mode" << endl;
		cerr << " Error is " << errbuf << endl;
		pcap_perror(m_pDevice, errbuf);
		cerr << "Try #2: " << errbuf << endl;
		exit(-1);
	}	*/
	
	//find_my_address(pcap_fileno(m_pDevice), m_szDevName, &m_MAC);
	
	// Start up the output thread
	//nResult = pthread_create(&m_ThreadOutput, NULL, Thread_Output, (void *) this);
	//if(nResult) {
	//	cerr << "* Error creating output thread for adapter " << m_szDevName << endl;
	//	cerr << "    Code: " << nResult << endl;
	//	exit(-1);
	//}
	
	//cout << "   Output thread started for " << m_szDevName << endl;
	
	return 0;
}	


void AdapterPCap::readPacket () {
	// Need to supplement this with a sleep that at least defers control
	// to someone else
	
	if(m_bBatchRead) {
		pcap_dispatch(m_pDevice,-1,pkt_callback,(u_char *) this);
	} else {
		pcap_dispatch(m_pDevice,-1,pkt_callback,(u_char *) this);
	}		
}

void AdapterPCap::setMAC (char * pMAC) {
	memcpy(m_byMAC, pMAC, ETH_MAC_LENGTH);
}

char * AdapterPCap::getMAC () {
	return m_byMAC;
}

void	 AdapterPCap::setReadPacket (Packet * pPacket) {
	m_pReadPacket = pPacket;	
}

char * AdapterPCap::getDevName () {
	return m_szDevName;
}


void AdapterPCap::setDevName (char * pDevName) {
	cout << "Setting device name to " << pDevName << endl;
	strncpy(m_szDevName, pDevName,PCAP_DEVICE_NAME_LEN);
}

void AdapterPCap::setDevName(string sDevName) {
	cout << "Setting device name to " << sDevName << endl;
	strncpy(m_szDevName, sDevName.c_str(), PCAP_DEVICE_NAME_LEN);
}

bool AdapterPCap::extractExtendedDOM (NodeDOM * pNode) {
	int		j;
	bool		bDevName;
	
	bDevName = false;
	
	for(j=0; j<pNode->getNumChildren(); j++) {
		if(pNode->getChild(j)->getTag() == "device") {
			setDevName(pNode->getChild(j)->getData());
			bDevName = true;
		}		
	}
	
	if(!bDevName) {
		cerr << "Error: Device name not specified for the pcap device" << endl;
		cerr << "   Device name (specified by <device> tag) is distinct from the <name>" << endl;
		cerr << "   tag that is only used for descriptive purposes. The <device> tag should" << endl;
		cerr << "   map to an actual device identity in the system (eth0, etc.)" << endl;
		return false;
	}
	
	return true;	
}

void	AdapterPCap::startDevice () {
	openDevice();
}

void	AdapterPCap::dumpBasicInfo  () {
	
}

void AdapterPCap::writePacket (Packet * pPacket) {
	int		nBytes;

	// All variants of Linux, Winpcap, and Mac OS X >= 10.5
	nBytes = pcap_inject(m_pDevice, pPacket->getData(), pPacket->getLength());	
	
	// Uncomment and comment the above line for Mac OS X 10.4
	//nBytes = write(pcap_fileno(m_pDevice), pPacket->getData(), pPacket->getLength());
	
	
	if(nBytes < 0) {
		cerr << "Error: Could not write the packet on the device " << m_szDevName << " of " << pPacket->getLength() << " bytes long." << endl;
	} 
}

		
