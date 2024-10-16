/*
 * fmnc_connection_tcp_slice.h
 *
 *  Created on: Apr 14, 2014
 *      Author: striegel
 */

#ifndef FMNC_CONNECTION_TCP_SLICE_H_
#define FMNC_CONNECTION_TCP_SLICE_H_

#include <stdint.h>

#include "fmnc_connection.h"
#include "fmnc_measurement_packet.h"
#include "fmnc_test_analysis.h"
#include "../pkt/Packet.h"

class FMNC_Test_Analysis;

#define FMNC_TCP_STATE_NONE		0		// Nothing - no state
#define FMNC_TCP_STATE_SYN		1		// SYN Received from client
#define FMNC_TCP_STATE_SYNACK	2		// SYN-ACK has been sent to client, waiting for ACK
#define FMNC_TCP_STATE_WAITDATA	3		// Waiting on the first data packet from the client
#define FMNC_TCP_STATE_ACTIVE	4		// Client has sent first data, normal operation from here on out
#define FMNC_TCP_STATE_WAITING	5		// Waiting for ACKs from client

#define FMNC_TCP_STATE_WAITING_FIN_ACK		6		// Waiting for the FIN ACK from the client to wrap things up


/** A connection that will be sliced on the outbound writes from the server to the client
 */
class FMNC_Connection_TCP_Slice : public FMNC_Connection
{
	public:
		FMNC_Connection_TCP_Slice();
		~FMNC_Connection_TCP_Slice ();

		bool	isMatch (Packet * pPacket);

		void setSeqNum_Client (uint32_t lSeqNum);
		uint32_t getSeqNum_Client ();
		void 	 incSeqNum_Client (uint32_t lInc);

		// What has the client acked back to us, i.e. what byte range has the client successfully
		//  received and acknowledged to us as the server?
		void setAckedNum_FromClient 		(uint32_t lSeqNum);
		uint32_t getAckedNum_FromClient 	();
        //Check the received ack to determin retransmission
        
        void setLastRcvd_Ack 		(uint32_t lAckNum);

        void incRcvd_RepsAck 		() ;
        void resetRcvd_RepsAck 		() ;
        uint32_t getLastRcvd_Ack 	() ;
        uint32_t getRcvd_RepsAck 	() ;


		void setSeqNum_Server (uint32_t lSeqNum);
		uint32_t getSeqNum_Server();

		/// @brief Increase the sequence number for the server side
		/// @param lInc The amount to increase the server sequence number
		void 	 incSeqNum_Server (uint32_t lInc);

		/// @brief Retrieve the state associated with this particular connection
		/// @returns Integer denoting the state, see the definition header for info
		int	 getState ();

		/// @brief Set the current state of the TCP connection (None, SYN, SYN+ACK)
		/// @param nState The new state for the connection
		void setState (int nState);

		// Process a packet from the client
		bool processPacketFromClient (Packet * pPacket);

		// Respond to the SYN packet and craft a response
		Packet * respondToSYN (Packet * pSynPacket);

		// Respond to the SYN packet and craft a response
		Packet * respondToData (Packet * pDataPacket);


		virtual void	populateInformation (Packet * pPacket);

		virtual void	populateConnectionTuple (Packet * pPacket);

		// Take a packet and flip all source / destination aspects including
		// Layer 2 (MAC Src, Dst), Layer 3 (Src, Dst IP), Layer 4 (Src, Dst Port)
		void	reversePacket (Packet * pPacket);

		void	clearOptions_TCP (Packet * pPacket);
		void	clearOptions_TCP_TsOptOn (Packet * pPacket);

        void setTrainDuration(double alpha);
        void setSearch_Param(int Rmin, int Rmax);
        int getRmin();
        int getRmax();
        int getRate_Index();
        uint16_t getInstantRTT();
        vector<int> m_nRates;

		void	setReferencePacket (Packet * pPacket);
		Packet * getReferencePacket ();

		FMNC_List_Measurement_Packets *		getMeasurementPkts_Setup ();
		FMNC_List_Measurement_Packets *		getMeasurementPkts_Received ();
		FMNC_List_Measurement_Packets *		getMeasurementPkts_Sent ();

		uint16_t	getCount_PureAcks ();
		void		incrementCount_PureAcks ();

		virtual string			getXML_Summary ();

		virtual bool  		conductAnalysis     ();
		virtual bool  		fluctuation_Analysis     ();
		virtual bool  		hiccup_Analysis     ();
		virtual string		extractAnalysis_Web ();



	protected:
		bool processPacketFromClient_InitialSYN (Packet * pPacket);

	private:
		// What is the current state of the connection?
		int				m_nState;

		// What is the client sequence number?
		uint32_t		m_nClientSeqNum;

		// What is the server sequence number (us)?
		uint32_t		m_nServerSeqNum;

		// What is the highest number that they have acked to us?
		uint32_t		m_lAckedNum_FromClient;

		// When did we send data out?

		Packet *		m_pRefPacket;

		uint16_t		m_nPureAcks;
        //Retransmission assist: check the deplicated acks
        uint32_t m_lastRcvd_ack;
        uint32_t m_rcvd_repsack;
        uint32_t m_lAnalyzed;
        int m_nRmin;
        int m_nRmax;
        double m_dAlpha;
        int m_nRate_Index;

        uint16_t m_nInstantRTT;


		FMNC_List_Measurement_Packets	m_Setup;
		FMNC_List_Measurement_Packets	m_Sent;
		FMNC_List_Measurement_Packets 	m_Received;

		FMNC_Test_Analysis	*			m_pTestAnalysis;

};


#endif /* FMNC_CONNECTION_TCP_SLICE_H_ */
