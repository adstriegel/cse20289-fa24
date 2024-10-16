/* PacketCacheModule.cc
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
 * $Revision: 1.10 $  $Date: 2008/04/11 22:52:47 $
 **********************************************************
 */

#include <iostream>
#include <fstream>
#include <sys/time.h>
using namespace std;

#include "../pkt/Packet.h"

#include "../xml/NodeDOM.h"

#include "../mon/Monitor.h"			// Monitor related functions

#include "PacketCacheModule.h"
#include "../util/ip-utils.h"


PacketCacheModule::PacketCacheModule () : IOModule () {
  setCacheID("PktCache");
  m_lNumPkts = 0;
  m_lNumHits = 0;
  m_lTotalBytes = 0;
  m_lHitBytes = 0;
  m_lMatchBytes = 0;
  m_lInstRecvBytes = 0;
  m_lInstMatchBytes = 0;
}

PacketCacheModule::~PacketCacheModule () {
}

char PacketCacheModule::initialize () {
	return 1;	
}

char PacketCacheModule::processPacket (Packet * pPacket) {
  fp_msg_t * fpPkt;
  static struct timeval start_time;
  static long time,prev_time;

  m_lNumPkts++;
  if(m_lNumPkts == 1){
    memcpy(&start_time, pPacket->getArrivalTime(),sizeof(struct timeval));
    prev_time = -1;
  }
  m_lTotalBytes += pPacket->getLength();
  m_lInstRecvBytes += pPacket->getLength();

  if(m_lNumPkts%100==0){
  //  if(1){
    time=calcTimeDiff(pPacket->getArrivalTime(), &start_time);
    cout <<"   time " <<time/1000 << "    prev_t " << prev_time <<"  numPkts " <<m_lNumPkts << "  totalBytes " <<m_lTotalBytes << "  BW_usage " <<m_lTotalBytes/time << "  instBytes " << m_lInstRecvBytes << "   inst_usage " << m_lInstRecvBytes/(time-prev_time) << endl;
      m_lInstRecvBytes = 0;
      prev_time=time;
  }
  return 0; //use temporarily to test link usage

  // ignore the packet whose size is less than 100 bytes
  if(pPacket->getLength()<100) return 0;
  
  m_lNumPkts++;
  if(m_lNumPkts == 1){
    memcpy(&start_time, pPacket->getArrivalTime(),sizeof(struct timeval));
    //cout << "time diff " <<   calcTimeDiff(pPacket->getArrivalTime(), &start_time)/1000 <<endl;
    //exit(0);
  }
  m_lTotalBytes += pPacket->getLength();
  m_lInstRecvBytes += pPacket->getLength();
  fpPkt=m_pktCacheTable.fpHashMatch(pPacket,1);
  if(fpPkt && fpPkt->num>0){
    m_lNumHits++;
    //m_lHitBytes += fpPkt->len;
    m_lMatchBytes += fpPkt->mbytes;
    m_lInstMatchBytes += fpPkt->mbytes;
  }

  if(m_lNumPkts%100==0){
      //cout <<m_sCacheID <<"   numPkts " <<m_lNumPkts <<"   numHits " <<m_lNumHits << "   hitRate " << (float)m_lNumHits/m_lNumPkts << "   totalBytes " <<m_lTotalBytes << "   hitBytes " <<m_lHitBytes << "   " << (float)m_lHitBytes/m_lTotalBytes<< endl;
    cout <<m_sCacheID  << "   time " <<calcTimeDiff(pPacket->getArrivalTime(), &start_time)/1000 <<"  numPkts " <<m_lNumPkts <<"  numHits " <<m_lNumHits << "  totalBytes " <<m_lTotalBytes << "  hitBytes " <<m_lMatchBytes << "   " << (float)100*m_lMatchBytes/m_lTotalBytes << "  instBytes " << m_lInstRecvBytes << "  instMatch " << m_lInstMatchBytes << "   " << (float)100*m_lInstMatchBytes/m_lInstRecvBytes << endl;

    //cout.flush();
      m_lNumHits = 0;
      m_lInstRecvBytes = 0;
      m_lInstMatchBytes = 0;
  }


  //exit(0);
  return 0;
}
				
bool PacketCacheModule::extractExtendedDOM (NodeDOM * pNode) {
	int			j;
	
	for(j=0; j<pNode->getNumChildren(); j++) {
		if(pNode->getChild(j)->getTag() == "CacheID") {
			m_sCacheID = pNode->getChild(j)->getData();
		}
	}
	return true;	
}

		
string	 PacketCacheModule::getCacheID () {
	return m_sCacheID;
}

void PacketCacheModule::setCacheID (string sID) {
	m_sCacheID = sID;
}



		
