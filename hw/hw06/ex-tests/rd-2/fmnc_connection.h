/*
 * fmnc_connection.h
 *
 *  Created on: Apr 14, 2014
 *      Author: striegel
 */

#ifndef FMNC_CONNECTION_H_
#define FMNC_CONNECTION_H_

#include "../pkt/Packet.h"
#include "../pkt/address/NetAddressIPv4.h"

#include <vector>
using namespace std;

#include <string>
using namespace std;

#include <pthread.h>

#include "../util/ParamDictionary.h"

class FMNC_Client;
class FMNC_Test_Sequence;
class FMNC_Test_Analysis;

/** One individual Fast Mobile Network Connection that is captured by a five-tuple
 * designation which includes source IP, destination IP, source port, destination
 * port, and protocol (TCP, UDP).
 *
 */
class FMNC_Connection
{
	public:
		FMNC_Connection ();
		~FMNC_Connection ();

		virtual bool	isMatch (Packet * pPacket);


		FMNC_Client * getClient ();
		void setClient (FMNC_Client * pClient);

		int getType ();
		void setType (int nType);
        void setAnalysis (bool m);
        uint16_t getFINACKcounter ();
        void IncFINACKcounter ();
        bool IsAnalysis();

		timeval * getTime_Creation ();
		void	  setTime_Creation (timeval * pCreation);

		timeval * getTime_LastUsed ();
		void	  setTime_LastUsed (timeval * pLastUsed);

		uint32_t	getBytes_Sent ();
		void		setBytes_Sent (uint32_t lBytesSent);
		void		adjustBytes_Sent (uint32_t lAdjust);

		uint32_t	getBytes_Received();
		void		setBytes_Received (uint32_t lBytesReceived);
		void		adjustBytes_Received (uint32_t lAdjust);

		uint32_t	getPackets_Sent ();
		void		setPackets_Sent (uint32_t lPacketSent);
		void		adjustPackets_Sent (uint32_t lAdjust);

		uint32_t	getPackets_Received    ();
		void		setPackets_Received    (uint32_t lPacketsReceived);
		void		adjustPackets_Received (uint32_t lAdjust);

		void		UpdateExp_ACK (uint32_t lAdjust);
		uint32_t		getExp_ACK ();
		virtual void	populateInformation (Packet * pPacket);

		virtual void	populateConnectionTuple (Packet * pPacket);

		/// Display the connection tuple info the console in a single line, no end carriage return
		void			displayTuple ();

		NetAddressIPv4	* getAddress_Source ();
		NetAddressIPv4 	* getAddress_Dest ();
		uint16_t		  getPort_Source ();
		void				setPort_Source (uint16_t nPort);
		uint16_t		getPort_Dest ();
		void				setPort_Dest (uint16_t nPort);

		void			setTestSequence (FMNC_Test_Sequence * pTestSeq);
		void			setTestAnalysis (FMNC_Test_Analysis * pTestAna);
		FMNC_Test_Sequence * getTestSequence ();
		FMNC_Test_Analysis * getTestAnalysis ();

		virtual string			getXML_Summary ();

		void 		setSessionID (uint32_t lID);
		uint32_t	getSessionID ();

		string		getAttributes_Base();

		virtual bool  		conductAnalysis     ();
		virtual bool  		fluctuation_Analysis     ();
		virtual bool  		hiccup_Analysis     ();
		virtual string		extractAnalysis_Web ();

		/** Set the object or type of material being requested related to this particular connection
		 * @param sRequest The object being requested
		 */
		void		setRequest (string sRequest);

		/** Retrieve the object being requested (if known)
		 * @returns The object driving this particular connection
		 */
		string		getRequest ();

		/** Extract the request itself as a XML attribute
		 * @param bTailSpace Denote if there should a space following this attribute
		 * @returns The attribute as a XML attribute
		 */
		string		getAttribute_Request (bool bTailSpace);


		/** Retrieve a reference to the dictionary associated with this particular
		 * connection. Those values may be populated over time.
		 * @returns NULL if unallocated, non-NULL otherwise
		 */
		ParamDictionary * 	getDictionary ();

		/** Allocate a dictionary for this particular connection
		 * @returns True if successful, false if unsuccessful
		 */
		bool				allocateDictionary ();

		/** Populate the dictionary with information from the base information
		 * for the connection
		 * @param True if successful, false otherwise
		 */
		bool 				populateDictionary_Base ();

	protected:

		/** Given a particular request object, sanitize the object to ensure that there are not unexpected
		 * characters.  The characters should all fall in the normal ASCII range without quotations or anything
		 * that would create issues for the resultant XML file
		 * @returns The newly sanitized string, a zero length string if the request cannot be sanitized with catastrophic issues
		 */
		string		sanitizeRequest (string sRequest);

	private:
		/* The client associated with this particular connection */
		FMNC_Client 	*	m_pClient;

		/* What kind of connection is this particular connection? */
		int					m_nType;

		/* When was this connection created? */
		timeval				m_Time_Creation;

		/* When was this connection last used? */
		timeval				m_Time_LastUsed;

		/* How many bytes have been sent? */
		uint32_t			m_lBytes_Sent;

		/* How many bytes have been received? */
		uint32_t			m_lBytes_Received;

		/* How many packets have been sent? */
		uint32_t			m_lPkts_Sent;

		/* How many packets have been received */
		uint32_t			m_lPkts_Rcvd;

        uint16_t mFINACKcounter;
		/* Connection tuple info (IPv4 - for now) */

		/* Maximal Seq number */
		uint32_t			m_lExp_ACK;
        bool m_pAnalysis;
		// TODO: Replumb this for a generic flow tuple (v4 or v6 compliant)
		NetAddressIPv4		m_IP_Source;
		NetAddressIPv4		m_IP_Dest;
		uint16_t			m_nPort_Source;
		uint16_t			m_nPort_Dest;

		FMNC_Test_Sequence 	* m_pTestSequence;
		FMNC_Test_Analysis 	* m_pTestAnalysis;

		uint32_t			m_lSessionID;

		// The object driving this particular connection
		string				m_sRequest;

		ParamDictionary *	m_pDictionary;

        //For fluctuation analysis
        uint32_t m_lAnalyzed;
};

class FMNC_List_Connections
{
	public:
		FMNC_List_Connections ();

		// Given an existing packet, figure out if we have this connection already
		//  in the list
		FMNC_Connection *	findConnection (Packet * pPacket);


		/// Search the list for old sessions
		FMNC_Connection * 	findConnection (uint32_t lSessionID);

		// Add this connection to the list of connections
		bool	add (FMNC_Connection * pConnection);
        bool  removeConnection(FMNC_Connection*);

		/// Display the list of currently active connections contained within this particular list
		/// to the console
		void	dumpList ();


		bool	populateOldList (uint32_t nOldTime);

		FMNC_Connection *	popOldEntry ();

		void	displayToConsole ();


	private:
		vector<FMNC_Connection *>		m_Connections;

		vector<FMNC_Connection *>		m_OldConnections;

		pthread_mutex_t 				m_MutexList;
};

#endif /* FMNC_CONNECTION_H_ */
