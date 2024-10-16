/* PacketCacheModule.h 
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
 * $Revision: 1.5 $  $Date: 2008/04/08 19:56:59 $
 **********************************************************
 */

#ifndef PACKETCACHEMODULE_H_
#define PACKETCACHEMODULE_H_

#include "../pkt/IOModule.h"
//#include "../pkt/Packet.h"
#include "PacketCacheTable.h"

#include <vector>
using namespace std;



/** An I/O module that builds/maintains a cache table for a given adapter stream (input or output)
 * and then relays the packet to the target module while noting the
 * interface and adapter that the module itself came from.  
 */
class PacketCacheModule : public IOModule {
	public:
		PacketCacheModule ();
		~PacketCacheModule ();

		/** Initialize a module by applying any necessary 
		 * startup functionality such as threads, thread safety, or 
		 * other initializations. 
		 * @returns True if successful, false otherwise
		 */
		char		initialize ();	

		/** Process a packet coming along the input chain. Appropriately
		 * process the packet and/or create packets for output based on this
		 * packet. The actual chaining is taken care of at the Adapter level
		 * when the input chain is started upon reception of a packet.
		 * @param pPacket A valid Packet object for processing
		 * @returns 1 if this function has gobbled it (no more processing) or
		 *          0 if the packet should be processed by future modules
		 */ 
		char		processPacket (Packet * pPacket);
				
		/** Parse the DOM to populate the file where the DOM
		 * is generated from an XML input file. The DOM should contain
		 * the type of adapter and the underlying input and output
		 * modules. The extractExtended function takes in the 
		 * extended information specific to the child class. 
		 * @param pNode The base node to process for information
		 * @returns True if successful, false otherwise
		 */		
		bool		extractExtendedDOM (NodeDOM * pNode);
		
		/** Retrieve the unique ID (within the context of the overarching module)
		 * to identify this specific tap
		 * @returns A valid integer value unique for a tap within a given module
		 */
		string		getCacheID ();

		/** Set the unique ID for this tap within the context of a module
		 * @param nID The new ID for the tap conveyed when packets are
		 *  			  passed up to the overarching module
		 */		
		void		setCacheID (string nID);		

		
	private:	
		/** The identifier displayed to the screen for this specific module.  By default, the 
		 * identifier will be PktCache.
		 */
		string					m_sCacheID;

		PacketCacheTable m_pktCacheTable;

		/** The number of packets processed by this module. 
		 *This field can be used to record stats and compute hit rate.
		 */
		unsigned long			m_lNumPkts;
		
		// m_lNumHits / m_lNumPkts = hit rate
		/** The number of packets hitting the cache table.  */
		unsigned long			m_lNumHits;

		/** The total bytes of all processed packets. */
		unsigned long			m_lTotalBytes;

		// m_lHitBytes/m_lTotalBytes = bandwidth saved via using the MD5 cache table
		/** the total bytes of packets hitting the cache table.  */ 
		unsigned long			m_lHitBytes;
		/** the total bytes match the rabin fingerprint cache table.
                 *  Note: the matched bytes of one pkt may be less than the whole pkt size.
                 */ 
		unsigned long			m_lMatchBytes;

		 //instant received / matched bytes in one interval (eg every 100 packets)
		unsigned long                   m_lInstRecvBytes;
		unsigned long                   m_lInstMatchBytes;
};


#endif /*PACKETCACHEMODULE_H_*/
