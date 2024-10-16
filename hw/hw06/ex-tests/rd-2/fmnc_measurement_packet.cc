/*
 * fmnc_measurement_packet.cc
 *
 *  Created on: Jun 9, 2014
 *      Author: Aaron
 */


#include "fmnc_measurement_packet.h"

#include <iostream>
using namespace std;

#include <string>
using namespace std;

#include "../util/ip-utils.h"

FMNC_Measurement_Packet::FMNC_Measurement_Packet()
{
	m_sMetaData = "";
	m_nGroup = FMNC_MEASUREMENT_GROUP_NONE;
}

bool 	FMNC_Measurement_Packet::adjustRelativeTime (struct timeval * pTime)
{
	// Is the time structure valid?
	if(pTime == NULL)
	{
		cerr << "* Error: Adjustment time was a NULL pointer, ignoring......" << endl;
		return false;
	}

	// Check that we do not push the time too far
	if(pTime->tv_sec > m_Time.tv_sec)
	{
		return false;
	}
	else if (pTime->tv_sec == m_Time.tv_sec)
	{
		if(pTime->tv_usec > m_Time.tv_usec)
		{
			return false;
		}

		// Less than or equal is OK
	}

	// At this point, we know the adjustment time value is at worst case the exact same
	// time value that we have here or earlier with respect to time.  Hence, we cannot
	// go negative, only down to zero

	m_Time.tv_sec -= pTime->tv_sec;
	m_Time.tv_usec -= pTime->tv_usec;
	return true;
}


void FMNC_Measurement_Packet::setMetaData (string sData)
{
	m_sMetaData = sData;
}

void FMNC_Measurement_Packet::appendMetaData (string sAppend)
{
	m_sMetaData += sAppend;
}

string FMNC_Measurement_Packet::getMetaData ()
{
	return m_sMetaData;
}

void FMNC_Measurement_Packet::setTime (struct timeval * pTime)
{
	m_Time = *pTime;
}

struct timeval * FMNC_Measurement_Packet::getTime ()
{
	return &m_Time;
}

bool FMNC_Measurement_Packet::instrumentPacket (Packet * pPacket)
{
	return false;
}

bool FMNC_Measurement_Packet::instrumentPacket_Base (Packet * pPacket)
{
	m_Time = *(pPacket->getArrivalTime());
	return true;
}

uint16_t	FMNC_Measurement_Packet::getGroup ()
{
	return m_nGroup;
}

void		FMNC_Measurement_Packet::setGroup (uint16_t nGroup)
{
	m_nGroup = nGroup;
}

string 		FMNC_Measurement_Packet::getAttribute_Group()
{
	string	sXML;
	char 	szTemp[50];

	sXML = "";

	if(m_nGroup != FMNC_MEASUREMENT_GROUP_NONE)
	{
		sXML = "Group = \"";
		sprintf(szTemp, "%d", m_nGroup);
		sXML += szTemp;
		sXML += "\" ";
	}

	return sXML;
}


string 	FMNC_Measurement_Packet::getXML ()
{
	string 	sXML;

	sXML = "<Packet ";

	sXML += getAttribute_Time();
	sXML += getAttribute_MetaData();
	sXML += getAttribute_Group();

	return sXML;
}

string FMNC_Measurement_Packet::getJSON_Base ()
{
	string	 sJSON;
	char 	 szTemp[50];

	sJSON = "";

	sJSON += getJSON_Time();

	return sJSON;
}


string FMNC_Measurement_Packet::getXML_Attributes_Base ()
{
	string 	sXML;

	sXML = getAttribute_Time();
	sXML += " ";

	sXML += getAttribute_MetaData();
	sXML += " ";
	sXML += getAttribute_Group();
	sXML += " ";

	return sXML;
}

string  FMNC_Measurement_Packet::getJSON_Time ()
{
	string 	sJSON;
	char 	szTemp[50];

	sJSON = "";
	sJSON += "\"Time\":";

	sprintf(szTemp, "\"%ld.%d\"", m_Time.tv_sec, m_Time.tv_usec);
	sJSON += szTemp;

	return sJSON;
}

string	FMNC_Measurement_Packet::getAttribute_MetaData ()
{
	string	sXML;

	sXML = "";

	if(m_sMetaData.length() > 0)
	{
		sXML += "Meta=\"" + m_sMetaData + "\"";
	}

	return sXML;
}

string	FMNC_Measurement_Packet::getAttribute_Time ()
{
	string 	sXML;
	char 	szTemp[50];

	sXML = "Time=\"";

	sprintf(szTemp, "%ld.%d\"", m_Time.tv_sec, m_Time.tv_usec);

	sXML += szTemp;

	return sXML;
}

string 	FMNC_Measurement_Packet::getAttribute_Time (timeval * pBaseTime)
{
	uint32_t		lTimeDiff;
	char 	szTemp[50];
	string 	sXML;

	lTimeDiff = calcTimeDiff (pBaseTime, &m_Time);

	uint32_t		lSec;
	uint32_t		lMicroSec;

	lMicroSec = lTimeDiff % 1000000;
	lSec = lTimeDiff / 1000000;

	sXML = "Time=\"";
	sprintf(szTemp, "%ud.%ud\"", lSec, lMicroSec);
	sXML += szTemp;
	return sXML;
}

string 	FMNC_Measurement_Packet::getAttribute_Time (FMNC_Measurement_Packet * pBasePacket)
{
	return getAttribute_Time(pBasePacket->getTime());
}

string 	FMNC_Measurement_Packet::getTime_AsString ()
{
	string 	sTime;
	char 	szTemp[50];

	sTime = "";

	sprintf(szTemp, "%ld.%d\"", m_Time.tv_sec, m_Time.tv_usec);

	sTime += szTemp;

	return sTime;
}


FMNC_List_Measurement_Packets::FMNC_List_Measurement_Packets ()
{
	pthread_mutex_init(&m_MutexList, NULL);
}

FMNC_List_Measurement_Packets::~FMNC_List_Measurement_Packets ()
{
	int		j;

	pthread_mutex_lock(&m_MutexList);

	for(j=0; j<m_Packets.size(); j++)
	{
		delete m_Packets[j];
		m_Packets[j] = NULL;
	}

	m_Packets.clear();
	pthread_mutex_unlock(&m_MutexList);

}

FMNC_Measurement_Packet *	FMNC_List_Measurement_Packets::getPacket (int nIndex)
{
	FMNC_Measurement_Packet * pRetrieve;

	if(nIndex < 0 || nIndex >= getCount())
	{
		return NULL;
	}

	pthread_mutex_lock(&m_MutexList);
	pRetrieve = m_Packets[nIndex];
	pthread_mutex_unlock(&m_MutexList);

	return pRetrieve;

}

uint32_t	FMNC_List_Measurement_Packets::getCount ()
{
	uint32_t	nSize;

	pthread_mutex_lock(&m_MutexList);
	nSize = m_Packets.size();
	pthread_mutex_unlock(&m_MutexList);

	return nSize;
}

string		FMNC_List_Measurement_Packets::getCount_AsString()
{
	char szTemp[25];
	string sVal;

	sprintf(szTemp, "%lu", m_Packets.size());
	sVal = szTemp;
	return sVal;
}


string FMNC_List_Measurement_Packets::getXML(string sTag)
{
	int		j;
	string	sXML;

	sXML = "<" + sTag + " Count=\"" + getCount_AsString() + "\">";
	sXML += "\n";

	for(j=0; j<m_Packets.size(); j++)
	{
		sXML += m_Packets[j]->getXML();
		sXML += "\n";
	}

	sXML += "</" + sTag + ">";

	return sXML;
}

void  FMNC_List_Measurement_Packets::addPacket (FMNC_Measurement_Packet * pPacket)
{
	pthread_mutex_lock(&m_MutexList);

	if(pPacket != NULL)
	{
		m_Packets.push_back(pPacket);
	}
	else
	{
		cerr << "* Error: Tried to add a NULL measurement packet to the list of measurement packets" << endl;
	}

	pthread_mutex_unlock(&m_MutexList);
}


FMNC_Measurement_Packet_IPv4::FMNC_Measurement_Packet_IPv4 () : FMNC_Measurement_Packet ()
{
	m_byTTL = 0;
	m_byTOS = 0;
	m_nLength = 0;
	m_nID = 0;
}

void	FMNC_Measurement_Packet_IPv4::setTTL (uint8_t byTTL)
{
	m_byTTL = byTTL;
}

uint8_t	FMNC_Measurement_Packet_IPv4::getTTL ()
{
	return m_byTTL;
}

string 	FMNC_Measurement_Packet_IPv4::getTTL_AsString()
{
	char 	szTemp[25];
	string	sVal;

	sprintf(szTemp, "%d", m_byTTL);

	sVal = szTemp;
	return sVal;
}


void	FMNC_Measurement_Packet_IPv4::setTOS (uint8_t byTOS)
{
	m_byTOS = byTOS;
}



uint8_t	FMNC_Measurement_Packet_IPv4::getTOS ()
{
	return m_byTOS;
}

string 	FMNC_Measurement_Packet_IPv4::getTOS_AsString()
{
	char 	szTemp[25];
	string	sVal;

	sprintf(szTemp, "%d", m_byTOS);

	sVal = szTemp;
	return sVal;
}

void	FMNC_Measurement_Packet_IPv4::setLength (uint16_t nLength)
{
	m_nLength = nLength;
}

uint16_t	FMNC_Measurement_Packet_IPv4::getLength ()
{
	return m_nLength;
}

string FMNC_Measurement_Packet_IPv4::getLength_AsString ()
{
	char szTemp[25];
	string sVal;

	sprintf(szTemp, "%u", m_nLength);
	sVal = szTemp;

	return sVal;
}


void FMNC_Measurement_Packet_IPv4::setID (uint16_t nID)
{
	m_nID = nID;
}

uint16_t FMNC_Measurement_Packet_IPv4::getID ()
{
	return m_nID;
}

string FMNC_Measurement_Packet_IPv4::getID_AsString ()
{
	char szTemp[25];
	string sVal;

	sprintf(szTemp, "0x%X", m_nID);
	sVal = szTemp;

	return sVal;
}


bool FMNC_Measurement_Packet_IPv4::instrumentPacket (Packet * pPacket)
{
	return instrumentPacket_IPv4(pPacket);
}

string 	FMNC_Measurement_Packet_IPv4::getXML ()
{
	string 	sXML;

	sXML += "<PacketIPv4 ";

	sXML += getXML_Attributes_Base();
	sXML += getXML_Attributes_IPv4();

	sXML += " />";

	return sXML;
}

string FMNC_Measurement_Packet_IPv4::getXML_Attributes_IPv4 ()
{
	string	sXML;

	sXML = "TTL=\"" + getTTL_AsString() + "\" TOS=\"" + getTOS_AsString() + "\" IPLength=\"" + getLength_AsString() + "\" ID=\"" + getID_AsString() + "\"";
	sXML += " ";

	return sXML;
}


bool FMNC_Measurement_Packet_IPv4::instrumentPacket_IPv4 (Packet * pPacket)
{
	instrumentPacket_Base(pPacket);

	m_nLength = getPacketLength(pPacket->getData(), pPacket->getLength());

	// TODO: Fix this to use something in ip-utils
	m_byTTL = pPacket->getData()[L2_OFFSET + OFFSET_IPV4_TTL];
	m_byTOS = pPacket->getData()[L2_OFFSET + OFFSET_IPV4_TOS];

	m_nID = getIPv4_ID(pPacket->getData(), pPacket->getLength());

	return true;
}

FMNC_Measurement_Packet_SYN::FMNC_Measurement_Packet_SYN() : FMNC_Measurement_Packet_IPv4 ()
{
	memset(m_byOptions, 0, 100);
	m_byOptionLength = 0;
	m_lClientSeq = 0;
}

bool FMNC_Measurement_Packet_SYN::instrumentPacket (Packet * pPacket)
{
	instrumentPacket_IPv4(pPacket);

	m_byOptionLength = (uint8_t) getTCPOptions_Size (pPacket->getData());

	if(m_byOptionLength <= 100)
	{
		memcpy(m_byOptions, pPacket->getData() + OFFSET_IPV4_DATA, m_byOptionLength);
	}
	else
	{
		cerr << "* Warning: TCP Option length is viewed to be more than 100 bytes, capping at 100" << endl;
		memcpy(m_byOptions, pPacket->getData() + OFFSET_IPV4_DATA, 100);
	}

	m_lClientSeq = getTCP_SeqNumber(pPacket->getData(), pPacket->getLength());

	return true;
}

void	FMNC_Measurement_Packet_SYN::setOptionLength (uint8_t byLength)
{
	m_byOptionLength = byLength;
}

uint8_t	FMNC_Measurement_Packet_SYN::getOptionLength ()
{
	return m_byOptionLength;
}

string	FMNC_Measurement_Packet_SYN::getOptionLength_AsString ()
{
	char szTemp[25];
	string sVal;

	sprintf(szTemp, "%d", m_byOptionLength);
	sVal = szTemp;
	return sVal;
}

string 	FMNC_Measurement_Packet_SYN::getXML ()
{
	string sXML;

	sXML = "<PktSYN ";

	sXML += getXML_Attributes_Base();
	sXML += getXML_Attributes_IPv4();

	if(m_byOptionLength > 0)
	{
		char	szTemp[5];

		sXML += " OpLen=\"" + getOptionLength_AsString() + "\" Options=\"";

		for(int j=0; j<m_byOptionLength; j++)
		{
			sprintf(szTemp, "%02X", m_byOptions[j]);
		}

		sXML += "\"";
	}

	sXML += " ClientSN=\"" + getClientSeqNum_AsString() + "\"";

	sXML += " />";

	return sXML;
}

void	FMNC_Measurement_Packet_SYN::setClientSeqNum (uint32_t lSeqNum)
{
	m_lClientSeq = lSeqNum;
}

uint32_t FMNC_Measurement_Packet_SYN::getClientSeqNum ()
{
	return m_lClientSeq;
}

string	FMNC_Measurement_Packet_SYN::getClientSeqNum_AsString()
{
	char szTemp[25];
	string sVal;

	sprintf(szTemp, "%u", m_lClientSeq);

	sVal = szTemp;
	return sVal;
}



FMNC_Measurement_Packet_TCP::FMNC_Measurement_Packet_TCP () : FMNC_Measurement_Packet_IPv4 ()
{
	m_bFlagAck = false;
	m_lSeqNum = 0;
	m_lAckNum = 0;
	m_sData = "";
	m_byFlags = 0x00;
    m_Ts = 0;
}

void  FMNC_Measurement_Packet_TCP::addData (string sData)
{
	m_sData += sData;
}

string 	FMNC_Measurement_Packet_TCP::getXML ()
{
	string sXML;

	sXML = "<PktTCP ";

	sXML += getXML_Attributes_Base();
	sXML += getXML_Attributes_IPv4();

	sXML += "SN=\"" + getSeqNum_AsString() + "\" ";

	if(m_bFlagAck)
	{
		sXML += "AN=\"" + getAckNum_AsString() + "\" ";
	}

	sXML += "Flags=\"" + getFlagsAsString() + "\" ";

	sXML += "TsVal=\"" + getTs_AsString() + "\" ";

	if(m_sData.size() > 0)
	{
		sXML += ">\n";

		sXML += "<Payload>";
		sXML += "HIDE";
		sXML += "</Payload>";
		sXML += "\n";

		sXML += "</PktTCP>";
	}
	else
	{
		sXML += " />";
	}

	return sXML;
}

void	FMNC_Measurement_Packet_TCP::setSeqNum (uint32_t lSeqNum)
{
	m_lSeqNum = lSeqNum;
}

uint32_t	FMNC_Measurement_Packet_TCP::getTs ()
{
	return m_Ts;
}

string	FMNC_Measurement_Packet_TCP::getTs_AsString ()
{
	char szTemp[25];
	string sVal;

	sprintf(szTemp, "%ld", m_Ts);

	sVal = szTemp;
	return sVal;
}

uint32_t FMNC_Measurement_Packet_TCP::getSeqNum ()
{
	return m_lSeqNum;
}

string FMNC_Measurement_Packet_TCP::getSeqNum_AsString ()
{
	char szTemp[25];
	string sVal;

	sprintf(szTemp, "%u", m_lSeqNum);
	sVal = szTemp;

	return sVal;
}


void	FMNC_Measurement_Packet_TCP::setAckNum (uint32_t lAckNum)
{
	m_lAckNum = lAckNum;
}

uint32_t FMNC_Measurement_Packet_TCP::getAckNum ()
{
	return m_lAckNum;
}

string FMNC_Measurement_Packet_TCP::getAckNum_AsString ()
{
	char szTemp[25];
	string sVal;

	sprintf(szTemp, "%u", m_lAckNum);
	sVal = szTemp;

	return sVal;
}


bool FMNC_Measurement_Packet_TCP::instrumentPacket (Packet * pPacket)
{
	instrumentPacket_IPv4(pPacket);

	m_lSeqNum = getTCP_SeqNumber(pPacket->getData(), pPacket->getLength());
	m_lAckNum = getTCP_AckNumber(pPacket->getData(), pPacket->getLength());
    m_Ts = getTCP_Ts(pPacket->getData(), pPacket->getLength());

	m_byFlags = getTCP_Flags (pPacket->getData());

	if(getTCPFlag_ACK(pPacket->getData(), pPacket->getLength()))
	{
		m_bFlagAck = true;
	}
	else
	{
		m_bFlagAck = false;
	}

	return true;
}

void		FMNC_Measurement_Packet_TCP::setFlags (uint8_t byFlags)
{
	m_byFlags = byFlags;
}

uint8_t		FMNC_Measurement_Packet_TCP::getFlags ()
{
	return m_byFlags;
}

string		FMNC_Measurement_Packet_TCP::getFlagsAsString ()
{
	string	sVal;

	sVal = "";

	if(isFlagSet_URG())
	{
		sVal += "U";
	}

	if(isFlagSet_ACK())
	{
		sVal += "A";
	}

	if(isFlagSet_PSH())
	{
		sVal += "P";
	}

	if(isFlagSet_RST())
	{
		sVal += "R";
	}

	if(isFlagSet_SYN())
	{
		sVal += "S";
	}

	if(isFlagSet_FIN())
	{
		sVal += "F";
	}

	return sVal;
}

bool		FMNC_Measurement_Packet_TCP::isFlagSet_PSH ()
{
	if(m_byFlags & BITMASK_TCP_FLAG_PSH)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool		FMNC_Measurement_Packet_TCP::isFlagSet_ACK ()
{
	if(m_byFlags & BITMASK_TCP_FLAG_ACK)
	{
		return true;
	}
	else
	{
		return false;
	}

}

bool		FMNC_Measurement_Packet_TCP::isFlagSet_RST ()
{
	if(m_byFlags & BITMASK_TCP_FLAG_RST)
	{
		return true;
	}
	else
	{
		return false;
	}

}

bool		FMNC_Measurement_Packet_TCP::isFlagSet_FIN ()
{
	if(m_byFlags & BITMASK_TCP_FLAG_FIN)
	{
		return true;
	}
	else
	{
		return false;
	}

}

bool		FMNC_Measurement_Packet_TCP::isFlagSet_SYN ()
{
	if(m_byFlags & BITMASK_TCP_FLAG_SYN)
	{
		return true;
	}
	else
	{
		return false;
	}

}

bool		FMNC_Measurement_Packet_TCP::isFlagSet_URG ()
{
	if(m_byFlags & BITMASK_TCP_FLAG_URG)
	{
		return true;
	}
	else
	{
		return false;
	}

}
