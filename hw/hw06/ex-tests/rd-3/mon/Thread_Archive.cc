/* Thread_Archive.cpp
 **********************************************************
 * This code is part of the RIPPS (Rogue Identifying
 * Packet Payload Slicer) system developed at the University
 * of Notre Dame. 
 *
 * PI:       Dr. Aaron Striegel    striegel@nd.edu
 * 
 * Students: Chad Mano (lead)      Yingxin Jiang
 *           Dave Salyers          Dave Cieslak
 *           Qi Liao               Andrew Blaich   
 * 
 **********************************************************
 * $Revision: 1.1.1.1 $  $Date: 2008/02/13 04:21:21 $
 **********************************************************
 */

#include "Thread_Archive.h"
#include "Monitor.h"
#include "Host.h"
#include "Connection.h"

#include "mgmt.h"

Archiver		g_theArchive;

ArchiveItem::ArchiveItem () {
	
}

ArchiveItem::~ArchiveItem () {
	m_pData = NULL;
	m_nType = ARCHIVE_ITEM_NONE;
}

ArchiveItem::ArchiveItem (char * pString) {
	int			nLength;
	
	nLength = strlen(pString);
	
	if(nLength > 0) {
		m_pData = new char[nLength+1];
		m_nType = ARCHIVE_ITEM_LOGSTRING;
		strncpy((char *) m_pData, pString,nLength);
	} else {
		m_nType = ARCHIVE_ITEM_NONE;
		m_pData = NULL;
	}
}

void ArchiveItem::setContent (int nType, void * pData) {
	m_nType = nType;
	m_pData = pData;	
}

int ArchiveItem::getType () {
	return m_nType;
}

void * ArchiveItem::getData () {
	return m_pData;
}

Archiver::Archiver () {
	pthread_mutex_init(&m_ArchiveBuf, NULL);	
}

Archiver::~Archiver () {
	
}

void Archiver::addItem (ArchiveItem * pItem) {
	pthread_mutex_lock(&m_ArchiveBuf);
	
	m_Queue.push_back(pItem);
	
	pthread_mutex_unlock(&m_ArchiveBuf);
}

void Archiver::initialize () {
	m_LogFile.open("Log-RIPPS.txt", ofstream::out | ofstream::app);	
}

void Archiver::archiveString(char * pString) {
	ArchiveItem	*	pItem;
	pItem = new ArchiveItem(pString);
	addItem(pItem);	
}

void Archiver::archivePacket  (PacketHolder * pPacket) {
	char		szOutInfo[1025];
	
	pPacket->dumpBriefString(szOutInfo, 1024);
	archiveString(szOutInfo);	
}

void Archiver::archiveTime () {
	struct timeval		curTime;

	gettimeofday(&curTime, NULL);
	
	struct tm	 *  loctime;
	time_t		theTime;

	char		szOutBuffer[256];
	
	theTime = curTime.tv_sec;
	loctime = localtime(&theTime);

	strftime (szOutBuffer, 256, "%d %b %y %H:%M:%S", loctime);
	
	ArchiveItem * pItem;
	pItem = new ArchiveItem(szOutBuffer);
	addItem(pItem);
}


void Archiver::logConnection (ArchiveItem * pItem) {
	Connection *		pConn;
	char	szOutFile[256];
	int		nLength;
	
	pConn = (Connection *) pItem->getData();
	
	strcpy(szOutFile, "conlogs/");
	
	nLength = strlen(szOutFile);
	
	pConn->determineLogName(szOutFile+nLength);
	
	pConn->dumpFile(szOutFile);
	
	//cout << " Cleaning up the connection object" << endl;
	delete pConn;
}

void Archiver::logString (ArchiveItem * pItem) {
	char *		pString;
	
	pString = (char *) (pItem->getData());
	
	m_LogFile << pString;
	
	delete pString;
}


char Archiver::doArchive () {
	ArchiveItem	* pItem;
	
	pthread_mutex_lock(&m_ArchiveBuf);
	
	if(m_Queue.size() > 0) {
		pItem = m_Queue[0];
		m_Queue.pop_front();
		
		// Done with the buffer manipulation, release it so our
		// disk writes do not block other threads from adding to the
		// archive buffer
		pthread_mutex_unlock(&m_ArchiveBuf);

		switch(pItem->getType()) {
			case ARCHIVE_ITEM_CONNECTION:
				logConnection(pItem);
				break;
			case ARCHIVE_ITEM_LOGSTRING:
				logString(pItem);
				break;
			default:
				cerr << "Error: Unknown archival item type (" << pItem->getType() << ")" << endl;
				break;	
		}
		
		delete pItem;
		
		return 1;
	} else {
		pthread_mutex_unlock(&m_ArchiveBuf);	
		return 0;	
	}
}

void * Thread_Archive (void * pArg) {
	int			nValidate;
	
	nValidate = 0;
	
	
	while(g_theMonitor.isMonitorRunning()) {
		if(!g_theArchive.doArchive()) {
			// Nothing in the archive queue, go to sleep for 10 milliseconds
			yieldCPU();
//			usleep(10000);						
		}
		
		nValidate++;
		
		if(nValidate >= 20) {
			nValidate = 0;
			g_theMonitor.checkValidations();
			g_theMonitor.checkSummarizeConnections();
		}		
		
	}
	
	return NULL;
}


