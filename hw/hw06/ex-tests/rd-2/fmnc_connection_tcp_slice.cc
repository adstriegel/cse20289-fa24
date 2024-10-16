/*
 * fmnc_connection_tcp_slice.cc
 *
 *  Created on: Jun 3, 2014
 *      Author: striegel
 */

#include "fmnc_connection_tcp_slice.h"
#include "../util/ip-utils.h"
#include "fmnc_test_sequence.h"

#include <iostream>
using namespace std;

FMNC_Connection_TCP_Slice::FMNC_Connection_TCP_Slice () : FMNC_Connection ()
{
	m_nClientSeqNum = 0;
	m_nServerSeqNum = 0;
	m_nState = FMNC_TCP_STATE_NONE;
	m_lAckedNum_FromClient = 0;
	m_pRefPacket = NULL;
	m_nPureAcks = 0;
    m_lAnalyzed = 0;
    m_nRmin = -1;
    m_nRmax = -1;
    m_dAlpha = 0;
    m_nRate_Index = 1;
    m_nInstantRTT = 0;
    m_nRates.clear();

}

FMNC_Connection_TCP_Slice::~FMNC_Connection_TCP_Slice ()
{
	// If the reference packet is present, we own it.  We need to therefore free the packet back into the
	//  global pool
	if(m_pRefPacket != NULL)
	{
		m_pRefPacket->release();
		m_pRefPacket = NULL;
	}

	if(getTestSequence() != NULL)
	{
		delete getTestSequence();
	}

	if(m_pTestAnalysis != NULL)
	{
		delete m_pTestAnalysis;
	}
}

string	FMNC_Connection_TCP_Slice::getXML_Summary ()
{
	string		sXML;

	sXML = "";

	sXML += "<ConnectionTCPSlice ";

	sXML += getAttributes_Base();

	sXML += ">\n";

	// Get the measurement packet info
	sXML += m_Setup.getXML("MeasureSetup");

	sXML += "\n";

	sXML += m_Received.getXML("MeasureRcvd");

	sXML += "\n";

	sXML += m_Sent.getXML("MeasureSent");

	sXML += "\n";

	if(getTestSequence() != NULL)
	{
		sXML += getTestSequence()->getXML();
	}

	sXML += "\n";
	sXML += "</ConnectionTCPSlice>";
	sXML += "\n";

	return sXML;
}

FMNC_List_Measurement_Packets *		FMNC_Connection_TCP_Slice::getMeasurementPkts_Setup ()
{
	return &m_Setup;
}

FMNC_List_Measurement_Packets *		FMNC_Connection_TCP_Slice::getMeasurementPkts_Received ()
{
	return &m_Received;
}

FMNC_List_Measurement_Packets *		FMNC_Connection_TCP_Slice::getMeasurementPkts_Sent ()
{
	return &m_Sent;
}

void FMNC_Connection_TCP_Slice::setLastRcvd_Ack 		(uint32_t lAckNum)
{
	m_lastRcvd_ack = lAckNum;
}

void FMNC_Connection_TCP_Slice::incRcvd_RepsAck 		()
{
	m_rcvd_repsack ++;
}
void FMNC_Connection_TCP_Slice::resetRcvd_RepsAck 		()
{
	m_rcvd_repsack = 0;
}
uint32_t FMNC_Connection_TCP_Slice::getLastRcvd_Ack 	()
{
	return m_lastRcvd_ack;
}
uint32_t FMNC_Connection_TCP_Slice::getRcvd_RepsAck 	()
{
	return m_rcvd_repsack;
}

void FMNC_Connection_TCP_Slice::setAckedNum_FromClient 		(uint32_t lSeqNum)
{
	m_lAckedNum_FromClient = lSeqNum;
}

uint32_t FMNC_Connection_TCP_Slice::getAckedNum_FromClient 	()
{
	return m_lAckedNum_FromClient;
}


int	 FMNC_Connection_TCP_Slice::getState ()
{
	return m_nState;
}

void FMNC_Connection_TCP_Slice::setState (int nState)
{
	m_nState = nState;
}

bool FMNC_Connection_TCP_Slice::isMatch (Packet * pPacket)
{
	// Compare source / destination IP addresses

	if(memcmp(pPacket->getData()+L2_OFFSET+OFFSET_IPV4_PROTOCOL_SRCIP, getAddress_Source()->getAddress(), DEFAULT_IP_LEN) != 0)
	{
		//cout << "  Match failure - Source IP" << endl;
		return false;
	}

	if(memcmp(pPacket->getData()+L2_OFFSET+OFFSET_IPV4_PROTOCOL_DSTIP, getAddress_Dest()->getAddress(), DEFAULT_IP_LEN) != 0)
	{
		//cout << "  Match failure - Dest IP" << endl;
		return false;
	}

	uint16_t	nPort;

	nPort = getPort_Source();

	if(nPort != getTCP_SrcPort(pPacket->getData()))
	{
		//cout << "  Match failure - Source Port" << endl;
		//cout << "    " << nPort << " vs. " << getTCP_SrcPort(pPacket->getData()) << endl;
		return false;
	}

	nPort = getPort_Dest();

	if(nPort != getTCP_DstPort(pPacket->getData()))	{
		//cout << "  Match failure - Dest Port" << endl;
		return false;
	}

	return true;
}


void FMNC_Connection_TCP_Slice::reversePacket (Packet * pPacket)
{
	// Flip the Layer 2 MAC address
	char	byMAC[DEFAULT_MAC_LEN];

	memcpy(byMAC, pPacket->getData()+OFFSET_ETH_DSTMAC, DEFAULT_MAC_LEN);
	memcpy(pPacket->getData()+OFFSET_ETH_DSTMAC, pPacket->getData()+OFFSET_ETH_SRCMAC, DEFAULT_MAC_LEN);
	memcpy(pPacket->getData()+OFFSET_ETH_SRCMAC, byMAC, DEFAULT_MAC_LEN);

	// Reset the TTL

	// Flip the source and destination addresses
	memcpy(pPacket->getData()+L2_OFFSET+OFFSET_IPV4_PROTOCOL_SRCIP, getAddress_Dest()->getAddress(), DEFAULT_IP_LEN);
	memcpy(pPacket->getData()+L2_OFFSET+OFFSET_IPV4_PROTOCOL_DSTIP, getAddress_Source()->getAddress(), DEFAULT_IP_LEN);

	// TODO: Double check the size of the IPv4 header

	// Recompute the IP header?

	// Flip the source and destination ports
	uint16_t	nPort;

	// TODO: Validate endianness here, we might be in trouble??

	nPort = htons(getPort_Dest());
	memcpy(pPacket->getData()+L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_SRCPORT, &nPort, 2);
	nPort = htons(getPort_Source());
	memcpy(pPacket->getData()+L2_OFFSET+L3_IPV4_OFFSET+OFFSET_TCP_DSTPORT, &nPort, 2);

	// Recompute the TCP header?

	// Fix the IP checksum
	updateIPv4_Checksum(pPacket->getData(), pPacket->getLength());

	updateTCP_Checksum(pPacket->getData(), pPacket->getLength());

	updateIPv4_Checksum(pPacket->getData(), pPacket->getLength());
}

void FMNC_Connection_TCP_Slice::setReferencePacket (Packet * pPacket)
{
	m_pRefPacket = pPacket;

	if(m_pRefPacket != NULL)
	{
		clearTCP_Flags(pPacket->getData());
	}
}

Packet * FMNC_Connection_TCP_Slice::getReferencePacket ()
{
	return m_pRefPacket;
}

Packet * FMNC_Connection_TCP_Slice::respondToData (Packet * pDataPacket)
{
	Packet 	*	pPacket;

	// Instrument this packet
	FMNC_Measurement_Packet_TCP	* pMeasureData;
	pMeasureData = new FMNC_Measurement_Packet_TCP();
	pMeasureData->setMetaData("DataIn");

	char	szTemp[2000];
	string	sDataTmp;

	// TODO: Fix this hot mess //lsong 
    int nn = 20;
    if(pDataPacket->getLength() < 14+20+nn){
            return NULL;
    }

	//cout << " RespondToData Copying over " << (pDataPacket->getLength()-14-20-nn) << " bytes" << endl;

	memcpy(szTemp, pDataPacket->getData()+14+20+nn, pDataPacket->getLength()-14-20-nn);
	szTemp[pDataPacket->getLength()-14-20-nn] = '\0';
	sDataTmp = szTemp;
	pMeasureData->addData(sDataTmp);

	pMeasureData->instrumentPacket(pDataPacket);
	m_Setup.addPacket(pMeasureData);

	// Clone and reverse the packet to return it to the client
	pPacket = pDataPacket->cloneFromPool();

	// Flip the source and destination aspects
	reversePacket(pPacket);

	// Clear TCP flags
	clearTCP_Flags(pPacket->getData());

	// Set the ACK flag to be true
	setTCPFlag_ACK(pPacket->getData(), pPacket->getLength(), 1);


	// We ACK back at the client sequence number for all of the data received
	//
	//  TODO: Should actually pay attention to client order / information (maybe)
	incSeqNum_Client(getTCP_PayloadSize(pPacket->getData()));
	writeTCP_AckNumber(getSeqNum_Client(), pPacket->getData(), pPacket->getLength());

	// What should this be?
	writeTCP_SeqNumber(getSeqNum_Server(), pPacket->getData(), pPacket->getLength());

	// We need to make this into a zero payload packet
	//	  40 bytes (IP, TCP base headers only)
	//		 ACK flag set in the TCP header
	//	  54 bytes for the overall packet
	//		 14 (L2) + 20 (IPv4) + 20 (TCP) + 12 Option
	setPacketLength(pPacket->getData(), 20+nn);
	pPacket->setLength(20+nn+L2_OFFSET);

	updateIPv4_Checksum(pPacket->getData(), pPacket->getLength());
	updateTCP_Checksum(pPacket->getData(), pPacket->getLength());
	updateIPv4_Checksum(pPacket->getData(), pPacket->getLength());

	// Create a measurement packet
	FMNC_Measurement_Packet_TCP	*	pMeasureAck;

	pMeasureAck = new FMNC_Measurement_Packet_TCP();
	pMeasureAck->instrumentPacket(pPacket);
	pMeasureAck->appendMetaData("DataAck");
	m_Setup.addPacket(pMeasureAck);

	return pPacket;
}

Packet * FMNC_Connection_TCP_Slice::respondToSYN (Packet * pSynPacket)
{
	Packet 	*	pPacket;

	// Make a measurement packet to capture information with respect to the
	//  IP header and requested TCP options (even though we don't support any options)
	FMNC_Measurement_Packet_SYN	*	pMeasureSyn;

	pMeasureSyn = new FMNC_Measurement_Packet_SYN();
	pMeasureSyn->instrumentPacket(pSynPacket);
	m_Setup.addPacket(pMeasureSyn);

	// Clone and reverse the packet to return it to the client
	pPacket = pSynPacket->cloneFromPool();

	// Flip the source and destination aspects
	reversePacket(pPacket);

	// Clear any TCP options (we don't play TCP options) lsong: we need the Tsval
	clearOptions_TCP_TsOptOn(pPacket);
	//clearOptions_TCP(pPacket);

	// Make sure the SYN flag is still true
	//  Optional - should still be set

	// Set the ACK flag to be true
	setTCPFlag_ACK(pPacket->getData(), pPacket->getLength(), 1);
    cout<<"DBG(lsong):setTCPFlag_ACK"<<endl;

	// We ACK back at the client sequence number + 1
	incSeqNum_Client(1);
	writeTCP_AckNumber(getSeqNum_Client(), pPacket->getData(), pPacket->getLength());

	writeTCP_SeqNumber(getSeqNum_Server(), pPacket->getData(), pPacket->getLength());
	incSeqNum_Server(1);

	// Recompute the TCP checksum

	// Recompute any IP checksums
	updateIPv4_Checksum(pPacket->getData(), pPacket->getLength());
	updateTCP_Checksum(pPacket->getData(), pPacket->getLength());
	updateIPv4_Checksum(pPacket->getData(), pPacket->getLength());

	// Change the connection state to denote that we have now done a SYN-ACK
	setState(FMNC_TCP_STATE_SYNACK);

	// Create a measurement packet
	FMNC_Measurement_Packet_TCP	*	pMeasureSynAck;

	pMeasureSynAck = new FMNC_Measurement_Packet_TCP();
	pMeasureSynAck->instrumentPacket(pPacket);
	pMeasureSynAck->appendMetaData("SynAck");
	m_Setup.addPacket(pMeasureSynAck);

	// Clone / save the packet for outbound info, particularly Layer 2 info to help us out
	setReferencePacket(pPacket->cloneFromPool());

	return pPacket;
}

void FMNC_Connection_TCP_Slice::clearOptions_TCP (Packet * pPacket)
{
	uint16_t	nDiff;

	//nDiff = purgeTCPOptions_TsOptOn(pPacket->getData(), pPacket->getLength());
	nDiff = purgeTCPOptions(pPacket->getData(), pPacket->getLength());

	if(nDiff > 0)
	{
		//cout << "TCP Options Purge: " << nDiff << " bytes" << endl;
		pPacket->setLength(pPacket->getLength() - nDiff);
	}
	else
	{
		//cout << "No TCP Options to purge" << endl;
		// Nothing to purge
	}
}

void FMNC_Connection_TCP_Slice::clearOptions_TCP_TsOptOn (Packet * pPacket)
{
	uint16_t	nDiff;

	nDiff = purgeTCPOptions_TsOptOn(pPacket->getData(), pPacket->getLength());
	//nDiff = purgeTCPOptions(pPacket->getData(), pPacket->getLength());

	if(nDiff > 0)
	{
		cout << "TCP Options Purge with Timestamp On: " << nDiff << " bytes" << endl;
		pPacket->setLength(pPacket->getLength() - nDiff);
	}
	else
	{
		//cout << "No TCP Options to purge" << endl;
		// Nothing to purge
	}
}
void FMNC_Connection_TCP_Slice::setSeqNum_Server (uint32_t lSeqNum)
{
	m_nServerSeqNum = lSeqNum;
}

uint32_t FMNC_Connection_TCP_Slice::getSeqNum_Server()
{
	return m_nServerSeqNum;
}

void FMNC_Connection_TCP_Slice::incSeqNum_Client (uint32_t lInc)
{
	m_nClientSeqNum += lInc;
}

void FMNC_Connection_TCP_Slice::setSeqNum_Client (uint32_t lSeqNum)
{
	m_nClientSeqNum = lSeqNum;
}

uint32_t FMNC_Connection_TCP_Slice::getSeqNum_Client()
{
	return m_nClientSeqNum;
}

void FMNC_Connection_TCP_Slice::incSeqNum_Server (uint32_t lInc)
{
	m_nServerSeqNum += lInc;
}


void FMNC_Connection_TCP_Slice::populateInformation (Packet * pPacket)
{
	FMNC_Connection::populateInformation(pPacket);

	// Fill in the client sequence number as seen by this SYN packet
	setSeqNum_Client(getTCP_SeqNumber(pPacket->getData(), pPacket->getLength()));


	cout <<"  Src IP: ";
	dumpIPv4(getIPv4_Address_Src(pPacket->getData()));
	cout << endl;
	cout << "  Seq Number: " << getSeqNum_Client() << endl;
}

void FMNC_Connection_TCP_Slice::populateConnectionTuple (Packet * pPacket)
{
	NetAddressIPv4	*	pAddress;

	cout << "Populating the connection tuple" << endl;

	// Set the source and destination address tuples
	pAddress = getAddress_Source();
	pAddress->setAddress((unsigned char *) getIPv4_Address_Src(pPacket->getData()), DEFAULT_IP_LEN);

	pAddress = getAddress_Dest();
	pAddress->setAddress((unsigned char *) getIPv4_Address_Dst(pPacket->getData()), DEFAULT_IP_LEN);

	// Set the source and destination ports
	setPort_Source (getTCP_SrcPort(pPacket->getData()));
	setPort_Dest (getTCP_DstPort(pPacket->getData()));
}

uint16_t	FMNC_Connection_TCP_Slice::getCount_PureAcks ()
{
	return m_nPureAcks;
}

void	FMNC_Connection_TCP_Slice::incrementCount_PureAcks ()
{
	m_nPureAcks++;
}

bool	FMNC_Connection_TCP_Slice::hiccup_Analysis ()
{
        m_pTestAnalysis = getTestAnalysis();
        if(m_pTestAnalysis != NULL)
        {

                bool re;
                m_pTestAnalysis->setMeasurementLists(&m_Sent, &m_Received);
                re = m_pTestAnalysis->do_hiccup_analysis(m_lAnalyzed);
                m_lAnalyzed = m_Sent.getCount();
                return re;
        }
        else
        {
                cerr << "** Error: Please set the FMNC_Test_Analysis before analysis !"<<endl;
                return true;
        }

}
bool	FMNC_Connection_TCP_Slice::fluctuation_Analysis ()
{
        m_pTestAnalysis = getTestAnalysis();
        if(m_pTestAnalysis != NULL)
        {

                m_pTestAnalysis->setMeasurementLists(&m_Sent, &m_Received);
                m_pTestAnalysis->do_fluctuation_analysis(m_lAnalyzed, m_dAlpha, &m_nInstantRTT);
                if(m_pTestAnalysis->getFluctuation_Result() > -1)
                        m_nRate_Index += m_pTestAnalysis->getFluctuation_Result() - 1;
                m_lAnalyzed = m_Sent.getCount();
                return true;
        }
        else
        {
                cerr << "** Error: Please set the FMNC_Test_Analysis before analysis !"<<endl;
                return false;
        }


}
bool  		FMNC_Connection_TCP_Slice::conductAnalysis ()
{
        m_pTestAnalysis = getTestAnalysis();
        if(m_pTestAnalysis != NULL)
        {

                m_pTestAnalysis->setMeasurementLists(&m_Sent, &m_Received);

                if(m_pTestAnalysis->doAnalysis())
                {
                        setAnalysis(true) ;
                        return true;
                }
                else
                {
                        return false;
                }
        }
        else
        {
                cerr << "** Error: Please set the FMNC_Test_Analysis before analysis !"<<endl;
                return false;
        }
    
}

void FMNC_Connection_TCP_Slice::setSearch_Param(int rmin, int rmax)
{
        m_nRmin = rmin;
        m_nRmax = rmax;
        if(find(m_nRates.begin(),m_nRates.end(),rmin) == m_nRates.end()){
                auto it = std::lower_bound(m_nRates.begin(), m_nRates.end(), rmin); 
                m_nRates.insert( it, rmin  );
        }
        if(find(m_nRates.begin(),m_nRates.end(),rmax) == m_nRates.end()){
                auto it = std::lower_bound(m_nRates.begin(), m_nRates.end(), rmax); 
                m_nRates.insert( it, rmax  );
        }
}

int FMNC_Connection_TCP_Slice::getRmax()
{
        if(m_nRate_Index >= m_nRates.size())
                return -1;
        else
                return m_nRates[m_nRate_Index];
}
int FMNC_Connection_TCP_Slice::getRmin()
{
        if(m_nRate_Index == 0)
                return -1;
        else
                return m_nRates[m_nRate_Index-1];
}
uint16_t FMNC_Connection_TCP_Slice::getInstantRTT()
{
        return m_nInstantRTT;
}
int FMNC_Connection_TCP_Slice::getRate_Index()
{
        return m_nRate_Index;
}
void FMNC_Connection_TCP_Slice::setTrainDuration(double d)
{
        m_dAlpha = d;
}

string		FMNC_Connection_TCP_Slice::extractAnalysis_Web ()
{
	if(m_pTestAnalysis != NULL)
	{
		string 	sOutput;

		sOutput = "";



		return sOutput;
	}
	else
	{
		return "";
	}
}


