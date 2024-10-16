/* NetAddressEthernet.h
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
 * Development and equipment support has been provided in
 *  part by the following sponsors:
 * 
 *    National Science Foundation    CNS03-47392
 *    Sun Microsystems Academic Excellence Grant
 *    Intel IXP Development Grant 
 *    University of Notre Dame Faculty Grant
 * 
 **********************************************************
 */

#ifndef __NETADDRESS_ETHERNET_H_
#define __NETADDRESS_ETHERNET_H_

#include "NetAddress.h"

#define ADDRESS_ETHERNET_SIZE		6


/** A simple extension of the NetAddress class for Ethernet
 * based addressing that simply overrides the standard
 * output functionality but is otherwise nearly identical
 * to the base class
 */
class NetAddressEthernet : public NetAddress {
	public:
		NetAddressEthernet ();
	
		/** Clean the object before giving it back to the pool as
		 * appropriate.  Special care should be taken in derived functions
		 * to make this as minimal as possible.
		 */
		void clean ();			
		
		/** Create a new NetAddress or child object by either pulling
		 * an address from the global memory pool or by allocating a
		 * new object. 
		 * @returns A pointer to a valid NetAddress object if successful
		 */
		static  NetAddressEthernet *	create ();		
		
		/** Convert the network address to a human readable string
		 * that by default will be a hexadecimal output of the values
		 * @returns A human readable string
		 */
		virtual string 	toString ();
		
		/** Create a debug string containing a full breakdown of the
		 * internal class information
		 * @returns A debug-rich string
		 */
		virtual string	toDebugString ();

		/** Set the address based on the incoming string. The string
		 * should be in the form of colon based notation such as
		 * 00:FA:04:56:43:12 as with standard MAC address listing. The
		 * address may also be listed without any colons with the
		 * MAC address as a continuous string, 00FA04564312.  
		 * @param sAddress The address to convert
		 * @returns True if sucessful, false otherwise
		 */
		virtual bool	setAddressFromString (string sAddress);		
		
		/** Create a new NetAddressEthernet object with the input from the
		 * given string. See setAddressFromString for the appropriate format
		 * requirements for the address string.
		 * @param sAddress The address to use (similar to setAddressFromString)
		 * @returns A pointer to a valid NetAddress object if the allocation
		 *  is successful and the address is formatted correctly
		 */
		static NetAddressEthernet * createFromString (string sAddress);		
		
		/** Is the address a broadcast address in any
		 * sense per the IETF definition? Loosely speaking,
		 * we define an address as being all 1's from the
		 * right side with a minimum of a full byte. 
		 * @returns True if the address is a broadcast address,
		 *    false otherwise
		 */
		bool	isBroadcast ();
		
	private:
		// Nothing in the private space
};


#endif
