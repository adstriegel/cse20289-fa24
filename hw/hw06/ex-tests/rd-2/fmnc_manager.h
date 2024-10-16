
/*
   cleanqueue.h

   Worker thread queue based on the Standard C++ library list
   template class.

   ------------------------------------------

   Copyright @ 2013 [Vic Hargrave - http://vichargrave.com]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef __cleanqueue_h__
#define __cleanqueue_h__
#include <stdio.h>
#include <pthread.h>
#include <list>

using namespace std;
class Thread
{
  public:
    Thread();
    virtual ~Thread();

    int start();
    int join();
    int detach();
    pthread_t self();
    
    virtual void* run() = 0;
    
  private:
    pthread_t  m_tid;
    int        m_running;
    int        m_detached;
};

template <typename T> class cleanqueue
{
    list<T>          m_queue;
    pthread_mutex_t  m_mutex;
    pthread_cond_t   m_condv; 

  public:
    cleanqueue() {
        pthread_mutex_init(&m_mutex, NULL);
        pthread_cond_init(&m_condv, NULL);
    }
    ~cleanqueue() {
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_condv);
    }
    void add(T item) {
        pthread_mutex_lock(&m_mutex);
        m_queue.push_back(item);
        pthread_cond_signal(&m_condv);
        pthread_mutex_unlock(&m_mutex);
    }
    T remove() {
        pthread_mutex_lock(&m_mutex);
        while (m_queue.size() == 0) {
            pthread_cond_wait(&m_condv, &m_mutex);
        }
        T item = m_queue.front();
        m_queue.pop_front();
        pthread_mutex_unlock(&m_mutex);
        return item;
    }
    int size() {
        pthread_mutex_lock(&m_mutex);
        int size = m_queue.size();
        pthread_mutex_unlock(&m_mutex);
        return size;
    }
};
class CleanTask
{
    string m_filename;
    string m_xml;
 
  public:
    CleanTask(string f, string s) : m_filename(f), m_xml(s){}
            
    
    ~CleanTask(){}
 
    void clean();
};

class CleanThread : public Thread
{
    cleanqueue<CleanTask*>& m_queue;
  public:
    CleanThread(cleanqueue<CleanTask*>& queue) : m_queue(queue) {}
 
    void* run() {
        // Remove 1 item at a time and process it. Blocks if no items are 
        // available to process.
        for (int i = 0;; i++) {
            printf("thread %lu- waiting for item...\n", 
                  (long unsigned int)self() );
            CleanTask* item = m_queue.remove();
            item->clean();
            printf("thread %lu, - got one item\n", 
                  (long unsigned int)self() );
            /* printf("thread %lu, loop %d - item: message - %s, number - %d\n", */ 
            /*       (long unsigned int)self(), i, item->getMessage(), */ 
            /*        item->getNumber()); */
            delete item;
        }
        return NULL;
    }
};
#endif
/*
 *
 * fmnc_manager.h
 *
 *  Created on: Apr 1, 2014
 *      Author: striegel
 */

#ifndef FMNC_MANAGER_H_
#define FMNC_MANAGER_H_

#include <vector>
#include <sstream>
using namespace std;

#include <string>
using namespace std;

#include <fstream>
using namespace std;

#include "../mem/MemoryPool.h"
#include "../pkt/MultiIFModule.h"
#include "../stat/Stats.h"
#include "../pkt/Packet.h"

#include "../core/Console.h"

#include "fmnc_connection.h"
/* #include "fmnc_measurement_packet.h" */
/* #include "fmnc_measurement_pair.h" */

#define FMNC_DEFAULT_MAX_SESSIONS		100


#define FMNC_DEFAULT_INTERVAL_CLEANUP	5
#define FMNC_DEFAULT_OLD_CONNECTION		10
#define FMNC_DEFAULT_DELAY_FIN			250
#define FMNC_MAX_NUM_FIN_ACK			3
#define FMNC_DEFAULT_TEST_CHOICE		3
#define FMNC_DEFAULT_DELAY_INITIAL		5			// Initial delay of 5 ms

#define FMNC_DEFAULT_SLICE_SIZE				100
#define FMNC_DEFAULT_SLICE_SPACE_SEC		0
#define FMNC_DEFAULT_SLICE_SPACE_MICROSEC	1000		// 1000 microseconds, aka 1 ms



#define FMNC_CONNTYPE_UNKNOWN	0
#define	FMNC_CONNTYPE_NI		1
#define	FMNC_CONNTYPE_WEB		2
#define	FMNC_CONNTYPE_XML		3

#define FMNC_TIMER_START_SLICING	4
#define FMNC_TIMER_DO_SLICING		5
#define FMNC_TIMER_RETRANS	     	8
#define FMNC_TIMER_SEND_FIN			6

/* Finish on signle connection */
#define FMNC_TIMER_FINISH			9
// Garbage collector timer
#define FMNC_TIMER_CLEANUP			7


#define FMNC_WRITE_CALLBACK_MEASUREMENTPKT		20

class TCPServerSocket;
class TCPSocket;
class FMNC_Measurement_Packet_TCP;
class FMNC_Measurement_Pair;
/** Traffic can come in only via one adapter
 */
enum eFMNC_GatewayTaps {
	FMNC_TAP_NONE=0,
	FMNC_TAP_WAN			// Traffic from the Internet side
};

/** Enumerated values for the different types of stats that the RIPPS monitor
 * will keep track of
 */
enum eFMNC_ManagerStats {
	FMNC_MANAGER_TOTAL_COUNT_PKTS,		// How many packets have we received?
	FMNC_MANAGER_TOTAL_VOLUME_PKTS,		// Packet volume

	FMNC_MANAGER_NONIP_COUNT_PKTS,		// Count of non-IP packets
	FMNC_MANAGER_NONIP_VOLUME_PKTS,		// Volume of non-IP packets

	FMNC_MANAGER_IPV6_COUNT_PKTS,
	FMNC_MANAGER_IPV6_VOLUME_PKTS,

	FMNC_MANAGER_FILTER_IP_COUNT_DESTADDR,		// Packets filtered by virtue of destination IP
	FMNC_MANAGER_FILTER_IP_VOLUME_DESTADDR,		// Volume of filtered packets

	FMNC_MANAGER_FILTER_PORT_COUNT_DESTADDR,	// Packets filtered by virtue of destination port
	FMNC_MANAGER_FILTER_PORT_VOLUME_DESTADDR,	// Volume of filtered packets by port / protocol

	FMNC_MANAGER_UNKNOWN_TCP_CONN_COUNT,		// Number of unknown TCP connection packets
	FMNC_MANAGER_KNOWN_TCP_CONN_COUNT,			// Number of known TCP connection packets

	FMNC_MANAGER_KNOWN_TCP_CONN_VOLUME,				// Total volume of packets
	FMNC_MANAGER_KNOWN_TCP_CONN_VOLUME_PAYLOAD,		// Total payload volume

	FMNC_MANAGER_COUNT_SYN,		// Number of received SYN packets

	FMNC_MANAGER_COUNT_UNEXPECTED_SYN, 			// SYN out of order / wrong spot in 3-way handshake

	FMNC_MANAGER_COUNT_HANDSHAKE_ACK_HADPAYLOAD,	// Do we see a payload on the third part of the handshake?
	FMNC_MANAGER_COUNT_HANDSHAKE_NO_ACK,			// Did we not see an ACK on the third part of the handshake?

	FMNC_MANAGER_STAT_LAST
};

/** Statistics for the TWiCE gateway
 */
class FMNC_ManagerStats : public Stats {
	public:
		FMNC_ManagerStats ();

		/** Retrieve the title for a given field
		 * @param nStat		The index to the stat field
		 * @param szTitle		The char field to populate with the title
		 */
		virtual void		Get_Title	(int nStat, char * szTitle);


};

class FMNC_Connection_TCP_Slice;
class FMNC_Test_Sequence;
/* template <typename T> class cleanqueue; */
/* class CleanTask; */
/* class CleanThread; */


class FMNC_Manager : public MultiIFModule {

	public:
		FMNC_Manager();
		~FMNC_Manager ();


		/** Given the name of a tap module, map that module name to a specific
		 * ID that will be used whenever packets are seen on that particular
		 * tap. This function must be overridden to encapsulate the behavior
		 * how the names map to IDs.
		 * @param sName The name of the module
		 * @returns The ID to use for that module where -1 is a failure in name resolution
		 */
		virtual int 	mapNameToID (string sName);

		/** Process a packet from one of the underlying tap modules and
		 * appropriate process the packet to the same behavior as a processPacket
		 * call from a normal IOModule object
		 */
		virtual char	processTapPacket (Packet * pPacket, int nID);

		/** Initialize an input module by applying any necessary
		 * startup functionality such as threads, thread safety, or
		 * other initializations.
		 * @returns True if successful, false otherwise
		 */
		char		initialize ();

		/** Parse the DOM to populate the file where the DOM
		 * is generated from an XML input file. The DOM should contain
		 * the type of adapter and the underlying input and output
		 * modules. The extractExtended function takes in the
		 * extended information specific to the child class.
		 * @param pNode The base node to process for information
		 * @returns True if successful, false otherwise
		 */
		virtual bool	extractExtendedDOM (NodeDOM * pNode);

		/** Process a timer event from the associated TimerAggregator
		 * object contained within this class if timers are enabled
		 * for this particular module
		 * @param pEvent The timer event that has occurred
		 * @returns True if successfully processed, false if there was an error
		 */
		virtual bool		processTimer (TimerEvent * pEvent);

		/** Retrieve a copy of the statistics for this module
		 * @param pRoot	The DOM object serving as the root under which the stats are placed
		 * @returns A valid pointer to a NodeDOM object that serves as the root of
		 *   the statistic contents
		 */
		NodeDOM *		getStats (NodeDOM * pRoot);

		/** Process a command from the console or external control
		 * mechanism
		 * @param theCommands The vector of commands to process
		 * @param nOffset The offset at which to begin processing. The offset allows
		 *   us to pass the vector itself with modifications to the underlying
		 *   modules
		 * @returns Enumerated value representing result of command processing
		 */
		virtual CommandResult	processExtCommand (const vector<string> & theCommands, int nOffset);


		/** Process the fact that a write callback has been invoked due to a
		 * packet finally being written out due to interposed queuing delay
		 * or other mechanisms
		 * @param pPacket The packet involved in the callback
		 * @param pData The data associated with the callback
		 * @returns True if successful, false otherwise
		 */
		virtual bool	processWriteCallback (Packet * pPacket, void * pData);

		char 		processPacket_WAN (Packet * pPacket);

		uint16_t	getPort_NI ();
		void 		setPort_NI (uint16_t nPort);
		uint16_t	getPort_Result ();
		void 		setPort_Result (uint16_t nPort);

		uint16_t	getPort_Web ();
		void		setPort_Web (uint16_t nPort);

		uint16_t	getPort_XML ();
		void		setPort_XML (uint16_t nPort);

		uint32_t	getMaxSessions ();
		void		setMaxSessions (uint32_t nMaxSessions);

		char mapConnectionTypeFromPort (Packet * pPacket);


		/// If we have a new connection of this particular type, should we admit it?
		/// The answer for now is always yes
		char shouldAcceptNewConnection (char byType);

		/// Is this a new connection that we have not seen before?
		char isNewConnection (Packet * pPacket);

		// Handle a TCP packet
		char handleTCP (Packet * pPacket);

		// Handle a UDP packet
		char handleUDP (Packet * pPacket);

		uint32_t	getSequenceNumber (Packet * pPacket);

		char handleTCP_Existing (FMNC_Connection * pConnection, Packet * pPacket);

		/// Retrieve the address associated with the server.  If the address is NULL, it
		/// means that filtering with respect to the destination IP address is not enabled.  Source
		/// address filtering should be enabled via the filter construct (see fmnc.xml in the config)
		/// @returns NULL if no destination address filtering, non-NULL if destination filtering is enabled
		NetAddressIPv4 *	getServerAddress ();

		/// Set the address associated with the server.  If the address is NULL, it
		/// means that filtering with respect to the destination IP address is not enabled.  Source
		/// address filtering should be enabled via the filter construct (see fmnc.xml in the config)
		/// @param pAddress The new address for the server destination filtering (may be NULL)
		void				setServerAddress (NetAddressIPv4 * pAddress);

		/// Set the address associated with the server.  If the address is NULL, it
		/// means that filtering with respect to the destination IP address is not enabled.  Source
		/// address filtering should be enabled via the filter construct (see fmnc.xml in the config)
		/// @param sAddress Blank address implies a NULL address, well-formed dot notation otherwise (ex. 129.74.20.40)
		void				setServerAddress (string sAddress);

		/// Set the interval (in seconds) for the garbage cleanup timer
		/// @param nInterval The new interval (in seconds)
		void				setInterval_Cleanup (uint16_t nInterval);
		uint16_t			getInterval_Cleanup ();

		/// Set the age after which a connection that has not seen any packets will be purged from
		/// the list (in seconds)
		void				setAge_PurgeConnection (uint16_t nAge);
		uint16_t			getAge_PurgeConnection ();


		/// Set the timing delay from the last data packet in the test sequence until the final FIN is sent (by default)
		/// @param lDelay Delay in milliseconds1
		void				setDelay_SendFIN (uint32_t lDelay);
		uint32_t			getDelay_SendFIN ();


		/// Set the before the test sequence is invoked until the action timer fires
		/// @param lDelay Delay in milliseconds1
		void				setDelay_Initial (uint32_t lDelay);
		uint32_t			getDelay_Initial ();


		// Pre-defined tests
		void				setTestChoice (int nTest);
		int					getTestChoice ();

		uint16_t	getSliceSize ();
		void		setSliceSize (uint16_t nSlice);

		void		setSliceSpacing (uint32_t lSpaceSec, uint32_t lSpaceMicroSec);
		uint32_t	getSliceSpacing_Sec ();
		uint32_t	getSliceSpacing_MicroSec ();

		/** Process the fact that a write callback has been invoked due to a
		 * packet finally being written out due to interposed queuing delay
		 * or other mechanisms
		 * @param pPacket The packet involved in the callback
		 * @param pData The data associated with the callback
		 * @param nType The type for the callback (if specified earlier)
		 * @returns True if successful, false otherwise
		 */
		virtual bool	processWriteCallback (Packet * pPacket, void * pData, int nType);


		bool 			sendPacket_FIN (FMNC_Connection_TCP_Slice * pSlicedConnection, FMNC_Test_Sequence *	pTestSequence, bool bFINRespond);
		bool 			sendPacket_RST (FMNC_Connection_TCP_Slice * pSlicedConnection, FMNC_Test_Sequence *	pTestSequence);

		bool 			sendPacket_PureACK (FMNC_Connection_TCP_Slice * pSlicedConnection, uint32_t lSeqNum, uint32_t lAckNum, string sMeta);

	protected:

		/// Create a test sequence from the pre-defined pool
		FMNC_Test_Sequence *	createTestSequence_PreDef (FMNC_Connection_TCP_Slice *	pSlicedConnection);




		/// Check to see if this packet should be filtered out and not considered by the FMNC server
		/// @returns True if the packet should be ignored, false otherwise
		bool	filterDestinationAddress (Packet * pPacket);

		/// Check to see if this packet should be filtered out and not considered by virtue of the requested protocol and / or ports
		/// @returns True if the packet should be ignored, false otherwise
		bool	filterDestinationPort	 (Packet * pPacket);

		bool 	doTimer_StartSlicing (TimerEvent * pEvent);

		bool 	doTimer_Slicing (TimerEvent * pEvent);

		bool 	doTimer_SendFIN (TimerEvent * pEvent);


		bool 	doTimer_Finish (TimerEvent * pEvent);
		bool 	doTimer_Cleanup (TimerEvent * pEvent);
		bool 	doTimer_Retrans (TimerEvent * pEvent);

		string	convertTimerIDtoString (TimerEvent * pEvent);

		uint16_t	getPacketID ();
		void		incrementPacketID ();


		// Does not fix the checksum FYI
		void		stampPacketID	  (Packet * pPacket);

		// Set the TTL
		void		stampTTL 		  (Packet * pPacket);

		void		recomputeChecksums (Packet * pPacket);


		bool		writePacket	 	  (Packet * pPacketToWrite);

		/// Save the information with regards to this particular connection to disk
		bool		saveConnectionInfo	 (FMNC_Connection * pConnection);

	protected:
		void		incrementSessionID ();
		uint32_t	getSessionID();


		/** For the initial web request, determine the object that was requested
		 * assuming that it is a valid HTTP request. The first line will contain
		 * the object being requested.
		 */
		string		getWeb_Request (Packet * pPacket);




	private:
		// What port(s) are we listening on for non-interactive sessions?
		//   A zero signifies this port is disabled
		uint16_t		m_nPort_NI;

		// What port(s) are we listening on for feedback result sessions?
		//   A zero signifies this port is disabled
		uint16_t		m_nPort_Result;
		// What port(s) are we listening on for web-based sessions?
		//   A zero signifies this port is disabled
		uint16_t		m_nPort_Web;

		// What port(s) are we listening on for XML / configurable sessions?
		//   A zero signifies this port is disabled
		uint16_t		m_nPort_XML;

		// Maximum number of concurrent sessions (Default = 100)
		uint32_t		m_nMax_Sessions;

		TCPServerSocket	*	m_pWebSocket;

		// ID of the next packet to be sent (all connections) - IPv4 level
		uint16_t		m_nPacketID;

		// IP address for the server - default is NULL which means no filtering
		NetAddressIPv4 *	m_pServerAddress;

		// What is the interval for the garbage collector (cleanup) - in seconds?
		uint16_t			m_nInterval_Cleanup;

		// What is the age at which a connection is considered old (in seconds)?
		uint16_t			m_nOld_Connection;

		// How long to wait between the final FIN and the last sliced packet (in ms)?
		uint32_t			m_nDelay_SendFIN;

		// How long to wait initially before starting the test sequence?
		uint32_t			m_nDelay_Initial;

		FMNC_ManagerStats	m_Stats;

		FMNC_List_Connections	m_Connections;

		// The session ID provides a unique identifier for each individual measurement session
		uint32_t			m_lSessionID;

		TimerEvent *		m_pTimer_Cleanup;

		// What is the default test (when unspecified)?
		int					m_nTestChoice;


		uint16_t		m_nSliceSize;

		// What is the typical spacing between packets?
		uint32_t		m_nSliceSpace_Sec;
		uint32_t		m_nSliceSpace_MicroSec;
        // clean up queue
        cleanqueue<CleanTask*> m_CleanQueue;
        CleanThread* m_CleanThread;

};

#endif /* FMNC_MANAGER_H_ */
