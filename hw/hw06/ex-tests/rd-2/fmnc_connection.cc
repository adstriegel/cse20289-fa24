/*
 * fmnc_connection.cc
 *
 *  Created on: Apr 14, 2014
 *      Author: striegel
 */

#include<iostream>
using namespace std;


#include "fmnc_connection.h"
#include "../xml/NodeElem.h"

/////////////////////////////////////////////////////////////////////////////////////

FMNC_Connection::FMNC_Connection ()
{
	m_pClient = NULL;
	m_pTestSequence = NULL;
	m_pTestAnalysis = NULL;

	m_lBytes_Sent = 0;
	m_lBytes_Received = 0;
    m_lExp_ACK = 0;
	m_lPkts_Sent = 0;
	m_lPkts_Rcvd = 0;

    m_lAnalyzed = 0;
	m_sRequest = "";

	m_pDictionary = NULL;
    m_pAnalysis = false;
    mFINACKcounter=0;
}

FMNC_Connection::~FMNC_Connection ()
{
	if(m_pDictionary != NULL)
	{
		delete m_pDictionary;
		m_pDictionary = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////////////

ParamDictionary * 	FMNC_Connection::getDictionary ()
{
	return m_pDictionary;
}

/////////////////////////////////////////////////////////////////////////////////////

bool FMNC_Connection::allocateDictionary ()
{
	m_pDictionary = new ParamDictionary();
	if(m_pDictionary != NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////

void	FMNC_Connection::setRequest (string sRequest)
{
	m_sRequest = sanitizeRequest(sRequest);
}

/////////////////////////////////////////////////////////////////////////////////////

string	FMNC_Connection::getRequest ()
{
	return m_sRequest;
}

/////////////////////////////////////////////////////////////////////////////////////

string	FMNC_Connection::getAttribute_Request (bool bTailSpace)
{
	string	sXML;

	if(m_sRequest.size() > 0)
	{
		sXML = createAttributeString("Request", m_sRequest);

		if(bTailSpace)
		{
			sXML += " ";
		}
	}

	return sXML;
}

/////////////////////////////////////////////////////////////////////////////////////

string	FMNC_Connection::sanitizeRequest (string sRequest)
{
	cout << "Pre-sanitized request: " << sRequest << endl;

	// Exclusion
	//   " 		Quotation mark
	//	 \ 		Back slash

	if(m_sRequest.find("\"") != std::string::npos)
	{
		return "";
	}

	if(m_sRequest.find("\\") != std::string::npos)
	{
		return "";
	}

	cout << "Post-sanitized request (PASS): " << sRequest << endl;

	return sRequest;
}

/////////////////////////////////////////////////////////////////////////////////////

uint32_t	FMNC_Connection::getBytes_Sent ()
{
	return m_lBytes_Sent;
}

/////////////////////////////////////////////////////////////////////////////////////

void		FMNC_Connection::setBytes_Sent (uint32_t lBytesSent)
{
	m_lBytes_Sent = lBytesSent;
}

/////////////////////////////////////////////////////////////////////////////////////

void		FMNC_Connection::UpdateExp_ACK(uint32_t lSeq)
{
        if (lSeq > m_lExp_ACK)
                m_lExp_ACK = lSeq;
}
/////////////////////////////////////////////////////////////////////////////////////

uint32_t	FMNC_Connection::getExp_ACK()
{
        return m_lExp_ACK;
}
/////////////////////////////////////////////////////////////////////////////////////

void		FMNC_Connection::adjustBytes_Sent (uint32_t lAdjust)
{
	m_lBytes_Sent += lAdjust;
}

/////////////////////////////////////////////////////////////////////////////////////

uint32_t	FMNC_Connection::getBytes_Received()
{
	return m_lBytes_Received;
}

/////////////////////////////////////////////////////////////////////////////////////

void		FMNC_Connection::setBytes_Received (uint32_t lBytesReceived)
{
	m_lBytes_Received = lBytesReceived;
}

/////////////////////////////////////////////////////////////////////////////////////

void		FMNC_Connection::adjustBytes_Received (uint32_t lAdjust)
{
	m_lBytes_Received += lAdjust;
}

/////////////////////////////////////////////////////////////////////////////////////

uint32_t	FMNC_Connection::getPackets_Sent ()
{
	return m_lPkts_Sent;
}

/////////////////////////////////////////////////////////////////////////////////////

void		FMNC_Connection::setPackets_Sent (uint32_t lPacketSent)
{
	m_lPkts_Sent = lPacketSent;
}

/////////////////////////////////////////////////////////////////////////////////////

void		FMNC_Connection::adjustPackets_Sent (uint32_t lAdjust)
{
	m_lPkts_Sent += lAdjust;
}

/////////////////////////////////////////////////////////////////////////////////////

uint32_t	FMNC_Connection::getPackets_Received    ()
{
	return m_lPkts_Rcvd;
}

/////////////////////////////////////////////////////////////////////////////////////

void		FMNC_Connection::setPackets_Received    (uint32_t lPacketsReceived)
{
	m_lPkts_Rcvd = lPacketsReceived;
}

/////////////////////////////////////////////////////////////////////////////////////

void		FMNC_Connection::adjustPackets_Received (uint32_t lAdjust)
{
	m_lPkts_Rcvd += lAdjust;
}

/////////////////////////////////////////////////////////////////////////////////////

void 		FMNC_Connection::setSessionID (uint32_t lID)
{
	m_lSessionID = lID;
}

uint32_t	FMNC_Connection::getSessionID ()
{
	return m_lSessionID;
}

FMNC_Client * FMNC_Connection::getClient ()
{
	return m_pClient;
}

void FMNC_Connection::setClient (FMNC_Client * pClient)
{
	m_pClient = pClient;
}

int FMNC_Connection::getType ()
{
	return m_nType;
}
bool FMNC_Connection::IsAnalysis ()
{
	return m_pAnalysis;
}

void FMNC_Connection::setType (int nType)
{
	m_nType = nType;
}

void FMNC_Connection::setAnalysis (bool m)
{
	m_pAnalysis = m;
}

uint16_t FMNC_Connection::getFINACKcounter ()
{
	return mFINACKcounter;
}
void FMNC_Connection::IncFINACKcounter ()
{
	 mFINACKcounter++;
}
timeval * FMNC_Connection::getTime_Creation ()
{
	return &m_Time_Creation;
}

void	  FMNC_Connection::setTime_Creation (timeval * pCreation)
{
	m_Time_Creation = *pCreation;
}

timeval * FMNC_Connection::getTime_LastUsed ()
{
	return &m_Time_LastUsed;
}

void	  FMNC_Connection::setTime_LastUsed (timeval * pLastUsed)
{
	m_Time_LastUsed = *pLastUsed;
}

NetAddressIPv4	* FMNC_Connection::getAddress_Source ()
{
	return &m_IP_Source;
}

NetAddressIPv4 	* FMNC_Connection::getAddress_Dest ()
{
	return &m_IP_Dest;
}

uint16_t		  FMNC_Connection::getPort_Source ()
{
	return m_nPort_Source;
}

void			  FMNC_Connection::setPort_Source (uint16_t nPort)
{
	m_nPort_Source = nPort;
}

uint16_t		  FMNC_Connection::getPort_Dest ()
{
	return m_nPort_Dest;
}

void			FMNC_Connection::setPort_Dest (uint16_t nPort)
{
	m_nPort_Dest = nPort;
}

void FMNC_Connection::setTestSequence (FMNC_Test_Sequence * pTestSeq)
{
	m_pTestSequence = pTestSeq;
}
void FMNC_Connection::setTestAnalysis (FMNC_Test_Analysis * pTestAna)
{
	m_pTestAnalysis = pTestAna;
}

FMNC_Test_Sequence * FMNC_Connection::getTestSequence ()
{
	return m_pTestSequence;
}
FMNC_Test_Analysis * FMNC_Connection::getTestAnalysis ()
{
	return m_pTestAnalysis;
}

void FMNC_Connection::displayTuple ()
{
	cout << "  Src: ";
	cout << m_IP_Source.toString();
	cout << ":";
	cout << m_nPort_Source;
	cout << "  Dst: ";
	cout << m_IP_Dest.toString();
	cout << ":";
	cout << m_nPort_Dest;
}


void	FMNC_Connection::populateConnectionTuple (Packet * pPacket)
{

}

void FMNC_Connection::populateInformation (Packet * pPacket)
{
	// Mark this connection as being created effectively when the packet arrived
	setTime_Creation(pPacket->getArrivalTime());

	// Mark the connection as being "accessed" when it was created (for garbage collection)
	setTime_LastUsed(pPacket->getArrivalTime());


	populateConnectionTuple(pPacket);
}

bool FMNC_Connection::populateDictionary_Base ()
{
	if(m_pDictionary != NULL)
	{
		char szTemp[50];
		string sData;

		m_pDictionary->addEntry("ClientIP", m_IP_Source.toString());
		m_pDictionary->addEntry("ServerIP", m_IP_Dest.toString());

		sprintf(szTemp, "%d", m_nPort_Source);
		sData = szTemp;
		m_pDictionary->addEntry("ClientPort", sData);

		sprintf(szTemp, "%d", m_nPort_Dest);
		sData = szTemp;
		m_pDictionary->addEntry("ServerPort", sData);

		sprintf(szTemp, "%d", m_lSessionID);
		sData = szTemp;
		m_pDictionary->addEntry("SessionID", sData);


		sprintf(szTemp, "%d", m_lPkts_Sent);
		sData = szTemp;
		m_pDictionary->addEntry("PacketsTx", sData);

		sprintf(szTemp, "%d", m_lPkts_Rcvd);
		sData = szTemp;
		m_pDictionary->addEntry("PacketsRx", sData);

		sprintf(szTemp, "%d", m_lBytes_Sent);
		sData = szTemp;
		m_pDictionary->addEntry("BytesTx", sData);

		sprintf(szTemp, "%d", m_lBytes_Received);
		sData = szTemp;
		m_pDictionary->addEntry("BytesRx", sData);
	}


	return true;
}

string FMNC_Connection::getAttributes_Base ()
{
	string	sXMLAttr;
	string	sTemp;
	char 	szTemp[30];

	sXMLAttr = "";

	// When did this originate?
	sprintf(szTemp, "%d.%d", m_Time_Creation.tv_sec, m_Time_Creation.tv_usec);
	sTemp = szTemp;
	sXMLAttr += createAttributeString("CreationTime", sTemp);
	sXMLAttr += " ";

	// What is the ID for this particular connection?
	sprintf(szTemp, "%d", m_lSessionID);
	sTemp = szTemp;
	sXMLAttr += createAttributeString("SessionID", sTemp);
	sXMLAttr += " ";

	// Add the object request to the mix (if we had one)
	sXMLAttr += getAttribute_Request(true);

	// Tuple info
	sXMLAttr += createAttributeString("ClientIP", m_IP_Source.toString());
	sXMLAttr += " ";
	sXMLAttr += createAttributeString("ServerIP", m_IP_Dest.toString());
	sXMLAttr += " ";

	sprintf(szTemp, "%d", m_nPort_Source);
	sTemp = szTemp;
	sXMLAttr += createAttributeString("SrcPort", sTemp);
	sXMLAttr += " ";

	sprintf(szTemp, "%d", m_nPort_Dest);
	sTemp = szTemp;
	sXMLAttr += createAttributeString("DestPort", sTemp);
	sXMLAttr += " ";

	sprintf(szTemp, "%d", m_lPkts_Sent);
	sTemp = szTemp;
	sXMLAttr += createAttributeString("PktsTx", sTemp);
	sXMLAttr += " ";

	sprintf(szTemp, "%d", m_lPkts_Rcvd);
	sTemp = szTemp;
	sXMLAttr += createAttributeString("PktsRx", sTemp);
	sXMLAttr += " ";

	sprintf(szTemp, "%d", m_lBytes_Sent);
	sTemp = szTemp;
	sXMLAttr += createAttributeString("BytesTx", sTemp);
	sXMLAttr += " ";

	sprintf(szTemp, "%d", m_lBytes_Received);
	sTemp = szTemp;
	sXMLAttr += createAttributeString("BytesRx", sTemp);
	sXMLAttr += " ";

	return sXMLAttr;
}


string FMNC_Connection::getXML_Summary ()
{
	string sXML;

	sXML = "";

	sXML += "<Connection ";

	sXML += ">";

	// Anything in the middle?

	sXML += "</Connection>";

	return "";
}

bool FMNC_Connection::isMatch (Packet * pPacket)
{
	return false;
}

bool  		FMNC_Connection::fluctuation_Analysis   ()
{
	return false;
}
bool  		FMNC_Connection::hiccup_Analysis   ()
{
	return false;
}

bool  		FMNC_Connection::conductAnalysis     ()
{
	return false;
}

string		FMNC_Connection::extractAnalysis_Web ()
{
	return "";
}

FMNC_List_Connections::FMNC_List_Connections ()
{
	pthread_mutex_init(&m_MutexList, NULL);
}

bool FMNC_List_Connections::add (FMNC_Connection * pConnection)
{
	pthread_mutex_lock(&m_MutexList);
	m_Connections.push_back(pConnection);
	pthread_mutex_unlock(&m_MutexList);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////

FMNC_Connection * 	FMNC_List_Connections::findConnection (uint32_t lSessionID)
{
	int		j;
	FMNC_Connection *	pConnection;

	pConnection = NULL;

	pthread_mutex_lock(&m_MutexList);

	for(j=0; j<m_Connections.size(); j++)
	{
		if(m_Connections[j]->getSessionID() == lSessionID)
		{
			pConnection = m_Connections[j];
			break;
		}
	}

	pthread_mutex_unlock(&m_MutexList);
	return pConnection;
}

///////////////////////////////////////////////////////////////////////////////////

FMNC_Connection *	FMNC_List_Connections::findConnection (Packet * pPacket)
{
	int		j;
	FMNC_Connection *	pConnection;

	pConnection = NULL;

	pthread_mutex_lock(&m_MutexList);

	for(j=0; j<m_Connections.size(); j++)
	{
		if(m_Connections[j]->isMatch(pPacket))
		{
			pConnection = m_Connections[j];
			break;
		}
	}

	pthread_mutex_unlock(&m_MutexList);
	return pConnection;
}

bool FMNC_List_Connections::removeConnection (FMNC_Connection* pCon)
{


	pthread_mutex_lock(&m_MutexList);


    for(int i=0;i<m_Connections.size();i++){
            if(m_Connections[i] == pCon){
                    m_Connections.erase(m_Connections.begin()+i);
                    pthread_mutex_unlock(&m_MutexList);
                    return true;
            }
    }
    pthread_mutex_unlock(&m_MutexList);
    return false;
            
}

FMNC_Connection *	FMNC_List_Connections::popOldEntry ()
{
	FMNC_Connection * pConn;

	pConn = NULL;

	pthread_mutex_lock(&m_MutexList);

	if(m_OldConnections.size() > 0)
	{
		pConn = m_OldConnections[0];
		m_OldConnections.erase(m_OldConnections.begin());
	}

	pthread_mutex_unlock(&m_MutexList);
	return pConn;
}

bool	FMNC_List_Connections::populateOldList (uint32_t nOldTime)
{
	timeval		currentTime;
	int			j;
	bool		bFoundOld;

	bFoundOld = false;

	pthread_mutex_lock(&m_MutexList);

	// What is the current time?
	gettimeofday(&currentTime, NULL);

	for(j=0; j<m_Connections.size(); j++)
	{
		if((currentTime.tv_sec - m_Connections[j]->getTime_LastUsed()->tv_sec) >= nOldTime)
		{
			m_OldConnections.push_back(m_Connections[j]);
			m_Connections.erase(m_Connections.begin()+j);
			j--;
			bFoundOld = true;
		}
	}

	pthread_mutex_unlock(&m_MutexList);
	return bFoundOld;
}

void FMNC_List_Connections::displayToConsole ()
{
	int		j;

	pthread_mutex_lock(&m_MutexList);

	cout << "Connections: " << m_Connections.size() << endl;

	for(j=0; j<m_Connections.size(); j++)
	{
		cout << "  C ";
		printf("%03d", j);
		cout << " : ";

		m_Connections[j]->displayTuple();

		cout << "  " << m_Connections[j]->getTime_LastUsed()->tv_sec << "." << m_Connections[j]->getTime_LastUsed()->tv_usec << endl;
	}

	pthread_mutex_unlock(&m_MutexList);
}

void FMNC_List_Connections::dumpList ()
{
	int		j;

	pthread_mutex_lock(&m_MutexList);

	cout << "Connection List of " << m_Connections.size();

	for(j=0; j<m_Connections.size(); j++)
	{
		cout << "  C ";
		printf("%03d", j);
		cout << " : ";

		m_Connections[j]->displayTuple();
	}

	pthread_mutex_unlock(&m_MutexList);
}
