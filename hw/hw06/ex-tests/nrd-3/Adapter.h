#ifndef ADAPTER_H_
#define ADAPTER_H_

#include <vector>
using namespace std;

#include <string>
using namespace std;

#include <deque>
using namespace std;

#include <pthread.h>
#include <stdlib.h>

#include "../core/StdHeaders.h"

#include "../core/Console.h"

// Definitions to avoid header file dependencies
class InputModule;
class OutputModule;
class IOModule;
class Packet;
class NodeDOM;
class PktQueue;


#define		DEFAULT_ADAPTER_MAXQUEUE		1000


/** An adapter in the system that can read and write packets. Appropriate
 * functionality with regards to packet input should be derived from this
 * base class.
 * 
 * Each adapter has three threads running with regards to reading, processing,
 * and writing packets.  The threads can be thought of in a producer/consumer
 * fashion in that the read thread takes incoming packets from the adapter
 * into a queue, the process thread polls that queue for new information
 * and passes it to the chain of input modules, and the output thread processes
 * the outbound queue and passes it to the chain of output modules.
 */
class Adapter {
	public:
		Adapter ();
		
		~Adapter();
		
		/** Set the flag to denote the thread should keep running. The
		 * threads associated with the adapter will exit at their earliest
		 * convenience.  
		 */
		void		stopRunning ();
		
		/** Return the state of the keep running flag
		 * @returns True (non-zero) if should keep running, false (zero) otherwise
		 */
		char		shouldKeepRunning ();
		
		/** Parse the DOM to populate the file where the DOM
		 * is generated from an XML input file. The DOM should contain
		 * the type of adapter and the underlying input and output
		 * modules. The extractBaseXML function takes in the most
		 * basic input that is central to all adapters.  The virtualized
		 * function of extractExtendedDOM is for child-specific 
		 * information. 
		 * @param pNode The base node to process for information
		 * @returns True if successful, false otherwise
		 */
		bool		extractBaseDOM  (NodeDOM * pNode);

		/** Parse the DOM to populate the file where the DOM
		 * is generated from an XML input file. The DOM should contain
		 * the type of adapter and the underlying input and output
		 * modules. The extractExtended function takes in the 
		 * extended information specific to the child class. 
		 * @param pNode The base node to process for information
		 * @returns True if successful, false otherwise
		 */		
		virtual bool		extractExtendedDOM (NodeDOM * pNode)=0;
		
		/** Input modules are appended to the end of the chain as
		 * appropriate. The function will either replace the root of the
		 * chain or append appropriately
		 * @param pModule A valid (non-NULL) input module
		 */
		void			appendInputModule (IOModule * pModule);

		/** Output modules are appended to the beginning of the chain as
		 * appropriate. The function will either replace the root of the
		 * chain or append appropriately
		 * @param pModule A valid (non-NULL) output module
		 */		
		void			appendOutputModule (IOModule * pModule);
	
		/** Read a packet and give it to the input chain
		 */
		virtual void			readPacket () =0;
		
		/** Write this specific packet to the adapter itself for
		 * direct output onto the network medium.  As the base case has
		 * no real physical medium, the packet will simply be gobbled by
		 * the base adapter function (return to memory pool).
		 * @param pPacket The packet to add to the queue for output
		 */
		virtual void			writePacket (Packet * pPacket);

		/** Set the flag to denote if the adapter is enabled with regards to
		 * reading and writing packets.  The threads will simply sit and idle
		 * via sleep if the enabled flag is set to false. Note that any packets
		 * waiting to be sent will not be sent as the effect on both read and
		 * write threads will impact any subsequent packets not already being
		 * read or written
		 * @param bFlag Set whether or not processing is enabled (true or false)
		 */
		void			setFlagEnabled (char bFlag);
		
		/** Retrieve the flag denoting if the adapter should continue to process
		 * inbound and outbound packets
		 * @returns 1 if the adapter should process, 0 otherwise
		 */
		char			getFlagEnabled ();
		
		/** Retrieve the nominal name of the adapter.  Uniqueness of names is
		 * not enforced but the usage of names can be helpful for logging and
		 * debugging of underlying issues
		 * @returns A valid string object containing the name of the adapter
		 */
		string		getName			();
		
		/** Set the nominal name of the adapter.  See getName for additional
		 * commentary regarding names.
		 * @param sName The new nominal name of the adapter
		 */
		void			setName			(string sName);
	
		/** Add a packet for processing by the input modules in a thread-safe
		 * manner. Packets are then extracted from the input queue for the
		 * chain of input modules by the processing thread.  A NULL input will
		 * result in the return value as false.
		 * @param pPacket The packet to add to the input queue
		 * @returns True if successful, false otherwise
		 */
		bool			addInputQueue	(Packet * pPacket);
	
		/** Add a packet for processing by the output modules in a thread-safe
		 * manner. Packets are then extracted from the output queue for the
		 * chain of output modules by the output thread.  A NULL input will
		 * result in the return value as false. All packets placed in the 
		 * output queue should eventually be written to the adapter unless
		 * squelched by one of the output modules.
		 * @param pPacket The packet to add to the output queue
		 * @returns True if successful, false otherwise
		 */
		bool			addOutputQueue	(Packet * pPacket);
	
	
		/** Return the number of packets currently sitting in the input
		 * queue
		 * @returns Integer denoting the number of packets in the queue
		 */
		int			getInputQueueSize ();

		/** Return the number of packets currently sitting in the output
		 * queue
		 * @returns Integer denoting the number of packets in the queue
		 */
		int			getOutputQueueSize ();
		
		/** Set the maximum input queue size in a thread-safe manner. Note
		 * that this function will not purge existing packets in the queue
		 * if the new queue size creates an overflow state.  In such a case,
		 * new packets will be prevented from being added to the queue until
		 * the queue itself drains.
		 * @param nSize A positive integer denoting the new maximum queue size
		 *  for the input queue
		 * @returns 1 (true) if successful, 0 (false) otherwise
		 * 
		 */
		char			setMaxInputQueueSize (int nSize);

		/** Set the maximum output queue size in a thread-safe manner. Note
		 * that this function will not purge existing packets in the queue
		 * if the new queue size creates an overflow state.  In such a case,
		 * new packets will be prevented from being added to the queue until
		 * the queue itself drains.
		 * @param nSize A positive integer denoting the new maximum queue size
		 *  for the output queue
		 * @returns 1 (true) if successful, 0 (false) otherwise
		 */
		char			setMaxOutputQueueSize (int nSize);

		
		/** Pop the first packet from the input queue. Note that the callee
		 * effectively owns the returned packet if it is non-NULL and is 
		 * responsible for correct memory management.  
		 * @returns NULL if the queue is empty, a non-NULL pointer to a Packet object otherwise
		 */
		Packet *		popInputQueue ();

		/** Pop the first packet from the output queue. Note that the callee
		 * effectively owns the returned packet if it is non-NULL and is 
		 * responsible for correct memory management.  
		 * @returns NULL if the queue is empty, a non-NULL pointer to a Packet object otherwise
		 */
		Packet * 	popOutputQueue ();

		/** Pop the first packet from the input priority queue. Note that the callee
		 * effectively owns the returned packet if it is non-NULL and is 
		 * responsible for correct memory management.  
		 * @returns NULL if the queue is empty, a non-NULL pointer to a Packet object otherwise
		 */
		Packet *		popInputPriorityQueue ();

		/** Pop the first packet from the output priority queue. Note that the callee
		 * effectively owns the returned packet if it is non-NULL and is 
		 * responsible for correct memory management.  
		 * @returns NULL if the queue is empty, a non-NULL pointer to a Packet object otherwise
		 */
		Packet * 	popOutputPriorityQueue ();

		/** Give a packet to the input chain for processing.  The packet will either
		 *  start from the beginning of the chain (default) or resume the chain
		 *  where it left off
		 * @param pPacket The packet to process
		 * @returns 1 if the chain gobbled the packet or 0 if the packet is
		 *   still the responsibility of the callee.  A return value of 0 is
		 *   the most likely result.  
		 */
		char			doInputChain (Packet * pPacket);

		/** Give a packet to the output chain for processing.  The packet will either
		 *  start from the beginning of the chain (default) or resume the chain
		 *  where it left off
		 * @param pPacket The packet to process
		 * @returns 1 if the chain gobbled the packet or 0 if the packet is
		 *   still the responsibility of the callee.  A return value of 0 is
		 *   the most likely result.  
		 */
		char			doOutputChain (Packet * pPacket);

		/** Start up the device.  Nothing in the base class but operations
		 * as necessary (buffering, etc) for derived classes 
		 */
		virtual void			startDevice ();

		/** Start all of the modules associated with this adapter
		 */
		bool			startModules ();

		/** Dump the basic information to the console 
		 */
		virtual void			dumpBasicInfo  ();

		/** Dump the statistics for the specified module to the console
		 * @param sName The module to dump statistics for
		 */
		virtual void			dumpStats (string sName);

		/** Dump the listing of installed modules to the console
		  */
		void			listModules ();

		/** Process a command from the console or external control
		 * mechanism.  If the command is not understood, it will be passed to the
		 * processExtCommand virtual function that can be overridden by the
		 * derived classes. The command functionality is referenced by specifying
		 * the exact module name or module chain of an adapter followed by a
		 * set of arguments separated by spaces.  
		 * 
		 * Show the stats for the pcap-eth1 adapter on the input chain:
		 * 
		 *   pcap-eth1 ic show stats
		 * 
		 * @param pCommands The vector of commands to process
		 * @param nOffset The offset at which to begin processing. The offset allows
		 *   us to pass the vector itself with modifications to the underlying
		 *   modules
		 * @returns Enumerated value representing result of command processing
		 */ 
		CommandResult	processCommand (const vector<string> & theCommands, int nOffset);
		
		/** Process a command from the console or external control
		 * mechanism
		 * @param theCommands The vector of commands to process
		 * @param nOffset The offset at which to begin processing. The offset allows
		 *   us to pass the vector itself with modifications to the underlying
		 *   modules
		 * @returns Enumerated value representing result of command processing
		 */ 
		virtual CommandResult	processExtCommand (const vector<string> & theCommands, int nOffset);

		/** Gather statistics for this particular adapter and its input and
		 * output chains using the provided DOM node as a parent under which
		 * to place all data
		 * @param pParent The DOM node under which to place stats for this adapter
		 */
		bool gatherStats (NodeDOM * pParent);

		/** Gather extended statistics (typically adapter-centric stats)
		 * @param pParent The DOM node under which to place stats for this adapter
		 * @returns True if successful, false otherwise
		 */
		virtual bool gatherStatsExt (NodeDOM * pParent);

	protected:
		/** Does the name of this module match the name requested in the search?
		 * @param sSearch The search string
		 * @returns True if an exact match, false otherwise
		 */
		bool				matchName (string sSearch);	
	
		/** Give a packet to the output chain for processing
		 * @param pPacket The packet to process
		 * @returns 1 if the chain gobbled the packet or 0 if the packet is
		 *   still the responsibility of the callee.  A return value of 0 is
		 *   the most likely result.  
		 */	
		char			startOutputChain (Packet * pPacket);
	
		/** Retrieve a pointer to the input thread
		 * @returns A non-NULL pointer to the input thread
		 */
		pthread_t *		getThreadInput ();
		
		/** Retrieve a pointer to the processing thread
		 * @returns A non-NULL pointer to the processing thread
		 */
		pthread_t *		getThreadProcess ();		
		
		/** Retrieve a pointer to the output thread
		 * @returns A non-NULL pointer to the output thread
		 */
		pthread_t *		getThreadOutput ();
		
		/** Initialize the various mutexes and start up the threads. Note that
		 * the various threads do not actually start doing anything until the
		 * enabled flag is turned on.
		 */
		void				startThreads();
		
		/** Retrieve a pointer to the input mutex
		 * @returns A non-NULL pointer to the input mutex
		 */
		pthread_mutex_t *		getMutexInput ();
		
		/** Retrieve a pointer to the output mutex
		 * @returns A non-NULL pointer to the output mutex
		 */
		pthread_mutex_t *		getMutexOutput ();		
		
		/** Retrieve a pointer to the input priority queue mutex
		 * @returns A non-NULL pointer to the input mutex
		 */
		pthread_mutex_t *		getMutexInputPriority ();
		
		/** Retrieve a pointer to the output priority queue mutex
		 * @returns A non-NULL pointer to the output mutex
		 */
		pthread_mutex_t *		getMutexOutputPriority ();			
			
	private:
		/** Flag - Keep the underlying threads running? Each adapter has two
		 * associated threads to monitor both incoming and outgoing packets.
		 * By default, the flag will be set to true (keep running).
		 */
		char		m_bKeepRunning;	
		
		/** Flag - Denotes if the device is enabled regarding whether or not
		 * the underlying adapter threads will be processing packets. By default,
		 * the adapter will be disabled until enabled by the code.  
		 */
		char		m_bEnabled;
	
		/** A listing of the input modules for the adapter. Upon receipt of
		 * a packet, each module in the chain may see the packet subject to
		 * earlier modules in the list gobbling or consuming the packet. For
		 * instance, an ARP module might gobble an ARP response that does
		 * not need to be seen by other modules. 
		 */
		vector<IOModule *> 	m_InputModuleList;

		/** A listing of the output modules for the adapter. Upon receipt of
		 * a packet, each module in the chain may see the packet subject to
		 * earlier modules in the list gobbling or consuming the packet. For
		 * instance, an ARP module might gobble an ARP response that does
		 * not need to be seen by other modules. Output modules are a bit
		 * more rare than input modules as most output grooming is done by
		 * the process thread for the adapter in the input module. 
		 */
		vector<IOModule *> 	m_OutputModuleList;
	
		/** The queue of outbound packets in FIFO order 
		 */ 
		deque<Packet *>	m_OutPackets;
		
		/** The queue of outbound packets for the adapter */
		PktQueue *			m_pOutQueue;
				
		/** The queue of inbound packets, almost always FIFO with no restrictions
		 */
		PktQueue * 			m_pInQueue;

		/** The queue of inbound packets with priority over normal packets. The
		 * typical role of the priority queue is for module generated packets that
		 * do not flow the normal order of packets but still desire to be routed
		 * across the normal input chain.  Priority packets will often have
		 * resume module values already pre-set.
		 */
		PktQueue * 			m_pInPriorityQueue;

		/** The queue of outbound packets with priority over normal packets. The
		 * typical role of the priority queue is for module generated packets that
		 * do not flow the normal order of packets but still desire to be routed
		 * across the normal input chain.  Priority packets will often have
		 * resume module values already pre-set.
		 */
		PktQueue * 			m_pOutPriorityQueue;


		
		/** The queue of packets as read by the input thread in FIFO order
		 */
		deque<Packet *> 	m_InPackets;

		/** The queue of outbound packets in FIFO order 
		 */ 
		deque<Packet *>	m_OutPriorityPackets;
		
		/** The queue of packets as read by the input thread in FIFO order
		 */
		deque<Packet *> 	m_InPriorityPackets;

		
		/** Maximum input queue size 
		 */
		int			m_nMaxInQueueSize;
		
		/** Maximum output queue size
		 */
		int			m_nMaxOutQueueSize;


		/** The mutex governing the output queue 
		 */
		pthread_mutex_t	m_MutexOutQueue;

		/** The mutex governing the input queue 
		 */
		pthread_mutex_t	m_MutexInQueue;

		/** The mutex governing the input priority queue. Priority queue packets
		 * allow different modules in the change to preserve their desired ordering
		 * versus simply pushing to the front of the queue.
		 */
		pthread_mutex_t	m_MutexInPriorityQueue;

		/** The mutex governing the output priority queue. Priority queue packets
		 * allow different modules in the change to preserve their desired ordering
		 * versus simply pushing to the front of the queue.
		 */
		pthread_mutex_t	m_MutexOutPriorityQueue;

				
		/** Thread for the output mechanisms (simple queue) */
		pthread_t		m_ThreadOutput;

		/** Thread for input mechanism (simple consumer of input queue, 
		 * producer for output queue) */
		pthread_t		m_ThreadProcess;	
		
		/** Thread for input mechanism (simple consumer of output queue) */
		pthread_t		m_ThreadInput;	
		
		/** The nominal name of the adapter.  Names do not have to be unique
		 * as there is no specific search functionality in the overall framework
		 */
		string			m_sName;
		
		
};


#endif /*ADAPTER_H_*/
