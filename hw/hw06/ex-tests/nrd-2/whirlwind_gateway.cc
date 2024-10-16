/*
 * whirlwind_gateway.cc
 *
 * Created on: Oct 30, 2013
 * Author: striegel, xhu2
 */

#include <iostream>
#include <fstream>
using namespace std;

#include "whirlwind_gateway.h"

#include "../mon/Thread_Archive.h"
#include "../mon/Thread_Timer.h"

#include "../xml/NodeDOM.h"
#include "../xml/NodeElem.h"

#include "../util/802_11.h"
#include "../util/ip-utils.h"
#include "../util/tokenize.h"
#include "../util/mgmt.h"

#include "../core/Timestamp.h"

// Tent, 2014/08/30
//const string SRC_DOWN = "82:e6:50:b1:29:64";
//const string DST_DOWN = "18:64:72:c6:30:66";

// Tent, 2014/09/06
//const string SRC_DOWN = "c4:04:15:44:ec:c3";
//const string DST_DOWN = "18:64:72:c6:30:66"; 
 
// Tent, 2014/10/04, 11
//const string SRC_DOWN = "7c:95:f3:19:1e:ff";
//const string DST_DOWN = "00:0b:86:6e:d6:20";

// Tent, 2014/11/15
//const string SRC_DOWN = "00:0b:86:6e:d6:20";
//const string DST_DOWN = "ANY";

// Tent, 2014/11/22
//const string SRC_DOWN = "00:0b:86:6e:d6:20";
//const string DST_DOWN = "ANY";

// Tent, 2015/11/14
const string SRC_DOWN = "7c:95:f3:19:1e:ff";
const string DST_DOWN = "00:0b:86:9b:5d:d7";

// Train, 2014
//const string SRC_DOWN = "a4:5d:36:1a:a3:ff";
//const string DST_DOWN = "70:f1:a1:08:57:fe";

// Train, 2015
//const string SRC_DOWN = "2c:41:38:0a:3d:40";
//const string DST_DOWN = "70:f1:a1:08:57:fe";

// School, 2015/11/17
//const string SRC_DOWN = "";
//const string DST_DOWN = "";

Whirlwind_GatewayStats::Whirlwind_GatewayStats () : Stats () {
    Allocate(WW_GATEWAY_STAT_LAST);
}

void Whirlwind_GatewayStats::Get_Title (int nStat, char* szTitle) {
    switch (nStat) {
        case WW_GATEWAY_WAN_PKTS_ALL:
	        sprintf(szTitle, "In_Pkts_INet");
            break;
	    case WW_GATEWAY_WAN_VOL_ALL:
            sprintf(szTitle, "In_Vol_INet");
            break;
    	case WW_GATEWAY_WAN_PKTS_TCP:
            sprintf(szTitle, "In_Pkts_TCP");
            break;
        case WW_GATEWAY_WAN_VOL_TCP:
	        sprintf(szTitle, "In_Vol_TCP");
            break;
    	case WW_GATEWAY_WAN_PKTS_UDP:
	        sprintf(szTitle, "In_Pkts_UDP");
	        break;
	    case WW_GATEWAY_WAN_VOL_UDP:
	        sprintf(szTitle, "In_Vol_UDP");
	        break;
	    case WW_GATEWAY_WAN_PKTS_IPV4:
	        sprintf(szTitle, "In_Pkts_IPV4");
	        break;
	    case WW_GATEWAY_WAN_VOL_IPV4:
	        sprintf(szTitle, "In_Vol_IPV4");
	        break;
	    case WW_GATEWAY_WAN_PKTS_IPV6:
	        sprintf(szTitle, "In_Pkts_IPV6");
	        break;
    	case WW_GATEWAY_WAN_VOL_IPV6:
	        sprintf(szTitle, "In_Vol_IPV6");
	        break;
    	case WW_GATEWAY_WAN_PKTS_OTHER:
	        sprintf(szTitle, "In_Pkts_Other");
            break;
    	case WW_GATEWAY_WAN_VOL_OTHER:
	        sprintf(szTitle, "In_Vol_Other");
	        break;
        case WW_GATEWAY_WAN_PKTS_TOOSMALL:
	        sprintf(szTitle, "In_Pkts_TooSmall");
            break;
	    case WW_GATEWAY_WAN_VOL_TOOSMALL:
	        sprintf(szTitle, "In_Vol_TooSmall");
	        break;
    	default:
	        sprintf(szTitle, "F%d", nStat);
	        break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
Whirlwind_Gateway::Whirlwind_Gateway () {
    m_pMasterCache = new Whirlwind_MasterCache();
    m_nMinPktSizeForCache = WW_GATEWAY_DEFAULT_MIN_PKTSIZE_FOR_CACHE;
    m_sPcapPath = "../pcap/tent/";         // Relative to current directory by default
	m_sDictPath = "../hostname/tent/";     // The relative path of ip-host mapping files
	m_sCurrentTrace = "";
	m_nCurrentIndex = 0;
    m_bFlipEndian = false;
    m_lastStatsDump = 0;
    m_bAdjustTime = false;
    m_Time_CurrentFileAdjust.clearTime();
    m_Time_AccumulatedAdjust.clearTime();
    m_Time_LastRelativeTime.clearTime();

    m_IPtoHostMap.clear();
	m_Host_RE.clear();
}

Whirlwind_Gateway::~Whirlwind_Gateway () {
    delete m_pMasterCache;
    m_pMasterCache = NULL;
}

char Whirlwind_Gateway::initialize () {
    m_pMasterCache->initialize();
    enableTimers();

    return 1;
}

int Whirlwind_Gateway::mapNameToID (string sName) {
    if (sName == "WAN") {
      	return WW_TAP_WAN;
    } else if (sName == "WIRELESS") {
       	return WW_TAP_WIRELESS;
    } else {
       	return WW_TAP_NONE;
    }
}

bool Whirlwind_Gateway::extractExtendedDOM (NodeDOM* pNode) {
    string sTag;
    string sData;

    for (int i = 0; i < pNode->getNumChildren(); i++) {
    }
    return true;
}

bool Whirlwind_Gateway::processTimer (TimerEvent* pEvent) {
    switch (pEvent->getTimerID()) {
        case WW_GATEWAY_TIMER_ID_LOG_WRITE:
	        logStatsToFile();
	        pEvent->armTimer();

	        if (!addTimerEvent(pEvent)) {
	            cerr << "Error: Unable to arm the timer for the Stat Log Writing" << endl;
		        pEvent->release();
	            return false;
	        }
	        break;
    }

    return true;
}

CommandResult Whirlwind_Gateway::processExtCommand (const vector<string>& theCommands, int nOffset) {
    if (theCommands.size() <= nOffset) {
        return PROCESS_CMD_SYNTAX;
    }

    if (theCommands[nOffset] == "help") {
        cout << " Whirlwind Module Help" << endl;
        cout << "   startlog --  Enable logging" << endl;
        cout << "   readpcap --  read specified pcap file: readpcap [fileName] [offset] [distance]" << endl;
	    return PROCESS_CMD_SUCCESS;
    } else if (theCommands[nOffset] == "startlog") {
	    if (theCommands.size() == nOffset + 1) {
	        return PROCESS_CMD_SUCCESS;
	    }
    } else if (theCommands[nOffset] == "readpcap") {
        cout << "Value of command offset: " << nOffset << endl;
		string sTraceName = "";
	    if (theCommands.size() == nOffset + 2) {
	        sTraceName = theCommands[nOffset + 1];
	        processPcapFile(sTraceName);
	    } else if (theCommands.size() == nOffset + 3) {
	        sTraceName = theCommands[nOffset + 1];
	        int nStart = atoi(theCommands[nOffset + 2].c_str());
	        processPcapFile(sTraceName, nStart);
	    } else if (theCommands.size() == nOffset + 4) {
	        sTraceName = theCommands[nOffset + 1];
	        int nStart = atoi(theCommands[nOffset + 2].c_str());
	        int nDistance = atoi(theCommands[nOffset + 3].c_str());
	        processPcapFile(sTraceName, nStart, nDistance);
	    } else {
	        cout << "    Illegal # of commands in whirlwind:readpcap." << endl;
	    }

	    return PROCESS_CMD_SUCCESS;
    } else if (theCommands[nOffset] == "status") {
	    if(theCommands.size() < nOffset + 2) {
	    } else {
            if (theCommands[nOffset + 1] == "cache") {		
	        } else if (theCommands[nOffset + 1] == "perf") {

	        } else {
			}
	    }
	    return PROCESS_CMD_SUCCESS;
    }

    return PROCESS_CMD_UNKNOWN;
}

void Whirlwind_Gateway::dumpStats () {
	double megaB = 1024 * 1024;
    // 1. the aggregate results 

    double dwHitLenA = (double) m_Stats.Get_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL) / megaB;
    double totalVolA = (double) m_Stats.Get_Stat(WW_GATEWAY_WAN_VOL_IPV4) / megaB;
    double dwlnkVolA = (double) m_Stats.Get_Stat(WW_GATEWAY_WAN_VOL_DOWN) / megaB;
    double uplnkVolA = (double) m_Stats.Get_Stat(WW_GATEWAY_WAN_VOL_UP) / megaB;
	double httpVolA = (double) m_Stats.Get_Stat(WW_GATEWAY_WAN_VOL_DOWN_HTTP) / megaB;
	double httpsVolA = (double) m_Stats.Get_Stat(WW_GATEWAY_WAN_VOL_DOWN_HTTPS) / megaB; 

    double REPerctA = dwHitLenA / dwlnkVolA * 100.0;

    // 2. the step results
	updateStats_Diff();

    double dwHitLenD = (double) m_DiffStats.Get_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL) / megaB;
    double totalVolD = (double) m_DiffStats.Get_Stat(WW_GATEWAY_WAN_VOL_IPV4) / megaB;
    double dwlnkVolD = (double) m_DiffStats.Get_Stat(WW_GATEWAY_WAN_VOL_DOWN) / megaB;
    double uplnkVolD = (double) m_DiffStats.Get_Stat(WW_GATEWAY_WAN_VOL_UP) / megaB;
	double httpVolD = (double) m_DiffStats.Get_Stat(WW_GATEWAY_WAN_VOL_DOWN_HTTP) / megaB;
	double httpsVolD = (double) m_DiffStats.Get_Stat(WW_GATEWAY_WAN_VOL_DOWN_HTTPS) / megaB;

	double REPerctD = dwHitLenD / dwlnkVolD * 100.0;

    //cout << "    Total aggrgate volume (MB): " << totalVolA << endl;
	//cout << "    Uplnk aggrgate volume (MB): " << uplnkVolA << endl;
    //cout << "    Dwlnk aggrgate volume (MB): " << dwlnkVolA << endl;
	//cout << "    Dwlnk stepping volume (MB): " << dwlnkVolD << endl;
    //cout << "    Dwlnk stepping Saving (%) : " << REPerctD << endl;
	//cout << "    Dwlnk aggrgate Saving (%) : " << REPerctA << endl;

	cout << dwlnkVolD << "," << httpVolD << "," << httpsVolD << "," << REPerctD << "," << REPerctA << endl; 

	//cout << "    # of entries used    : " << m_pMasterCache->getNumEntries() << endl;
	//cout << endl;

	updateStats_Prior();
}

inline bool isUplinkTraffic (string sSrcMac, string sDstMac) {
	bool ret = false;
	if (sSrcMac == DST_DOWN && sDstMac == SRC_DOWN) {
		ret = true;
	}

	return ret;
}

inline bool isDwlinkTraffic (string sSrcMac, string sDstMac) {
	bool ret = false;

	if (sSrcMac == SRC_DOWN && sDstMac == DST_DOWN) {
		ret = true;
	}

	return ret;
}

void Whirlwind_Gateway::loadHostInfo (string sTraceName) {
	string sFile = m_sDictPath + sTraceName + ".csv"; 
	ifstream infile (sFile.c_str(), ios_base::in);

	if (!infile.is_open()) {
		cerr << "**** Error: Can't open the ip_host mapping file." << endl;
		return;
	}

	string line;
	while (getline(infile, line)) {
		vector<string> tokens;
		tokens.clear();

		Tokenize(line, tokens, ",");

		if (tokens.size() < 2) {
			continue;
		} else {
			if (tokens[0].compare(tokens[1]) == 0) 
				continue;

			m_IPtoHostMap.insert(std::make_pair(tokens[0], tokens[1]));
		}
	}
}

void Whirlwind_Gateway::dumpStats_HostRE () {
	string fName = "./HostRE-" + m_sCurrentTrace + ".csv";

	double totalRE = (double)m_Stats.Get_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL);

    ofstream oHFile;

    oHFile.open(fName.c_str());

	for (auto& x : m_Host_RE) {
		oHFile << x.first << "," << std::fixed << (x.second / totalRE) * 100 << endl;
	}

	oHFile.close();
}

void Whirlwind_Gateway::logStatsToFile () {
    // Compute the difference for the various stats
    m_DiffStats.computeDiff(&m_PriorStats, &m_Stats);
    m_PriorStats.syncAll(&m_Stats);

    // Compute the cache diff stats
    m_pMasterCache->updateStats_Diff();
    m_pMasterCache->saveStats_Prior();

    // Log the time
    struct timeval cTime;
    gettimeofday(&cTime, NULL);
    m_LogFile << cTime.tv_sec << ",";

    // Log the stats to the file

    // Overall stats: all time since we have been logging (comulative)

    // Average hit length
    if (m_Stats.Get_Stat(WW_GATEWAY_WAN_CACHE_HIT_PKTS) > 0) {
        m_LogFile << (double) m_Stats.Get_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL) / (double) m_Stats.Get_Stat(WW_GATEWAY_WAN_CACHE_HIT_PKTS) << ",";
    } else {
        m_LogFile << "0,";
    }

    // How much volume-wise have we saved
    if (m_Stats.Get_Stat(WW_GATEWAY_WAN_VOL_ALL) > 0) {
        m_LogFile << (double) m_Stats.Get_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL) / (double) m_Stats.Get_Stat(WW_GATEWAY_WAN_VOL_ALL) * 100.0 << ",";
    } else {
        m_LogFile << "0,";
    }

    /////////////////////////////////////////////////////////////////////////////////
    // Last period stats: since the last log recording

    // Average hit length
    if (m_Stats.Get_Stat(WW_GATEWAY_WAN_CACHE_HIT_PKTS) > 0) {
        m_LogFile << (double) m_DiffStats.Get_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL) / (double) m_DiffStats.Get_Stat(WW_GATEWAY_WAN_CACHE_HIT_PKTS) << ",";
    } else {
        m_LogFile << "0,";
    }

    // How much volume-wise have we saved
    if (m_Stats.Get_Stat(WW_GATEWAY_WAN_VOL_ALL) > 0) {
        m_LogFile << (double) m_DiffStats.Get_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL) / (double) m_DiffStats.Get_Stat(WW_GATEWAY_WAN_VOL_ALL) * 100.0 << ",";
    } else {
        m_LogFile << "0,";
    }

    // Actual content so far
    m_DiffStats.logCSV(m_LogFile);

    m_LogFile << "AVG,";

    // Time-based average of everything as well
    m_DiffStats.logCSV_TimeAvg(m_LogFile, 60);

    m_LogFile << "CACHE,";

    Whirlwind_CacheTableStats* pCacheStats;

    pCacheStats = m_pMasterCache->getStats_Diff();
    pCacheStats->logCSV(m_LogFile);

    m_LogFile << "AVGCACHE,";
    pCacheStats->logCSV_TimeAvg(m_LogFile, 60);

    m_LogFile << endl;
    m_LogFile.flush();

}

void Whirlwind_Gateway::updateStats_Prior() {
    m_PriorStats.syncAll(&m_Stats);
}

void Whirlwind_Gateway::updateStats_Diff() {
    m_DiffStats.computeDiff(&m_PriorStats, &m_Stats);
}

char Whirlwind_Gateway::processTapPacket (Packet* pPacket, int nID) {
    switch (nID) {
        case WW_TAP_WAN:
	        return processPacket_WAN(pPacket);
	        break;
    	case WW_TAP_WIRELESS:
	        return processPacket_Wireless(pPacket);
	        break;
    }

    // Default case is for the packet to continue (not processed)
    return PROCESSPKT_RESULT_CONTINUE;
}

char Whirlwind_Gateway::processPacket_WAN (Packet* pPacket) {
    // Process the packet that came in on the WAN side

    m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_ALL, 1);
    m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_ALL, pPacket->getLength());

    if (!isPacketIP(pPacket->getData(), pPacket->getLength())) {
		m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_OTHER, 1);
		m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_OTHER, pPacket->getLength());
		return 0;
    }

    char byIPVersion;
    int  nPayloadOffset;

    byIPVersion = getIPVersion(pPacket->getData(), pPacket->getLength());
    nPayloadOffset = 0;

    switch(byIPVersion) {
		case 6:
			// IPv6 - Not supported for now
			m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_IPV6, 1);
			m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_IPV6, pPacket->getLength());
			return PROCESSPKT_RESULT_CONTINUE;

		case 4:
			// IPv4 - Definitely supported :)
			m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_IPV4, 1);
			m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_IPV4, pPacket->getLength());
			
			// Is this IPsec?
			// We do not touch IPsec as it is pretty much pointless
			if (isPacketIPv4IPsec(pPacket->getData(), pPacket->getLength())) {
				m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_ENCV4, 1);
				m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_ENCV4, pPacket->getLength());
				return PROCESSPKT_RESULT_CONTINUE;
			}

			// Fill in the payload offset
			switch (getIPv4Protocol(pPacket->getData())) {
				case IP_PROTOCOL_TCP:
					m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_TCP, 1);
					m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_TCP, pPacket->getLength());
					nPayloadOffset = getTCP_HeaderSize(pPacket->getData(), pPacket->getLength()) + getIP_HeaderSize(pPacket->getData(),
									                   pPacket->getLength()) + L2_OFFSET;
					break;

				case IP_PROTOCOL_UDP:
					m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_UDP, 1);
					m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_UDP, pPacket->getLength());
					nPayloadOffset = OFFSET_UDP_DATA + getIP_HeaderSize(pPacket->getData(), pPacket->getLength()) + L2_OFFSET;
					//break;
					return PROCESSPKT_RESULT_CONTINUE;

				// GRE (Generic Routing Encapsulation) is used by several wireless APs to
				// support IP tunneling back to the controller for appropriate management
				case IP_PROTOCOL_GRE:
					m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_GRE, 1);
					m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_GRE, pPacket->getLength());

					// What bits are set?
					uint16_t	nOffset_GRE;
					bool        bFlag_GRE_C;
					bool        bFlag_GRE_K;
					bool 	    bFlag_GRE_S;
					int         nPktLen;
					char*       pData;

					nOffset_GRE = L2_OFFSET + getIP_HeaderSize(pPacket->getData(), pPacket->getLength());
					nOffset_GRE += 2;
					// Technically, we are sort of hacking it up here.  It really does not make that big of a deal
					// unless the window gets ultra small to parse the encapsulated packet from the get go.  The
					// Aruba APs we have used in the past do a bit of trickery anyway with protocol type fields of
					// 8300, 8200, etc.
					pData = pPacket->getData();
					nPktLen = pPacket->getLength();

                    if (pData[nOffset_GRE] == GRE_ENCAP_WLAN_B1 && pData[nOffset_GRE + 1] == GRE_ENCAP_WLAN_B2) {
                        // The packet that's been encapsulated is an 802.11, need to skip the GRE header and pass to ProcessPacket_Wireless
						char sTemp[2000];
						nOffset_GRE += 2;
						memcpy(sTemp, pData + nOffset_GRE, nPktLen - nOffset_GRE);
						pPacket->setData(nPktLen - nOffset_GRE, sTemp);
						return processPacket_Wireless(pPacket);
					} else {
						return PROCESSPKT_RESULT_CONTINUE;
					}

					break;

				default:
					m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_OTHER, 1);
					m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_OTHER, pPacket->getLength());
					return PROCESSPKT_RESULT_CONTINUE;
			}
			break;

    	default:
			m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_OTHER, 1);
			m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_OTHER, pPacket->getLength());
    		break;
    }

    // Compute the source and destination mac addresses to determin down / up link traffic
    string sMacSrc = getEtherMac_Src(pPacket->getData());
	string sMacDst = getEtherMac_Dst(pPacket->getData());

    if (isDwlinkTraffic(sMacSrc, sMacDst)) {
        m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_DOWN, 1);
        m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_DOWN, pPacket->getLength());

		if (getTCP_SrcPort(pPacket->getData()) == 80 || getTCP_DstPort(pPacket->getData()) == 80) {
			m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_DOWN_HTTP, pPacket->getLength());
		} else if (getTCP_SrcPort(pPacket->getData()) == 443 || getTCP_DstPort(pPacket->getData()) == 443) {
			m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_DOWN_HTTPS, pPacket->getLength());
		} else {
			m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_DOWN_OTHER, pPacket->getLength());
		}

		// Evaluate the master cache using this incomming downlink packet
		if (pPacket->getLength() >= getMinPktSizeForCache()) {
		    evalMasterCache(pPacket, nPayloadOffset);
		} else {
			m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_TOOSMALL, 1);
			m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_TOOSMALL, pPacket->getLength());
		}
    } 

    if (isUplinkTraffic(sMacSrc, sMacDst)) {
        m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_UP, 1);
        m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_UP, pPacket->getLength());
    }

    // Do not eat the packet, carry on 
    return PROCESSPKT_RESULT_CONTINUE;
}

char Whirlwind_Gateway::processPacket_Wireless (Packet* pPacket) {
    m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_ALL, 1);
    m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_ALL, pPacket->getLength());

    if (!isPacketIP_80211(pPacket->getData())) {
        m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_OTHER, 1);
        m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_OTHER, pPacket->getLength());
        return 0;
    }

    char byIPVersion;
    int nPayloadOffset;

	byIPVersion = getIPVersion_80211(pPacket->getData());
	nPayloadOffset = 0;

	switch(byIPVersion) {
	    case 6:
	        // IPv6 - Not supported for now 
		    m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_IPV6, 1);
            m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_IPV6, pPacket->getLength());
            return PROCESSPKT_RESULT_CONTINUE;
	    case 4:
		    // IPv4 - Definitely supported :)
            m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_IPV4, 1);
            m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_IPV4, pPacket->getLength());

            // Is this IPsec?
            // We do not touch IPsec as it is pretty much pointless
		    if (isPacketIPSec_80211(pPacket->getData())) {
                m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_ENCV4, 1);
                m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_ENCV4, pPacket->getLength());
                return PROCESSPKT_RESULT_CONTINUE;
            }

			switch (getIPv4Protocol_80211(pPacket->getData())) {
			    case IP_PROTOCOL_TCP:
					m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_TCP, 1);
					m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_TCP, pPacket->getLength());
					nPayloadOffset = getTCP_HeaderSize_80211(pPacket->getData()) + getIP_HeaderSize_80211(pPacket->getData()) + 
                                     LENGTH_80211 + LENGTH_LLC;
					break;

			    case IP_PROTOCOL_UDP:
					m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_UDP, 1);
					m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_UDP, pPacket->getLength());
					nPayloadOffset = OFFSET_UDP_DATA + getIP_HeaderSize_80211(pPacket->getData()) + LENGTH_80211 + LENGTH_LLC; 
					return PROCESSPKT_RESULT_CONTINUE;

			    default:
				    m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_OTHER, 1);
				    m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_OTHER, pPacket->getLength());
				    return PROCESSPKT_RESULT_CONTINUE;
			}
			break;

	    default:
		    m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_OTHER, 1);
		    m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_OTHER, pPacket->getLength());
		    break;
	}

    // Compute the source and destination mac addresses to determin down / up link traffic
    string sMacDst = getDstMac_80211(pPacket->getData());
    string sMacSrc = getSrcMac_80211(pPacket->getData());

    if (isDwlinkTraffic(sMacSrc, sMacDst)) {
        m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_DOWN, 1);
        m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_DOWN, pPacket->getLength());

		if (getTCP_SrcPort_80211(pPacket->getData()) == 80 || getTCP_DstPort_80211(pPacket->getData()) == 80) {
			m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_DOWN_HTTP, pPacket->getLength());
		} else if (getTCP_SrcPort_80211(pPacket->getData()) == 443 || getTCP_DstPort_80211(pPacket->getData()) == 443) {
			m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_DOWN_HTTPS, pPacket->getLength());
		} else {
			m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_DOWN_OTHER, pPacket->getLength());
		}

		// Evaluate the master cache using this incomming downlink packet
		if (pPacket->getLength() >= getMinPktSizeForCache()) {
			evalMasterCache_80211(pPacket, nPayloadOffset);
		} else {
			m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_TOOSMALL, 1);
			m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_TOOSMALL, pPacket->getLength());
		}
    } 

    if (isUplinkTraffic(sMacSrc, sMacDst)) {
		m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_UP, 1);
        m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_UP, pPacket->getLength());
    }

    return PROCESSPKT_RESULT_CONTINUE;
}

int Whirlwind_Gateway::evalMasterCache (Packet* pInPacket, int nPayloadOffset) {
	int nHits = 0;
	int nHitLen = 0;

	m_pMasterCache->evaluateCache(pInPacket, nPayloadOffset, &nHits, &nHitLen, 0, 0);

	if (nHitLen > 0) {

	    m_Stats.Add_Stat(WW_GATEWAY_WAN_CACHE_HIT_PKTS, 1);
	    m_Stats.Add_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL, nHitLen);

        if (getTCP_SrcPort(pInPacket->getData()) == 80 || getTCP_DstPort(pInPacket->getData()) == 80) {
            m_Stats.Add_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL_HTTP, nHitLen);
        } else if (getTCP_SrcPort(pInPacket->getData()) == 443 || getTCP_DstPort(pInPacket->getData()) == 443) {
            m_Stats.Add_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL_HTTPS, nHitLen);
        } else {
            m_Stats.Add_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL_OTHER, nHitLen);
        }

        string srcIP = IPv4toString(getIPv4_Address_Src(pInPacket->getData()));
		string dstIP = IPv4toString(getIPv4_Address_Dst(pInPacket->getData()));

		string srcHost;

        if (m_IPtoHostMap.find(srcIP) != m_IPtoHostMap.end()) {
			srcHost= m_IPtoHostMap[srcIP];
		} else {
			srcHost = "Unknown";
		}

		if (m_IPtoHostMap.find(dstIP) != m_IPtoHostMap.end()) {
			srcHost= m_IPtoHostMap[dstIP];
		}
		
		if (m_Host_RE.find(srcHost) != m_Host_RE.end()) {
			m_Host_RE[srcHost] += (double) nHitLen;
		} else {
			m_Host_RE.insert(std::make_pair(srcHost, (double) nHitLen));
		}
    }

	return 0;
}

int Whirlwind_Gateway::evalMasterCache_80211 (Packet* pInPacket, int nPayloadOffset) {
	int nHits = 0;
	int nHitLen = 0;

	m_pMasterCache->evaluateCache(pInPacket, nPayloadOffset, &nHits, &nHitLen, 0, 0);

	if (nHitLen > 0) {
		m_Stats.Add_Stat(WW_GATEWAY_WAN_CACHE_HIT_PKTS, 1);
		m_Stats.Add_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL, nHitLen);

        if (getTCP_SrcPort_80211(pInPacket->getData()) == 80 || getTCP_DstPort_80211(pInPacket->getData()) == 80) {
            m_Stats.Add_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL_HTTP, nHitLen);
        } else if (getTCP_SrcPort_80211(pInPacket->getData()) == 443 || getTCP_DstPort_80211(pInPacket->getData()) == 443) {
            m_Stats.Add_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL_HTTPS, nHitLen);
        } else {
            m_Stats.Add_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL_OTHER, nHitLen);
        }

        string srcIP = getIPv4_Src_80211(pInPacket->getData());
		string dstIP = getIPv4_Dst_80211(pInPacket->getData());

		string srcHost;

        if (m_IPtoHostMap.find(srcIP) != m_IPtoHostMap.end()) {
			srcHost= m_IPtoHostMap[srcIP];
		} else {
			srcHost = "Unknown";
		}

		if (m_IPtoHostMap.find(dstIP) != m_IPtoHostMap.end()) {
            srcHost= m_IPtoHostMap[dstIP];
        } else {
            srcHost = "Unknown";
        }
		
		if (m_Host_RE.find(srcHost) != m_Host_RE.end()) {
			m_Host_RE[srcHost] += (double) nHitLen;
		} else {
			m_Host_RE.insert(std::make_pair(srcHost, (double) nHitLen));
		}

    }

	return 0;
}

int Whirlwind_Gateway::getMinPktSizeForCache () {
    return m_nMinPktSizeForCache;
} 

void Whirlwind_Gateway::setMinPktSizeForCache (int nMinSize) {
    if (nMinSize > 0)
        m_nMinPktSizeForCache = nMinSize;
}

struct timeval* Whirlwind_Gateway::computeNextLogWrite (char byTimeMode) {
    // Set it from the current time in the listing
    adjustTime(&m_nextLogWrite, WW_GATEWAY_TIMER_INTERVAL_LOG_WRITE);
    return &m_nextLogWrite;
}

void Whirlwind_Gateway::setPcapPath (string sPath) {
    // TODO - Validate?
    m_sPcapPath = sPath;
}

string Whirlwind_Gateway::getPcapPath () {
    return m_sPcapPath;
}

bool Whirlwind_Gateway::processPcapFile (string sTraceName, int nOffset, int nPackets) {
	m_sCurrentTrace = sTraceName;
	m_nCurrentIndex++;

	loadHostInfo(sTraceName);

    string sFile;
    // Open the new file
    sFile = getPcapPath() + sTraceName + ".pcap";

    cout << "  Targeted File to Read: " << sFile << endl;

    m_CurrentFile.open(sFile.c_str());

    if (!m_CurrentFile.good()) {
        return false;
    }

    // tcpdump header processing
    //
    // Reference of file info available at:
    //   http://lists.linux-wlan.com/pipermail/linux-wlan-devel/2003-September/002701.html
    //
    // Also see:
    //   http://wiki.wireshark.org/Development/LibpcapFileFormat

    int                     nMagicNum;
    unsigned short          nMajor;
    unsigned short          nMinor;
    unsigned int            nSnapshotLen;
    unsigned int            nMediumType;

    // 32 bit magic number
    // 16 bit Major
    // 16 bit Minor
    // Timezone offset (ignore) - 32 bit
    // Timezone accuracy (ignore) - 32 bit
    // Snapshot length - 32 bit
    // Link layer type - 32 bit

    m_CurrentFile.read((char *) &nMagicNum, 4);
    m_CurrentFile.read((char *) &nMajor, sizeof(unsigned short));
    m_CurrentFile.read((char *) &nMinor, sizeof(unsigned short));

    if (nMagicNum == 0xa1b2c3d4) {
	    m_bFlipEndian = false;
    } else if (nMagicNum == 0xd4c3b2a1) {
	    m_bFlipEndian = true;
    } else {
	    cout << "Warning: Non-standard magic number at beginning of TCP dump file" << endl;
	    // Major version should be roughly 4, if it is not, probably endian-ness is different
        //  than this particular architecture
	    if (nMajor > 255) {
	        cout << "  Guessing endian-ness of file is requiring a flip" << endl;
	        m_bFlipEndian = true;
	    } else {
	        cout << "  Guessing endian-ness of file is OK" << endl;
	        m_bFlipEndian = false;
	    }
    }

    // Ignore time zone and TZ accuracy
    m_CurrentFile.read((char*)&nMagicNum, 4);
    m_CurrentFile.read((char*)&nMagicNum, 4);

    // The Medium Type determines how we should process the packet,
    // e.g., 802.11 (WiFi) or 802.3 (Ethernet)
    m_CurrentFile.read((char*)&nSnapshotLen, 4);
    m_CurrentFile.read((char*)&nMediumType, 4);

    if (m_bFlipEndian) {
	    nSnapshotLen = endianfixl(nSnapshotLen);
	    nMediumType = endianfixl(nMediumType);
    }

    // Just some debug info
	if (true) {
	    cout << "Tcpdump file debug information via whirlwind_gateway" << endl;
	    cout << "    Maj / Min Version:   " << nMajor << " / " << nMinor << endl;
	    cout << "    Endian Flip:         ";

	    if (m_bFlipEndian) 
	        cout << "Yes" << endl;
        else 
	        cout << "No" << endl;

	    cout << "    Snapshot Len:        " << nSnapshotLen << endl;
	    cout << "    Medium Type:         " << nMediumType << endl;
    }

    timeval firstArrivalTime;
    memset(&firstArrivalTime, 0, sizeof(timeval));

    // Read in the arrival time of the first packet
    m_CurrentFile.read((char*) &(firstArrivalTime.tv_sec), 4);
    m_CurrentFile.read((char*) &(firstArrivalTime.tv_usec), 4);

    // Endian for the timestamp?
    if(m_bFlipEndian) {
        firstArrivalTime.tv_sec = endianfixl(firstArrivalTime.tv_sec);
        firstArrivalTime.tv_usec = endianfixl(firstArrivalTime.tv_usec);
    }

    Timestamp fileTime;
    fileTime.setTime(firstArrivalTime);

	// File time adjust would always be the time of the first package
    m_Time_CurrentFileAdjust = fileTime;

    cout << "    Current file adjust time in sec: " << firstArrivalTime.tv_sec << endl; 
    // Pretend like we never read in the time
    m_CurrentFile.seekg(-8, ios_base::cur);

    bool skipPacket = true;
    int  processedPkts = 0;
    int  ignoredPkts = 0;

    m_nCurPktIndex = 0;

    // Loop through the end of the file
    while (!m_CurrentFile.eof() && (processedPkts < nPackets) && m_CurrentFile.good()) {
	    if (ignoredPkts >= (nOffset - 1)) {
	        skipPacket = false;
	    }

	    doNextPcapPacket(skipPacket, nMediumType);

	    if (skipPacket)
	        ignoredPkts++;
	    else 
	        processedPkts++;
    }
	//dumpStats();

    // Print out important summaries
	cout << endl;
	double megaB = 1024.0 * 1024.0;

    cout << "++++++++++ IP pkts processed in total: " << m_Stats.Get_Stat(WW_GATEWAY_WAN_PKTS_IPV4) << endl;
    cout << "++++++++++ IP pkts processed downlink: " << m_Stats.Get_Stat(WW_GATEWAY_WAN_PKTS_DOWN) << endl;

    m_Time_AccumulatedAdjust = m_Time_LastRelativeTime;
    cout << "========== Accumulated time adjust   : " << m_Time_AccumulatedAdjust.getSeconds() << endl;

	cout << "********** Downlnk Vol - Total: " << (double)m_Stats.Get_Stat(WW_GATEWAY_WAN_VOL_DOWN) / megaB << endl;
	cout << "********** Downlnk Vol - HTTP : " << (double)m_Stats.Get_Stat(WW_GATEWAY_WAN_VOL_DOWN_HTTP) / megaB << endl;
    cout << "********** Downlnk Vol - HTTPS: " << (double)m_Stats.Get_Stat(WW_GATEWAY_WAN_VOL_DOWN_HTTPS) / megaB << endl;
	cout << "********** Downlnk Vol - Other: " << (double)m_Stats.Get_Stat(WW_GATEWAY_WAN_VOL_DOWN_OTHER) /megaB << endl;
	cout << endl;

	cout << "********** Hit length  - Total: " << (double)m_Stats.Get_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL) / megaB << endl;
    cout << "********** Hit Length  - HTTP : " << (double)m_Stats.Get_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL_HTTP) / megaB << endl;
    cout << "********** Hit Length  - HTTPS: " << (double)m_Stats.Get_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL_HTTPS) / megaB << endl;
    cout << "********** Hit Length  - Other: " << (double)m_Stats.Get_Stat(WW_GATEWAY_WAN_CACHE_HIT_VOL_OTHER) / megaB << endl;
	cout << endl;

	cout << "!!!!!!!!!! Volume of big pkts : " << (double)m_Stats.Get_Stat(WW_GATEWAY_WAN_VOL_TOOBIG) / megaB << endl;

    m_CurrentFile.close();
    m_CurrentFile.clear();

	dumpStats_HostRE();
    m_pMasterCache->dumpCacheStats(m_sCurrentTrace);

    return true;
}

void Whirlwind_Gateway::doNextPcapPacket (bool skipFlag, unsigned int nMediumType) {
    if (m_CurrentFile.good()) {
	    // Read the packet content from the file
		// time_t struct       Seconds, microseconds (each 32 bits)
		// Capture Length      32 bits
		// Actual Length       32 bits
		timeval         arrivalTime;
		unsigned int    nCapLen;
		unsigned int    nActLen;

		memset(&arrivalTime, 0, sizeof(timeval));

		m_CurrentFile.read((char *) &(arrivalTime.tv_sec), 4);
		m_CurrentFile.read((char *) &(arrivalTime.tv_usec), 4);
		m_CurrentFile.read((char *) &nCapLen, 4);
		m_CurrentFile.read((char *) &nActLen, 4);

		if (m_bFlipEndian) {
			arrivalTime.tv_sec = endianfixl(arrivalTime.tv_sec);
			arrivalTime.tv_usec = endianfixl(arrivalTime.tv_usec);
			nCapLen = endianfixl(nCapLen);
			nActLen = endianfixl(nActLen);
		}

		// Bad timestamp, bail out
		if (timercmp(&arrivalTime, m_Time_CurrentFileAdjust.getCoreTime(), <)) {
			m_CurrentFile.seekg(nCapLen, ios_base::cur);
            return;
        }

		Timestamp adjustTS;

		// Subtract the current time
        adjustTS.setTime(arrivalTime);
		adjustTS = adjustTS - m_Time_CurrentFileAdjust; 

		// Update based on current accumulated time
        adjustTS = adjustTS + m_Time_AccumulatedAdjust;

		m_Time_LastRelativeTime = adjustTS;

	    if (skipFlag == true || nCapLen != nActLen) {
            // Directly jump to the next package
	        m_CurrentFile.seekg(nCapLen, ios_base::cur);
			return;
	    } 

        // Get a packet object from the pool or create one
	    Packet*  pPacket;
	    pPacket = Packet::createPacket();

	    if (pPacket == NULL) {
		    cout << "Error:    Failed to create package when reading pcap file." << endl;
		    m_CurrentFile.seekg(nCapLen, ios_base::cur);
		    return;
	    }

        // If the packet size is whithin the defined limit, go as normal
		if (nCapLen <= pPacket->getMaxSize()) {
		    pPacket->setLength(nCapLen);

		    if (m_bAdjustTime) {
  		        pPacket->setArrivalTime(adjustTS.getCoreTime());
		    } else {
			    pPacket->setArrivalTime(&arrivalTime);
		    }
		      
		    m_CurrentFile.read(pPacket->getData(), nCapLen);

			int nRTLen; // Length of the possible readiotap header;
		    // Proecess the packet - cache operations
			switch (nMediumType) {
				case 1:
                    // Ethernet
					processPacket_WAN(pPacket);
					break;
				case 105:
                    // 802.11
					processPacket_Wireless(pPacket);
					break;
				case 127:
                    // radiotap + 802.11
                    nRTLen = pPacket->getData()[OFFSET_RADIOTAP_LENGTH];
					char sTmp[2000];
					memcpy(sTmp, pPacket->getData() + nRTLen, nCapLen - nRTLen);
					pPacket->setData(nCapLen - nRTLen, sTmp);
					processPacket_Wireless(pPacket);
					break;
				default:
					break;
			}

            // Check to see if dump stats
		    if (m_nCurPktIndex == 0) {
		        m_lastStatsDump = pPacket->getArrivalTime()->tv_sec;
		    }

		    long tCurrentSec = pPacket->getArrivalTime()->tv_sec;
                    
		    if ((tCurrentSec - m_lastStatsDump) >= WW_GATEWAY_STATS_DUMP_INTERVAL && 
                (tCurrentSec - m_lastStatsDump) < 1000) {
			    //cout << "Gateway stats at " << tCurrentSec << ":" << endl;
			    dumpStats();
			    m_lastStatsDump = tCurrentSec;
		    }
                  
		    m_nCurPktIndex++;

            // Release the packet after done
		    pPacket->release();
		    pPacket = NULL;

		} else {
            // Need to handle jumbo packet
			int nMaxLen = pPacket->getMaxSize();
			int nRemain = nCapLen - nMaxLen;

            if (m_bAdjustTime) {
                pPacket->setArrivalTime(adjustTS.getCoreTime());
            } else {
                pPacket->setArrivalTime(&arrivalTime);
            }

			pPacket->setLength(nMaxLen);
			m_CurrentFile.read(pPacket->getData(), nMaxLen);

			if (!isPacketIP(pPacket->getData(), pPacket->getLength())) {
                m_CurrentFile.seekg(nRemain, ios_base::cur);
				return;
			}

			char byIPVersion;
			int  nHeaderSize;

			byIPVersion = getIPVersion(pPacket->getData(), pPacket->getLength());
			nHeaderSize = 0;

			if (byIPVersion != 4) {
				cout << "Jumbo is not IPv4 " << endl;
				m_CurrentFile.seekg(nRemain, ios_base::cur);
                return;
			}

			if (isPacketIPv4IPsec(pPacket->getData(), pPacket->getLength())) {
				m_CurrentFile.seekg(nRemain, ios_base::cur);
				return;
			}

			switch (getIPv4Protocol(pPacket->getData())) {
			    case IP_PROTOCOL_TCP:
					nHeaderSize = getTCP_HeaderSize(pPacket->getData(), pPacket->getLength()) + 
                                  getIP_HeaderSize(pPacket->getData(), pPacket->getLength()) + L2_OFFSET;
					break;
			    default:
					m_CurrentFile.seekg(nRemain, ios_base::cur);
					return;
			}

            // Process the first pacekt from the jumbo
			processPacket_WAN(pPacket);

            // Check to see if dump stats                                                                                                                                      
            if (m_nCurPktIndex == 0) {
                m_lastStatsDump = pPacket->getArrivalTime()->tv_sec;
            }

            long tCurrentSec = pPacket->getArrivalTime()->tv_sec;

            if ((tCurrentSec - m_lastStatsDump) >= WW_GATEWAY_STATS_DUMP_INTERVAL &&
                (tCurrentSec - m_lastStatsDump) < 1000) {
                //cout << "Gateway stats at " << tCurrentSec << ":" << endl;
                dumpStats();
                m_lastStatsDump = tCurrentSec;
            }

            m_nCurPktIndex++;

			char sHeader[nHeaderSize+1];
			memcpy(sHeader, pPacket->getData(), nHeaderSize);

			while (nRemain >= (nMaxLen - nHeaderSize)) {
				Packet* pSubPkt;
				pSubPkt = Packet::createPacket();
				if (pSubPkt == NULL) {
					m_CurrentFile.seekg(nRemain, ios_base::cur);
					return;
				}

				if (m_bAdjustTime) {
					pSubPkt->setArrivalTime(adjustTS.getCoreTime());
				} else {
					pSubPkt->setArrivalTime(&arrivalTime);
				}

				pSubPkt->setLength(nMaxLen);

				memcpy(pSubPkt->getData(), sHeader, nHeaderSize);
				m_CurrentFile.read(pSubPkt->getData() + nHeaderSize, nMaxLen - nHeaderSize);

				nRemain -= (nMaxLen - nHeaderSize);

				processPacket_WAN(pSubPkt);

				pSubPkt->release();
				pSubPkt = NULL;
			}

			Packet* pLastPkt;
			pLastPkt = Packet::createPacket();
			if (pLastPkt == NULL) {
				m_CurrentFile.seekg(nRemain, ios_base::cur);
				return;
			}

			if (m_bAdjustTime) {
				pLastPkt->setArrivalTime(adjustTS.getCoreTime());
			} else {
				pLastPkt->setArrivalTime(&arrivalTime);
			}

			pLastPkt->setLength(nRemain + nHeaderSize);
			memcpy(pLastPkt->getData(), sHeader, nHeaderSize);
			m_CurrentFile.read(pLastPkt->getData() + nHeaderSize, nRemain);

			processPacket_WAN(pLastPkt);

			pLastPkt->release();
			pLastPkt = NULL;

			// Dismiss!
			pPacket->release();
            pPacket = NULL;
                    
            m_Stats.Add_Stat(WW_GATEWAY_WAN_PKTS_TOOBIG, 1);
			m_Stats.Add_Stat(WW_GATEWAY_WAN_VOL_TOOBIG, nCapLen);
		    // Need to still seek forward in order to read next packet correctly
		    // m_CurrentFile.seekg(nCapLen, ios_base::cur);
	    }
	 
    } else {
        // Problems with the file, hmmm, yield and bail out
        yieldCPU();
		return;
    }
} 
