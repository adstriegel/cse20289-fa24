/* TWiCE_Gateway.cc
 **********************************************************
 * TWiCE: Transparent Wireless Capacity Enhancement
 **********************************************************
 * TWiCE is an extension to the ScaleBox code that focuses
 * on improvements to wireless capacity by the clever removal
 * of redundant content.
 *
 * Inspiration for TWiCE is drawn from results observed as
 * part of the NetSense cell phone study.  TWiCE is currently
 * being modernized as Proximity Secure Mixing (PSM).
 *
 * For additional information, please visit the NetScale
 * laboratory webpage at:
 *
 * 	http://netscale.cse.nd.edu/
 *
 * PI:       Dr. Aaron Striegel    striegel@nd.edu
 *
 * Students: Xueheng Hu			   xhu2@nd.edu
 *
 * Prior students that contributed to the precursors to TWiCE
 * and PSM include Dave Salyers, Yingxin Jiang, and Xiaolong Li.
 *
 ************************************************************
 * Support for the development of TWiCE was drawn in part from
 * sources including the National Science Foundation, Sprint,
 * Alcatel-Lucent, and the University of Notre Dame.
 ************************************************************
 */

#include <iostream>
using namespace std;

#include "TWiCE_Gateway.h"

#include "../mon/Thread_Archive.h"
#include "../mon/Thread_Timer.h"

#include "../xml/NodeDOM.h"
#include "../xml/NodeElem.h"

#include "../util/ip-utils.h"

/////////////////////////////////////////////////////////////////////////

TWiCE_GatewayStats::TWiCE_GatewayStats () : Stats ()
{
  Allocate(TWICE_GATEWAY_STAT_LAST);
}

/////////////////////////////////////////////////////////////////////////

void	TWiCE_GatewayStats::Get_Title	(int nStat, char * szTitle)
{
	switch(nStat)
	{
		case TWICE_GATEWAY_WAN_WCACHE_CLONE_PKT:
			sprintf(szTitle, "ClonePool");
			break;
		case TWICE_GATEWAY_WAN_PKTS:
			sprintf(szTitle, "In_Pkts_INet");
			break;
		case TWICE_GATEWAY_WAN_VOL:
			sprintf(szTitle, "In_Vol_INet");
			break;
		case TWICE_GATEWAY_WAN_WCACHE_TOO_SMALL_PKTS:
			sprintf(szTitle, "CountTooSmall");
			break;
		case TWICE_GATEWAY_WAN_WCACHE_HIT_MULTI:
			sprintf(szTitle, "WCache_Multi_Hit");
			break;
		case TWICE_GATEWAY_WAN_PKTS_IPV6:
			sprintf(szTitle, "CountIPv6");
			break;
		case TWICE_GATEWAY_WAN_PKTS_TCP:
			sprintf(szTitle, "CountTCP");
			break;
		case TWICE_GATEWAY_WAN_PKTS_UDP:
			sprintf(szTitle, "CountUDP");
			break;
		case TWICE_GATEWAY_WAN_PKTS_IPV4:
			sprintf(szTitle, "CountIPv4");
			break;
		case TWICE_GATEWAY_WAN_PKTS_OTHER:
			sprintf(szTitle, "CountNonIP");
			break;
		case TWICE_GATEWAY_WAN_MATCH_COUNT_LTE96:
			sprintf(szTitle, "CountMatch96LTE");
			break;
		case TWICE_GATEWAY_WAN_MATCH_COUNT_LTE128:
			sprintf(szTitle, "CountMatch128LTE");
			break;
		case TWICE_GATEWAY_WAN_MATCH_COUNT_LTE256:
			sprintf(szTitle, "CountMatch256LTE");
			break;
		case TWICE_GATEWAY_WAN_MATCH_COUNT_LTE512:
			sprintf(szTitle, "CountMatch512LTE");
			break;
		case TWICE_GATEWAY_WAN_MATCH_COUNT_LTE1024:
			sprintf(szTitle, "CountMatch1024LTE");
			break;
		case TWICE_GATEWAY_WAN_MATCH_COUNT_GT1024:
			sprintf(szTitle, "CountMatch1024GT");
			break;

		case TWICE_GATEWAY_WAN_PKTS_MATCH:
			sprintf(szTitle, "CountMatchPkts");
			break;

		case TWICE_GATEWAY_WAN_PKTS_TCP_MATCH:
			sprintf(szTitle, "CountMatchTCP_Pkts");
			break;

		case TWICE_GATEWAY_WAN_PKTS_UDP_MATCH:
			sprintf(szTitle, "CountMatchUDP_Pkts");
			break;


		default:
			sprintf(szTitle, "F%d", nStat);
			break;
	}
}

/////////////////////////////////////////////////////////////////////////

TWiCE_Gateway::TWiCE_Gateway ()
{
	m_pCacheTable = new TWiCECacheTable();
	m_pPacketPool = new TWiCEPacketPool();
	m_pCacheTable->setPacketPool(m_pPacketPool);

	m_nMinPktSizeTryCache = TWICEGW_DEFAULT_MINPKTSIZETOCACHE;

	m_nInterval_PoolMaintenance = TWICEGW_TIMER_POOL_MAINTENANCE_INTERVAL;
}

TWiCE_Gateway::~TWiCE_Gateway ()
{
	delete m_pCacheTable;
	delete m_pPacketPool;
}


void TWiCE_Gateway::setInterval_PoolMaintenance (unsigned int lNewVal)
{
	if(lNewVal > 0)
	{
		m_nInterval_PoolMaintenance = lNewVal;
	}
}

unsigned int TWiCE_Gateway::getInterval_PoolMaintenance ()
{
	return m_nInterval_PoolMaintenance;
}

char TWiCE_Gateway::initialize ()
{
	TimerEvent	*		pTimer;

	// TODO: Add in a mutex for accessing the packet pool listing
	m_pPacketPool->initialize();
	m_pCacheTable->initialize();

	enableTimers();

	// Create a timer for the pool maintenance
	pTimer = TimerEvent::createTimer();

	pTimer->setTimerID(TWICEGW_TIMER_ID_POOL_MAINTENANCE);

	// Set the current time
	gettimeofday(&m_nextPoolMaintenance, NULL);

	// Compute it for the next interval
	pTimer->setInterval_ms(getInterval_PoolMaintenance() / 1000);
	pTimer->setInvocationTime(computeNextPoolMaintenance(0));

	// Do something with the timer :)
	if(!addTimerEvent(pTimer)) {
		pTimer->release();
	}

	return 1;
}

struct timeval * TWiCE_Gateway::computeNextPoolMaintenance (char byTimeMode)
{
	// Set it from the current time in the listing
	adjustTime(&m_nextPoolMaintenance, m_nInterval_PoolMaintenance);
	return &m_nextPoolMaintenance;
}

struct timeval * TWiCE_Gateway::computeNextLogWrite (char byTimeMode)
{
	// Set it from the current time in the listing
	adjustTime(&m_nextLogWrite, TWICEGW_TIMER_LOG_WRITE_INTERVAL);
	return &m_nextLogWrite;
}


bool TWiCE_Gateway::extractExtendedDOM (NodeDOM * pNode) {
	int		j;
	string	sTag;
	string	sData;

	for(j=0; j<pNode->getNumChildren(); j++) {



	}

	return true;
}

bool TWiCE_Gateway::processTimer (TimerEvent * pEvent)
{
	cout << "*** TWiCE: Processing a timer of value " << pEvent->getTimerID() << endl;

	switch(pEvent->getTimerID())
	{
		case TWICEGW_TIMER_ID_POOL_MAINTENANCE:
			// Clean up at the packet pool

			cout << "    Timer: Pool Maintenance" << endl;

			//m_pPacketPool->doMaintenance();

			pEvent->armTimer();

			if(!addTimerEvent(pEvent))
			{
				cerr << "Error: Unable to arm the timer for the Packet Pool Maintenance" << endl;
				pEvent->release();
				return 0;
			}

			cout << "    Timer: Pool Maintenance (Complete)" << endl;

			break;

		case TWICEGW_TIMER_ID_LOG_WRITE:
//			cout << "  Log write" << endl;

			// Clean up at the packet pool
			logStatsToFile();

			pEvent->armTimer();

			if(!addTimerEvent(pEvent))
			{
				cerr << "Error: Unable to arm the timer for the Stat Log Writing" << endl;
				pEvent->release();
				return 0;
			}

			break;


	}

	return true;
}

NodeDOM * TWiCE_Gateway::getStats (NodeDOM * pRoot)
{
	return NULL;
}

void TWiCE_Gateway::logStatsToFile ()
{
	// Compute the difference for the various stats
	m_DiffStats.computeDiff(&m_PriorStats,&m_Stats);
	m_PriorStats.syncAll(&m_Stats);

	// Compute the cache diff stats
	m_pCacheTable->updateStats_Diff();
	m_pCacheTable->saveStats_Prior();

	// Log the time

//	cout << "Logging stats" << endl;

	struct timeval 	cTime;

	gettimeofday(&cTime, NULL);
	m_LogFile << cTime.tv_sec << ",";

	// Log the stats to the file

	// Our money stats

	/////////////////////////////////////////////////////////////////////////////
	// Overall stats -> all time since we have been logging (cumulative)

	// Overall hit length
	if(m_Stats.Get_Stat(TWICE_GATEWAY_WAN_WCACHE_HITS) > 0)
	{
		m_LogFile << (double) m_Stats.Get_Stat(TWICE_GATEWAY_WAN_WCACHE_HIT_LENGTH) / (double) m_Stats.Get_Stat(TWICE_GATEWAY_WAN_WCACHE_HITS) << ",";
	}
	else
	{
		m_LogFile << "0,";
	}

	// OK, how much volume-wise have we saved (could be good, could be bad)
	if (m_Stats.Get_Stat(TWICE_GATEWAY_WAN_VOL) > 0)
	{
		m_LogFile << (double) m_Stats.Get_Stat(TWICE_GATEWAY_WAN_WCACHE_HIT_LENGTH) / (double) m_Stats.Get_Stat(TWICE_GATEWAY_WAN_VOL) * 100.0 << ",";
	}
	else
	{
		m_LogFile << "0,";
	}

	/////////////////////////////////////////////////////////////////////////////
	// Last period stats -> since the last log recording

	// Average hit length
	if(m_DiffStats.Get_Stat(TWICE_GATEWAY_WAN_WCACHE_HITS) > 0)
	{
		m_LogFile << (double) m_DiffStats.Get_Stat(TWICE_GATEWAY_WAN_WCACHE_HIT_LENGTH) / (double) m_DiffStats.Get_Stat(TWICE_GATEWAY_WAN_WCACHE_HITS) << ",";
	}
	else
	{
		m_LogFile << "0,";
	}

	// OK, how much volume-wise have we saved (could be good, could be bad)
	if (m_DiffStats.Get_Stat(TWICE_GATEWAY_WAN_VOL) > 0)
	{
		m_LogFile << (double) m_DiffStats.Get_Stat(TWICE_GATEWAY_WAN_WCACHE_HIT_LENGTH) / (double) m_DiffStats.Get_Stat(TWICE_GATEWAY_WAN_VOL) * 100.0 << ",";
	}
	else
	{
		m_LogFile << "0,";
	}

	// Actual content so far
	m_DiffStats.logCSV(m_LogFile);

	m_LogFile << "AVG,";

	// Time-based average of everything as well
	m_DiffStats.logCSV_TimeAvg(m_LogFile, 60);

	m_LogFile << "CACHE,";

	TWiCE_CacheTableStats * pCacheStats;

	pCacheStats = m_pCacheTable->getStats_Diff();
	pCacheStats->logCSV(m_LogFile);


	m_LogFile << "AVGCACHE,";
	pCacheStats->logCSV_TimeAvg(m_LogFile,60);


	m_LogFile << endl;
	m_LogFile.flush();
}


CommandResult TWiCE_Gateway::processExtCommand (const vector<string> & theCommands, int nOffset)
{
	if(theCommands.size() <= nOffset) {
		return PROCESS_CMD_SYNTAX;
	}

//	cout << " Processing a command at the TWiCE module" << endl;

	if(theCommands[nOffset] == "help") {

		cout << " TWiCE Module Help" << endl;
		cout << "   startlog   Enable logging" << endl;

		return PROCESS_CMD_SUCCESS;
	}
	else if(theCommands[nOffset] == "startlog") {
		if(theCommands.size() == nOffset+1)
		{
			struct timeval cTime;

			gettimeofday(&cTime, NULL);

			// The next argument is the name of the file to use

			char	szTime[80];

			sprintf(szTime, "%d.csv", cTime.tv_sec);

			m_LogFile.open(szTime);

			m_LogFile << "Time" << ",";

			m_LogFile << "Cum_Avg_Length" << ",";
			m_LogFile << "Cum_Pct_Saved" << ",";

			m_LogFile << "Avg_Length" << ",";
			m_LogFile << "Pct_Saved" << ",";

			m_Stats.logCSV_FieldNames(m_LogFile);
			m_LogFile << "AVG,";
			m_Stats.logCSV_FieldNames(m_LogFile);

			m_LogFile << "CACHE,";

			TWiCE_CacheTableStats * pCacheStats;

			pCacheStats = m_pCacheTable->getStats();
			pCacheStats->logCSV_FieldNames(m_LogFile);

			m_LogFile << "AVGCACHE,";
			pCacheStats->logCSV_FieldNames(m_LogFile);


			m_LogFile << endl;

			m_PriorStats.Reset();
			logStatsToFile();

			TimerEvent * pTimer;

			// Create a timer for the pool maintenance
			pTimer = TimerEvent::createTimer();

			if(pTimer != NULL)
			{

				pTimer->setTimerID(TWICEGW_TIMER_ID_LOG_WRITE);
				pTimer->setInterval_ms(TWICEGW_TIMER_LOG_WRITE_INTERVAL/1000);

				// Set the current time
				gettimeofday(&m_nextLogWrite, NULL);

				// Compute it for the next interval
//				pTimer->armTimer();
				pTimer->setInvocationTime(computeNextLogWrite(0));

				//cout << "Adding the timer" << endl;

				// Do something with the timer :)
				if(!addTimerEvent(pTimer)) {
					cerr << "Could not add the timer, releasing it" << endl;
					pTimer->release();
				}
			}

			//cout << "Adding the timer is done" << endl;

			return PROCESS_CMD_SUCCESS;
		}
	}
	else if(theCommands[nOffset] == "status") {
		if(theCommands.size() < nOffset+2)
		{

		}
		else
		{
			if(theCommands[nOffset+1] == "pool")
			{
				m_pPacketPool->dumpStatus();
			}
			else if (theCommands[nOffset+1] == "cache")
			{
				m_pCacheTable->dumpStatus();
			}
			else if (theCommands[nOffset+1] == "perf")
			{
				cout << "TWiCE Performance Snapshot" << endl;
				cout << "  " << m_Stats.Get_Stat(TWICE_GATEWAY_WAN_PKTS) << " pkts, " << m_Stats.Get_Stat(TWICE_GATEWAY_WAN_VOL) << " bytes on WAN" << endl;


				// Quick performance assessment
				cout << "  -> Total Hits:      " << m_Stats.Get_Stat(TWICE_GATEWAY_WAN_WCACHE_HITS) << endl;
				cout << "  -> Total Width:     " << m_Stats.Get_Stat(TWICE_GATEWAY_WAN_WCACHE_HIT_LENGTH) << endl;

				if(m_Stats.Get_Stat(TWICE_GATEWAY_WAN_WCACHE_HITS) > 0)
				{
					cout << "  -> Avg Save / Hit:  " << (double) m_Stats.Get_Stat(TWICE_GATEWAY_WAN_WCACHE_HIT_LENGTH) / (double) m_Stats.Get_Stat(TWICE_GATEWAY_WAN_WCACHE_HITS) << endl;
				}

				// OK, how much volume-wise have we saved (could be good, could be bad)
				if (m_Stats.Get_Stat(TWICE_GATEWAY_WAN_VOL) > 0)
				{
					cout << "  -> B/W Savings:     " << (double) m_Stats.Get_Stat(TWICE_GATEWAY_WAN_WCACHE_HIT_LENGTH) / (double) m_Stats.Get_Stat(TWICE_GATEWAY_WAN_VOL) * 100.0 << endl;
				}
			}
		}

		return PROCESS_CMD_SUCCESS;
	}
	else if(theCommands[nOffset] == "top") {
		if(theCommands.size() < nOffset+3)
		{

		}
		else
		{
			if(theCommands[nOffset+1] == "cache" && theCommands[nOffset+2] == "hits")
			{
				m_pCacheTable->dumpTopList(TWICE_CACHE_RANKING_HITS);
			}
		}

		return PROCESS_CMD_SUCCESS;
	}

	return PROCESS_CMD_UNKNOWN;
}

bool TWiCE_Gateway::processWriteCallback (Packet * pPacket, void * pData) {
	return 1;
}


char TWiCE_Gateway::processTapPacket (Packet * pPacket, int nID)
{
  switch(nID)
    {
    case TWICE_TAP_WAN:
    	return processPacket_WAN(pPacket);
      break;
    case TWICE_TAP_WIRELESS:
      break;

    }
}

int TWiCE_Gateway::mapNameToID (string sName)
{

  if(sName == "WAN")
    {
      return TWICE_TAP_WAN;
    }
  else if(sName == "WIRELESS")
    {
      return TWICE_TAP_WIRELESS;
    }
  else
    {
      return TWICE_TAP_NONE;
    }
}

// Process a packet that came in from the Internet side of things

char TWiCE_Gateway::processPacket_WAN (Packet * pPacket)
{
	cout << "TWiCE WAN Packet" << endl;

  // Process the packet that came in on the WAN side
  m_Stats.Add_Stat(TWICE_GATEWAY_WAN_PKTS, 1);
  m_Stats.Add_Stat(TWICE_GATEWAY_WAN_VOL, pPacket->getLength());

  if(!isPacketIP(pPacket->getData(), pPacket->getLength()))
  {
      m_Stats.Add_Stat(TWICE_GATEWAY_WAN_PKTS_OTHER, 1);
      m_Stats.Add_Stat(TWICE_GATEWAY_WAN_VOL_OTHER, pPacket->getLength());
      return 0;
  }

  // Is this packet IPsec?

  char		byIPVersion;
  int 		nPayloadOffset;

  byIPVersion = getIPVersion(pPacket->getData(), pPacket->getLength());
  nPayloadOffset = 0;

  // Yes, I am being bad but that is OK

  switch(byIPVersion)
  {
  	  case 6:
  		  // IPv6 - Not supported for now

  		  m_Stats.Add_Stat(TWICE_GATEWAY_WAN_PKTS_IPV6, 1);
  		  m_Stats.Add_Stat(TWICE_GATEWAY_WAN_VOL_IPV6, pPacket->getLength());

  		  // Pretty easy to add this in but we will play coy for now

  		  // TODO: Add IPv6 Support for parsing / offset calculations

  		  return 0;

  	  case 4:
  		  // IPv4 - Definitely supported :)

  		  m_Stats.Add_Stat(TWICE_GATEWAY_WAN_PKTS_IPV4, 1);
  		  m_Stats.Add_Stat(TWICE_GATEWAY_WAN_VOL_IPV4, pPacket->getLength());

  		  // Is this IPsec?
  		  //
  		  //  We do not touch IPsec as it is pretty much pointless

  		  if(isPacketIPv4IPsec(pPacket->getData(), pPacket->getLength()))
  		  {
  	  		  m_Stats.Add_Stat(TWICE_GATEWAY_WAN_PKTSV4_ENC, 1);
  	  		  m_Stats.Add_Stat(TWICE_GATEWAY_WAN_VOLV4_ENC, pPacket->getLength());
  	  		  return 0;
  		  }

  		  // Should we check HTTPS?


  		  // Fill in the payload offset

  		  switch(getIPv4Protocol(pPacket->getData()))
  		  {
  		  	  case IP_PROTOCOL_TCP:
  		  		  m_Stats.Add_Stat(TWICE_GATEWAY_WAN_PKTS_TCP, 1);
  		  		  m_Stats.Add_Stat(TWICE_GATEWAY_WAN_VOL_TCP, pPacket->getLength());
  		  		  nPayloadOffset = getTCP_HeaderSize(pPacket->getData(), pPacket->getLength()) + getIP_HeaderSize(pPacket->getData(), pPacket->getLength()) + L2_OFFSET;
  		  		  break;

  		  	  case IP_PROTOCOL_UDP:
  		  		  m_Stats.Add_Stat(TWICE_GATEWAY_WAN_PKTS_UDP,1);
  		  		  m_Stats.Add_Stat(TWICE_GATEWAY_WAN_VOL_UDP, pPacket->getLength());
  		  		  nPayloadOffset = OFFSET_UDP_DATA + getIP_HeaderSize(pPacket->getData(), pPacket->getLength()) + L2_OFFSET;
  		  		  break;

  		  	  default:
  		  		  // Hmmm, dunno
  		  		  return 0;
  		  }

  		  break;
  	  default:
  		  break;
  }

  // Our order of operations is as follows
  //
  //  1. Wireless Packet Caching

  cout << "   Attempting to cache / analyze the packet" << endl;

  if(pPacket->getLength() >= getMinimumPacketSizeToTryCache())
  {
	  Packet *	pCacheResult;
	  int		nResult;
	  char      bSavedInPool;
//	  Packet *	pPacketCopyForPool;

	  cout << "     Packet is long enough - prepping packet" << endl;

	  TWiCEPacketPoolEntry * 	pPoolEntry;

//	  pPacketCopyForPool = NULL;
	  bSavedInPool = 0;

      m_Stats.Add_Stat(TWICE_GATEWAY_WAN_WCACHE_CLONE_PKT, 1);


	  // Lock the pool
	  pthread_mutex_lock(&m_MutexPacketPool);
	  pPoolEntry = m_pPacketPool->createCloneInPool(pPacket);
	  pthread_mutex_unlock(&m_MutexPacketPool);

	  cout << "     Packet is long enough - clone is prepped" << endl;


	  // Clone the packet
	  // pPacketCopyForPool = pPacket->cloneFromPool();

	  // OLD CODE
//	  cout << "Attempting wireless packet cache" << endl;
//	  pCacheResult = tryWirelessPacketCache(pPacket,nResult,nPayloadOffset, NULL, bSavedInPool);

	  cout <<  "   Trying to wireless packet cache things" << endl;

	  // New version
	  pCacheResult = tryWirelessPacketCache(pPacket,nResult,nPayloadOffset, pPoolEntry->getPacket(), bSavedInPool, pPoolEntry);

	  cout << "      Finished with attempt at caching (first round)" << endl;

//	  cout << " -> Done with the attempt" << endl;

	  switch(nResult)
	  {
	  	  case TWICE_WCACHE_RESULT_NOCHANGE:
	  		  cout << "    No change" << endl;
	  		  return 0;

	  	  case TWICE_WCACHE_RESULT_HIT:
	  		  cout << "    DING!" << endl;


	  		  m_Stats.Add_Stat(TWICE_GATEWAY_WAN_PKTS_MATCH, 1);


	  		  switch(getIPv4Protocol(pPacket->getData()))
	  		  {
	  		  	  case IP_PROTOCOL_TCP:
	  		  		m_Stats.Add_Stat(TWICE_GATEWAY_WAN_PKTS_TCP_MATCH, 1);
	  	  		  	break;

	  	  		  case IP_PROTOCOL_UDP:
	  	  			  m_Stats.Add_Stat(TWICE_GATEWAY_WAN_PKTS_UDP_MATCH, 1);
	  	  			break;
	  		  }

	  		  // pCacheResult is the same packet, only tokenized with the good stuff
	  		  break;

	  	  default:
	  		  cerr << "Ruh roh: Unexpected result from trying to attempt to cache the packet via TWiCE" << endl;
	  		  cerr << "  Error code was " << nResult << endl;
	  		  break;
	  }
  }
  else
  {
	  m_Stats.Add_Stat(TWICE_GATEWAY_WAN_WCACHE_TOO_SMALL_PKTS, 1);
	  m_Stats.Add_Stat(TWICE_GATEWAY_WAN_WCACHE_TOO_SMALL_VOL, pPacket->getLength());
  }

//  cout << "    Finished on packet analysis - exiting function" << endl;

  // Nope, did not eat it, carry on wayward son
  return 0;
}

int	TWiCE_Gateway::getMinimumPacketSizeToTryCache ()
{
	return m_nMinPktSizeTryCache;
}

void TWiCE_Gateway::setMinimumPacketSizeToTryCache (int nMinSize)
{
	if(nMinSize > 0)
	{
		m_nMinPktSizeTryCache = nMinSize;
	}
}

// The no pool variant to try
Packet * TWiCE_Gateway::doWirelessCache (Packet * pInPacket, int & nResult, int nPayloadOffset)
{
	int		nResultLocation;
	TWiCECacheEntry * pCacheEntry;

	nResult = TWICE_WCACHE_RESULT_NOCHANGE;
	nResultLocation = -1;

	cout << "** Attempting to run a packet through the cache (alt version)" << endl;
	printf("   Pkt: %p  nResult: %d  Offset: %d  Base Pkt: %p  Saved: %d\n", pInPacket, nResult, nPayloadOffset, pBasePacket, bSavedInPool);

	cout << "   --> Attempting to find a hit" << endl;
	pCacheEntry = m_pCacheTable->attemptFindHit(pBasePacket, nPayloadOffset, &nResultLocation, 0, 0, m_pPacketPool, bSavedInPool, pPoolEntry);
	cout << "    -> Result is back" << endl;

}



Packet * TWiCE_Gateway::tryWirelessPacketCache (Packet * pInPacket, int & nResult, int nPayloadOffset, Packet * pBasePacket, char & bSavedInPool,
												TWiCEPacketPoolEntry * pPoolEntry)
{
	int		nResultLocation;
	TWiCECacheEntry * pCacheEntry;
	bool	bRecursive;

	nResult = TWICE_WCACHE_RESULT_NOCHANGE;
	nResultLocation = -1;

	// OK, it is now time to officially get it on :)

	cout << "** Attempting to run a packet through the cache" << endl;
	printf("   Pkt: %p  nResult: %d  Offset: %d  Base Pkt: %p  Saved: %d\n", pInPacket, nResult, nPayloadOffset, pBasePacket, bSavedInPool);

	// We will make a copy of the packet in case it is cached.  Eventually, we need to make
	//  this copying dependent on if we actually need to cache it rather than blindly
	//  making a copy just in case

/*	if(pBasePacket == NULL)
	{
		m_Stats.Add_Stat(TWICE_GATEWAY_WAN_WCACHE_CLONE_PKT, 1);
		pBasePacket = pInPacket->cloneFromPool();
		bSavedInPool = 0;
		bRecursive = false;
//		cout << "  Allocating a new packet" << endl;
	}
	else
	{
		// Note that we are being called in a recursive manner. Freeing the base packet back to the overall pool
		//  is not our responsibility, it is the original caller of the recursive functions
//		cout << "    Recursive call - no cloning" << endl;
		bRecursive = true;
		m_Stats.Add_Stat(TWICE_GATEWAY_WAN_WCACHE_RECURSE, 1);
	}*/

	// Will use this statistic to measure how many times we try to cache the packet
	m_Stats.Add_Stat(TWICE_GATEWAY_WAN_WCACHE_RECURSE, 1);

	// attemptFindHit
	//		pBasePacket			The base packet
	//		nOffset				Offset into the packet (where to start the window search)
	//		nResultLocation		Where the actual match will be stored (location if found)
	//	    nMaxSearch			How big to search for
	//   	nCacheModule		Which cache approach to use
	//		m_pPacketPool		Pointer to the packet pool
	//		bSavedInPool		Was it saved (answer is always yes)
	//		pPoolEntry			The pool entry tagged for this base packet


	cout << "   --> Attempting to find a hit" << endl;
	pCacheEntry = m_pCacheTable->attemptFindHit(pBasePacket, nPayloadOffset, &nResultLocation, 0, 0, m_pPacketPool, bSavedInPool, pPoolEntry);
	cout << "    -> Result is back" << endl;

	if(pCacheEntry == NULL)
	{
//		cout << "No luck whatsoever, fail, fail, fail" << endl;

/*		// Fail, no hit
		if(bRecursive == false)
		{
			m_Stats.Add_Stat(TWICE_GATEWAY_WAN_WCACHE_NOHIT, 1);
			if(!bSavedInPool)
			{
				cout << " Did not make it in the pool, freeing it " << endl;
				bSavedInPool = 1;
				pBasePacket->release();
				return NULL;
			}
			else
			{
				return NULL;
			}
		}
		else
		{
			m_Stats.Add_Stat(TWICE_GATEWAY_WAN_WCACHE_NOHIT_RECURSE, 1);
			return NULL;
		}*/

		m_Stats.Add_Stat(TWICE_GATEWAY_WAN_WCACHE_NOHIT, 1);
		return NULL;
	}
	else
	{
		// We have a winner

		// OK, we have a winner, let us tag it in the list and our stats that we found a hit

		// How far does the hit go?  At this point, we have plausibly verified that we had an initial signature
		//  hit and that yes indeed, the signature mapped to an honest to goodness exact match.  Thus, we know that
		//  at least the window size is an exact match but we do not know how far to the right that the hit
		//  extends

//		cout << "  Checking for the right side ending of the hit out of " << pInPacket->getLength() - nPayloadOffset - m_pCacheTable->getWindowSize() << " possible bytes remaining" <<  endl;

		int	nMatchWidth;

//		printf("  Looking for a match that starts at %d\n", nResultLocation);

		nMatchWidth = m_pPacketPool->findRightBound(pInPacket, pCacheEntry, nResultLocation, m_pCacheTable->getWindowSize());

//		printf("    Resulting width is %d that began at %d\n", nMatchWidth, nResultLocation);


		//printf("     Offset of %d vs. prior hit in %d\n", nOffset+nDistanceOffset, m_ppEntries[nTableEntry]->getOffset());


//		cout << "    Contiguous match found that has a total width of " << nMatchWidth << " bytes. " << endl;

		m_Stats.Add_Stat(TWICE_GATEWAY_WAN_WCACHE_HITS, 1);
		m_Stats.Add_Stat(TWICE_GATEWAY_WAN_WCACHE_HIT_LENGTH, nMatchWidth);

		if(nMatchWidth <= 96)
		{
			m_Stats.Add_Stat(TWICE_GATEWAY_WAN_MATCH_COUNT_LTE96, 1);
		}
		else if(nMatchWidth <= 128)
		{
			m_Stats.Add_Stat(TWICE_GATEWAY_WAN_MATCH_COUNT_LTE128, 1);
		}
		else if(nMatchWidth <= 256)
		{
			m_Stats.Add_Stat(TWICE_GATEWAY_WAN_MATCH_COUNT_LTE256, 1);
		}
		else if(nMatchWidth <= 512)
		{
			m_Stats.Add_Stat(TWICE_GATEWAY_WAN_MATCH_COUNT_LTE512, 1);
		}
		else if(nMatchWidth <= 1024)
		{
			m_Stats.Add_Stat(TWICE_GATEWAY_WAN_MATCH_COUNT_LTE1024, 1);
		}
		else
		{
			m_Stats.Add_Stat(TWICE_GATEWAY_WAN_MATCH_COUNT_GT1024, 1);

		}

		// Tokenize this bad boy appropriately to savor the savings


		// Is there enough remaining to keep searching?

		int		nPotential;

		nPotential = pInPacket->getLength() - nResultLocation - nMatchWidth;

		nResult = TWICE_WCACHE_RESULT_HIT;

		if(nPotential >= getMinimumPacketSizeToTryCache())
		{
			int		nNestedResult;
 /*
			cout << "** ** NESTING Wireless Cache Call ** **" << endl;
			cout << "  Match width was " << nMatchWidth << " bytes" << endl;
			cout << "   Packet at this time is ";
			if(bSavedInPool)
			{
				printf("saved in the pool\n");
			}
			else
			{
				printf("NOT saved in the pool yet\n");
			}
*/
//			printf("   Prior match was from %d to %d\n", nResultLocation, nResultLocation+nMatchWidth);

			// Call it, result does not matter
			tryWirelessPacketCache(pInPacket, nNestedResult, nResultLocation+ nMatchWidth, pBasePacket, bSavedInPool, pPoolEntry);
			m_Stats.Add_Stat(TWICE_GATEWAY_WAN_WCACHE_HIT_MULTI, 1);
			m_Stats.Add_Stat(TWICE_GATEWAY_WAN_WCACHE_HIT_MULTI_POTENTIAL, nPotential);
			nResult = TWICE_WCACHE_RESULT_HIT;


			// Freedom of memory is entirely separated from the recursion since we kind of failed at it with
			//  our first version of the code

			// If we are the base caller (root of the recursion call), cleanup may be our responsibility
			/*
			if(!bRecursive && !bSavedInPool)
			{
				// Let the packet run free like nature intended
				pBasePacket->release();
				pBasePacket = NULL;
				//cout << "** Exiting the wireless cache try (cleanup / nested)" << endl;
				return NULL;
			}*/

			return pInPacket;
		}
		else
		{
			/*
			if(bRecursive)
			{
				return NULL;
			}

			if(!bSavedInPool)
			{
				// Let the packet run free like nature intended
				pBasePacket->release();
				pBasePacket = NULL;
			}
			*/

			// We are done, just give it up
			nResult = TWICE_WCACHE_RESULT_HIT;
			return pInPacket;
		}
	}

	/*
	if(!bRecursive)
	{
		cout << "Error: What is going on?" << endl;
	}

	if(!bSavedInPool && pBasePacket != NULL)
	{
		cout << "Picking up the slack???" << endl;
		pBasePacket->release();
		pBasePacket = NULL;
	}*/

	//cout << "** Committed new data to the cache, no release of the packet" << endl;
	return NULL;
}

