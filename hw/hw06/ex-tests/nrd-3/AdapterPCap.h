/* AdapterPCap.h
 **********************************************************
 * This code is part of the Scalability in a Box (ScaleBox)
 * system developed at the University of Notre Dame. 
 * 
 * For additional information, please visit the NetScale
 * laboratory webpage at:
 *    http://gipse.cse.nd.edu/NetScale
 *
 * PI:       Dr. Aaron Striegel    striegel@nd.edu
 * 
 * Students: Dave Salyers (lead)   Yingxin Jiang
 *           Xiaolong Li           Jeff Smith
 * 
 **********************************************************
 * $Revision: 1.1.1.1 $  $Date: 2008/02/13 04:21:21 $
 **********************************************************
 */

#ifndef ADAPTERPCAP_H_
#define ADAPTERPCAP_H_

#include <pcap.h>
#include <pthread.h>
#include "Adapter.h"
#include "../util/ip-utils.h"
#include "../util/net-const.h"		// Various network constants

class Packet;

/** The maximum length of a pcap device name */
#define 	PCAP_DEVICE_NAME_LEN		21

/** A class derived from the base Adapter class to encapsulate
 * packets read and written by the lipcap library. Read functionality
 * is provided by the libpcap callback function (one at a time) while
 * write functionality is provided by writing to the raw libpcap
 * file handle.  
 */
class AdapterPCap : public Adapter {
	public:
		AdapterPCap ();
		~AdapterPCap ();

		/** Dump the basic information to the console 
		 */		
		virtual void			dumpBasicInfo  ();
		
		/** Parse the DOM to populate the file where the DOM
		 * is generated from an XML input file. The DOM should contain
		 * the type of adapter and the underlying input and output
		 * modules. The extractExtended function takes in the 
		 * extended information specific to the child class. 
		 * @param pNode The base node to process for information
		 * @returns True if successful, false otherwise
		 */		
		bool		extractExtendedDOM (NodeDOM * pNode);


		/** Read a packet and give it to the input chain
		 */
		void			readPacket ();		
			
		/** Update the packet currently being read in by the read
		 * thread.  This function is used solely for the purpose of
		 * callback from the libpcap callback function as there is only
		 * one argument passed in.
		 */
		void			setReadPacket (Packet * pPacket);
	
		/** Set the MAC address associated with this adapter
		 * @param pMAC Pointer to a valid MAC address
		 */
		void		setMAC (char * pMAC);
		
		/** Retrieve the MAC address associated with this adapter
		 * @returns A valid pointer to the MAC address for this adapter
		 */		
		char *	getMAC ();

		/** Retrieve the device name in the libpcap sense that this adapter
		 * is attached to 
		 * @returns A non-NULL pointer to the beginning of the zero-terminated
		 *  string 
		 */
		char *	getDevName ();
		
		/** Set the device name that will be used by libpcap in the openDevice
		 * function. Device names roughly follow the syntax of the ifconfig
		 * command (eth0, bge0, etc).  
		 * @param pDevName The name as a zero (null char) terminated string
		 */
		void	setDevName (char * pDevName);

		/** Set the device name that will be used by libpcap in the openDevice
		 * function. Device names roughly follow the syntax of the ifconfig
		 * command (eth0, bge0, etc).  
		 * @param pDevName The name as a C++ string object
		 */		
		void	setDevName (string sDevName);
		
		/** Attempt to open the libpcap device for use in this adapter. A failure
		 * will cause the system to exit.  The function will both initialize the
		 * libpcap device as well as start the read and write threads for the 
		 * adapter as well. 
		 */
		int 	openDevice ();

		/** Start up the device as called from the parent class.  The function
		 * simply calls openDevice to start up the threads for the class
		 */
		virtual void			startDevice ();
		
		/** Write this specific packet to the adapter itself for
		 * direct output onto the network medium
		 * @param pPacket The packet to add to the queue for output
		 */
		virtual void			writePacket (Packet * pPacket);
		
		
		/** Set the flag denoting if pcap should do a batch read or do
		 * an individual read
		 * @param bVal The new value for the flag (1=Yes, 0=No)
		 */
		void				setFlag_BatchRead (char bVal);

		/** Retrieve the flag denoting if pcap should do a batch read
		 * or do an individual read
		 * @returns 1 (True) if the read should be done in a batch manner,
		 *    0 otherwise 
		 */		
		char				getFlag_BatchRead ();
		 		
	private:
		/** String identifying libpcap specific interface. Examples
		 * include eth0, bge0, and others.  The m_szDevName should be
		 * a valid interface such as seen by ifconfig.
		 */
		char		m_szDevName[PCAP_DEVICE_NAME_LEN];
		
		/** The pcap device */
		pcap_t	*		m_pDevice;			
		
		/** The temporary holding location for a single packet as read
		 * by the callback function. The callback function only allows a
		 * single dynamic argument.  It is passed the adapter object and this
		 * placeholder is used to record the packet read by the callback
		 * function.
		 */
		Packet *			m_pReadPacket;
		
		/** MAC address (6 bytes) of the adapter */
		char			m_byMAC[ETH_MAC_LENGTH];

		/** IP address of the adapter */		
		char			m_byIP[MAX_IP_LENGTH];
		
		/** Length of the IP address for the adapter (4 for IPv4, 16 for IPv6) */
		int			m_nIPLength;	
		
		/** Batch read via libpcap or per-packet reads, default value is true */
		char			m_bBatchRead;			
};

#endif /*ADAPTERPCAP_H_*/
