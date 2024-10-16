/* PktTime.cpp
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
 * $Revision: 1.4 $  $Date: 2008/04/11 22:52:11 $
 **********************************************************
 */

#include <iostream>
using namespace std;

#include <math.h>
 
#include "../mem/MemPoolCustom.h"
 
#include "RIPPS_PktPair.h"
#include "RIPPS_Monitor.h"

RIPPS_MonitorPacket::RIPPS_MonitorPacket () : MemPoolObject (MEMPOOL_OBJ_RIPPS_MONPKT) {

}

RIPPS_MonitorPacket::~RIPPS_MonitorPacket () {
	// Callback purging (if necessary)

	// TODO Fix callback
/*	if(m_pLinkCallback != NULL) {		
		m_pLinkCallback->clearCallback();
	} */
}

RIPPS_MonitorPacket * RIPPS_MonitorPacket::createMonPkt () {
	RIPPS_MonitorPacket *	pMonPkt;
	
	pMonPkt = (RIPPS_MonitorPacket *) g_MemPool.getObject(MEMPOOL_OBJ_RIPPS_MONPKT);		

	return pMonPkt;		
}


void RIPPS_MonitorPacket::clean () {
	m_nState = MONPKT_STATE_NONE;
	m_lAck = 0;
	m_pLinkCallback = NULL;
	m_lPktSize=0;
	
	// Default is no gobble
	m_bStingGobble = 0;
	
	// Initialize the time structs
	memset(&m_ArrTime, 0, sizeof(m_ArrTime));
	memset(&m_DepTime, 0, sizeof(m_DepTime));
	
	m_nDataSrcPkt = -1;
	m_nPktNum = -1;	
}

void RIPPS_MonitorPacket::setPktSize (unsigned int nSize) {
  m_lPktSize = nSize;
}

//////////////////////////////////////////////////////////////////////////////////////////////

void RIPPS_MonitorPacket::setCallback (Packet * pCallPkt) {	
	m_pLinkCallback = pCallPkt;
	pCallPkt->registerWriteCallback(NULL, this);
	// TODO Fix callback
	//pCallPkt->bCallBack = 1;	
	//pCallPkt->pCallPkt = this;
}

void RIPPS_MonitorPacket::formatShortDump (char * pBuffer) {
	//sprintf(pBuffer, "%ld,%ld,%ld,%ld,%ld,%d,%f", m_ArrTime.tv_sec, m_ArrTime.tv_usec, m_DepTime.tv_sec, m_DepTime.tv_usec,m_lAck,m_lPktSize,getLRTT()*1000.0);
   sprintf(pBuffer, "%ld %ld %ld %ld %ld %d %f G%d %d %d", m_ArrTime.tv_sec, m_ArrTime.tv_usec, m_DepTime.tv_sec, m_DepTime.tv_usec,m_lAck,m_lPktSize,getLRTT()*1000.0,m_bStingGobble,m_nDataSrcPkt,m_nPktNum);
}

void RIPPS_MonitorPacket::dumpBrief() {
	char		szTemp[257];
	
	formatShortDump(szTemp);
	
	cout << szTemp << endl;
}	

void RIPPS_MonitorPacket::dumpDetail () {
	cout << "       AT: " << m_ArrTime.tv_sec << " s, " << m_ArrTime.tv_usec << " us" << endl;
	
	if(m_nState == MONPKT_STATE_DEPART) {
		cout << "       DT: " << m_DepTime.tv_sec << " s, " << m_DepTime.tv_usec << " us" << endl;
		cout << "       LRTT: " << getLRTT() * 1000.0 << " ms" << endl;		
	}	
}

void RIPPS_MonitorPacket::setDataSrcPkt (int nPktNum) {
	m_nDataSrcPkt = nPktNum;	
}

int RIPPS_MonitorPacket::getDataSrcPkt () {
	return m_nDataSrcPkt;
}
	
void RIPPS_MonitorPacket::setPktNum 	  (int nPktNum) {
	m_nPktNum = nPktNum;
}

int RIPPS_MonitorPacket::	getPktNum 	  () {
	return m_nPktNum;	
}

void RIPPS_MonitorPacket::setFlag_Gobble (char bFlag) {
	m_bStingGobble = bFlag;	
}
	
char RIPPS_MonitorPacket::getFlag_Gobble () {
	return m_bStingGobble;	
}


void RIPPS_MonitorPacket::dumpAT () {
	cout << "  AT: " << m_ArrTime.tv_sec << " s, " << m_ArrTime.tv_usec << " us";
}

struct timeval * RIPPS_MonitorPacket::getArrivalTime () {
	return &m_ArrTime;
}

struct timeval * RIPPS_MonitorPacket::getDepartureTime () {
	return &m_DepTime;
}

int RIPPS_MonitorPacket::getState () {
	return m_nState;
}

char RIPPS_MonitorPacket::isMatched () {
	if(m_nState == MONPKT_STATE_DEPART) {
		return 1;
	} else {
		return 0;
	}
}

void RIPPS_MonitorPacket::setArrivalTime (struct timeval * pTime) {
	m_ArrTime = *pTime;
	m_nState = MONPKT_STATE_ARRIVE;
}

void RIPPS_MonitorPacket::setDepartureTime (struct timeval * pTime) {
	m_DepTime = *pTime;
	m_nState = MONPKT_STATE_DEPART;
}

long int RIPPS_MonitorPacket::getLRTT_us () {
	return 0;
}

float RIPPS_MonitorPacket::getLRTT () {
	float			fVal;
		
	if(m_DepTime.tv_sec == m_ArrTime.tv_sec) {
		fVal = m_DepTime.tv_usec - m_ArrTime.tv_usec;
		fVal = fVal / 1000000.0;
	} else {
		fVal = m_DepTime.tv_sec - m_ArrTime.tv_sec;
		fVal += m_DepTime.tv_usec / 1000000.0;
		fVal += (1000000 - m_ArrTime.tv_usec) / 1000000.0;	
		fVal -= 1;
	}
	
	return fVal;
}

void RIPPS_MonitorPacket::setAck (unsigned int nAck) {
	m_lAck = nAck;
}

unsigned int RIPPS_MonitorPacket::getAck () {
	return m_lAck;
}

List_RIPPSMonitorPkts::List_RIPPSMonitorPkts () {
	m_bNewData = 0;
	m_nCurrentPkt = 0;
}

List_RIPPSMonitorPkts::~List_RIPPSMonitorPkts () {
	RIPPS_MonitorPacket	*	pPacket;
	int					j;

	//cout << "     List_MonitorPkts destructor" << endl;

	// Delete all of the monitor packets since we own them
	
	for(j=0; j<m_ListOpen.size(); j++) {
		pPacket = m_ListOpen[j];
		pPacket->release();
	}
	
	for(j=0; j<m_ListMatched.size(); j++) {
		pPacket = m_ListMatched[j];
		pPacket->release();
	}
	
	for(j=0; j<m_ListRT.size(); j++) {
		pPacket = m_ListRT[j];
		pPacket->release();
	}	

	for(j=0; j<m_ListNoGobble.size(); j++) {
		pPacket = m_ListNoGobble[j];
		pPacket->release();
	}

	m_ListOpen.clear();
	m_ListMatched.clear();
	m_ListNoGobble.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////

bool List_RIPPSMonitorPkts::addOpenPacket (RIPPS_MonitorPacket * pPacket) {
	unsigned	int		lLastAck;
	unsigned  int	lExpAck;
	
	if(pPacket == NULL) {
		cerr << "Error: Attempted to add NULL packet to monitor list" << endl;
		return false;
	}
	
	// Track the order in which packets were added
	m_nCurrentPkt++;	
	pPacket->setPktNum(m_nCurrentPkt);
	
	// Nothing in the list, always add it then
	if(m_ListMatched.size() == 0) {
		m_ListOpen.push_back(pPacket);	
		return true;	
	}
	
	lLastAck = m_ListMatched[m_ListMatched.size()-1]->getAck();
	lExpAck = pPacket->getAck();
	
	// First case, the packet is greater than the last one, always add it
	if(lLastAck < pPacket->getAck()) {
		m_ListOpen.push_back(pPacket);	
		return true;				
	} else if (lLastAck == pPacket->getAck()) {
		m_ListOpen.push_back(pPacket);	
		return true;				
	} else {				
		// TODO - Issue with not being in order like we thought
		cerr << "Warning: Expected ack is not at the end of the list" << endl;
		cerr << "  Ignoring ack for now" << endl;
		cerr << "  Exp Ack to add: " << lExpAck << endl;
		cerr << "   Last in list is: " << lLastAck << endl;		
		m_Stats.Add_Stat(RIPPS_MONPKT_STAT_PKT_BADEXPACKORDER, 1);
		cerr.flush();	
		return false;
	}
}

void List_RIPPSMonitorPkts::noteRetransmission (RIPPS_MonitorPacket * pMonPkt) {
	m_ListRT.push_back(pMonPkt);	
}


RIPPS_MonitorPacket * List_RIPPSMonitorPkts::findPacket_byExpAck (unsigned int lExpAck) {
	int		j;

	//cout << "  Search ack: " << (unsigned int) lExpAck << endl;

	for(j=0; j<m_ListOpen.size(); j++) {
	  //cout << "    List ack: " << (unsigned int) (m_ListOpen[j])->getAck();
		//cout << "   ";
		//m_ListOpen[j]->dumpAT();
		//cout << endl;
		
		if((m_ListOpen[j])->getAck() == lExpAck) {
		  // List the match
		  //cout << "Hit on the " << j << "th packet in the list shown below" << endl;
		  //m_ListOpen[j]->dumpBrief();
			return (m_ListOpen[j]);
		}
	}
	
	return NULL;
}

char List_RIPPSMonitorPkts::matchesExistingRetransmission (RIPPS_MonitorPacket * pPacket) {
	int				j;

	for(j=0; j<m_ListRT.size(); j++) {
		if(m_ListRT[j]->getAck() == pPacket->getAck()) {
			return 1;	
		}	
	}			
	
	return 0;
}

char List_RIPPSMonitorPkts::applyMatchToPacket (RIPPS_MonitorPacket * pPacket) {
	
	vector<RIPPS_MonitorPacket *>::iterator		listIterate;
	int			j;
	char		bDelete;
	
	bDelete = 0;

	// A monitor packet is only valid if it has a reasonable arrival time.  If the arrival
	//  time is zero (i.e. never initialized, do not put this in the matched category 
	if(pPacket->getArrivalTime()->tv_sec != 0) {
		if(matchesExistingRetransmission(pPacket)) {
			// This saves the time of the retransmission and the ambiguous ACK that
			//  is likely a response to the retransmission itself			
			noteRetransmission(pPacket);
			bDelete = 0;		// Don't delete, the retransmit list gets it				
		} else if(pPacket->getLRTT() <= 0) {
			m_Stats.Add_Stat(RIPPS_MONPKT_STAT_PKT_OO, 1);
			bDelete = 1;		// Purge it, bad OO matching					
		} else if(!pPacket->getFlag_Gobble()) {
		  // For now, we are just going to ignore the no gobble packets as those are the
		  //  ones most likely to have the problem
		  
		  // TODO Option to monitor out acks
		  
		  //if(g_theMonitor.monitorOutAcks()) {
		  //  bDelete = 0;
		  //  m_ListMatched.push_back(pPacket);
		  //} else {
		    //m_ListNoGobble.push_back(pPacket);
		    
		    // We will allow these for now but we may have to come back and add in
		    //  flags down from the monitor if slicing was not enabled as we have
		    //  seen problems with these packets
		    
			 m_ListMatched.push_back(pPacket);
			 m_bNewData = 1;
		    
		    bDelete = 0;
		  //}
		} else {	
			m_ListMatched.push_back(pPacket);
			m_bNewData = 1;
		}
		
		bDelete = 0;
	} else {
		m_Stats.Add_Stat(RIPPS_MONPKT_STAT_DUMMY_MATCH, 1);
		bDelete = 1;
	}

	if(m_ListOpen.size() == 0) {
		cerr << "Error: List of open (unmatched packets) is zero length" << endl;
		cerr << "   List_MonitorPkts::applyMatchToPacket" << endl;
		
		if(bDelete) {
			pPacket->release();
		}
		
		return 1;
	}

	listIterate = m_ListOpen.begin();
	
	for(j=0; j<m_ListOpen.size(); j++) {
		if(*(listIterate) == pPacket) {
			m_ListOpen.erase(listIterate);
			break;
		}
		
		listIterate++;
	}
	
	if(bDelete) {
		pPacket->release();
		return 1;
	}
	
	// If this packet is to be gobbled, it cannot purge old acks	
	if(!pPacket->getFlag_Gobble()) {
		purgeOldAcks(pPacket->getAck(), ACK_GAP_LIMIT);
	}
	
	return 1;
}

void List_RIPPSMonitorPkts::purgeOldAcks (unsigned int lAck, unsigned int lNumLimit) {
	vector<RIPPS_MonitorPacket *>::iterator		listIterate;
	unsigned int		lDiff;
	RIPPS_MonitorPacket * pMonPkt;

	while(1) {
		//cout << "  List size is " << m_ListOpen.size() << endl;
		//cout.flush();
		
		if(m_ListOpen.size() <= 0) {
			break;
		}
		
		listIterate = m_ListOpen.begin();	
		
		lDiff = lAck - (*listIterate)->getAck();

		if(lDiff > 0 && lDiff < lNumLimit) {
			// Purge it
			//cout << "    Purged ack! " << (unsigned int) (*listIterate)->getAck();
			//cout << "   ";
			//(*listIterate)->dumpAT();
			//cout << endl;
			//cout.flush();
			
			pMonPkt = *listIterate;
			
			m_ListOpen.erase(listIterate);
			
			// If the arrival time was non-zero, it means it was an actual packet that
			//  we were trying to monitor, not a dummy packet
			if(!pMonPkt->getArrivalTime()->tv_sec != 0) {	
				m_Stats.Add_Stat(RIPPS_MONPKT_STAT_PURGED_ACKS, 1);
			} else {
			}
			
			pMonPkt->release();			
		} else {
			break;
		}			
	}
	
	
}

int List_RIPPSMonitorPkts::getCountOpen () {
	return m_ListOpen.size();
}

int	 List_RIPPSMonitorPkts::getCountMatches () {
	return m_ListMatched.size();
}

int	 List_RIPPSMonitorPkts::	getCountRetransmissions () {
	return m_ListRT.size();
}

int List_RIPPSMonitorPkts::getCountNoGobble () {
  return m_ListNoGobble.size();
}

RIPPS_MonitorPacket * List_RIPPSMonitorPkts::getNoGobble (int nIndex) {
  if(nIndex < 0 || nIndex >= m_ListNoGobble.size()) {
    return NULL;
  }

  return m_ListNoGobble[nIndex];
}

RIPPS_MonitorPacket * List_RIPPSMonitorPkts::getRetransmission (int nIndex) {
	if(nIndex < 0 || nIndex >= m_ListRT.size()) {
		return NULL;
	}
	
	return m_ListRT[nIndex];
}

double List_RIPPSMonitorPkts::computeStdDev		() {
	double			fAvg;
	int				j;
	
	double			fSum;
	double			fTerm;
	
	fAvg = computeLRTT();
	fSum = 0;
	
	for(j=0; j<m_ListMatched.size(); j++) {
		fTerm = (m_ListMatched[j]->getLRTT() - fAvg) * (m_ListMatched[j]->getLRTT() - fAvg);
		fSum += fTerm;
	}	
	
	fSum = fSum / (double) m_ListMatched.size();
	fSum = sqrt(fSum);
	
	return fSum;
}

float List_RIPPSMonitorPkts::computeLRTT (int nWindow) {
	vector<RIPPS_MonitorPacket *>::iterator		listIt;
	float		fCumSum;
	int			nCount;

	if(m_bNewData == 0) {
		return m_fLRTT;
	}
	
	fCumSum = 0;
	nCount = 0;

	if(nWindow == 0) {
		nWindow = m_ListMatched.size();
	}
	
	if(nWindow > getCountMatches()) {
		nWindow = getCountMatches();
	}
		
	RIPPS_MonitorPacket *	pMonPkt;	
		
	while(nWindow > 0) {
//		cout << "  Iterating with " << nWindow << " packet matches left" << endl;
		
		pMonPkt = m_ListMatched[m_ListMatched.size() - nCount - 1];
//		pMonPkt->dumpDetail();
		
		fCumSum += pMonPkt->getLRTT();
		nCount++;
		
		nWindow--;		
	}
	
	m_fLRTT = fCumSum / (float) nCount;
	m_bNewData = 0;
	
	return m_fLRTT;
}

void List_RIPPSMonitorPkts::dumpBriefList_Open () {	
	int			j;
	RIPPS_MonitorPacket * pMonPkt;
	
	cout << "  List_MonitorPkts containing " << m_ListOpen.size() << " open pkts" << endl;
	
	for(j=0; j<m_ListOpen.size(); j++) {
		pMonPkt = m_ListOpen[j];
		pMonPkt->dumpBrief();
	}
}

void List_RIPPSMonitorPkts::dumpBrief () {
	cout << "      ";
	cout << "  LRTT: ";
	
	if(getCountMatches() > 0) {
		cout << computeLRTT() * 1000.0 << " ms from " << getCountMatches() << " pkts" << endl;
	} else {
		cout << "N/A" << endl;
	}
}

void List_RIPPSMonitorPkts::dumpDetail () {
	cout << "      ";
	cout << "  LRTT: ";
	
	if(getCountMatches() > 0) {
		cout << computeLRTT() * 1000.0 << " ms on " << getCountMatches() << endl;
	} else {
		cout << "N/A" << endl;
	}
	
	cout << "      Individual Packet Chars";
	
	unsigned int		j;
	RIPPS_MonitorPacket *	pMonPkt;
	
	for(j=0; j<m_ListMatched.size(); j++) {
		pMonPkt = m_ListMatched[j];
		pMonPkt->dumpDetail();
	} 
}

void List_RIPPSMonitorPkts::purgeOld (float fTime) {

}

RIPPS_ListMonPktsStats * List_RIPPSMonitorPkts::getStatsObject() {
	return &m_Stats;
}

RIPPS_MonitorPacket * List_RIPPSMonitorPkts::getMonPkt (int nPkt) {
	if(nPkt < 0 || nPkt >= m_ListMatched.size()) {
		return NULL;
	} else {
		return m_ListMatched[nPkt];
	}	
}

RIPPS_ListMonPktsStats::RIPPS_ListMonPktsStats() {
	Allocate(RIPPS_MONPKT_STAT_LAST);
}

RIPPS_ListMonPktsStats::RIPPS_ListMonPktsStats (RIPPS_ListMonPktsStats * pCopy) {
	int		j;
	
	Allocate(RIPPS_MONPKT_STAT_LAST);
	
	for(j=0; j<RIPPS_MONPKT_STAT_LAST; j++) {
		Set_Stat(j, pCopy->Get_Stat(j));	
	}
}

