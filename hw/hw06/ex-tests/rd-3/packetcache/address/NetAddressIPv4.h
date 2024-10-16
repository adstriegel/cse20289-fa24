/* NetAddressIPv4.h
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

#ifndef __NETADDRESS_IPV4_H_
#define __NETADDRESS_IPV4_H_

#include "NetAddress.h"

#define ADDRESS_IPV4_SIZE		4

/** A simple extension of the NetAddress class for IPv4
 * based addressing that simply overrides the standard
 * output functionality but is otherwise nearly identical
 * to the base class
 */

class NetAddressIPv4 : public NetAddress {
	public:
		NetAddressIPv4 ();

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
		static  NetAddressIPv4 *	create ();		
		
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
		 * should be in the form of dot based notation such as
		 * 129.74.20.40 as with standard IP listings. Host names
		 * are not acceptable as an input.  
		 * @param sAddress The address to convert
		 * @returns True if sucessful, false otherwise
		 */
		virtual bool	setAddressFromString (string sAddress);			
		
		/** Create a new NetAddressIPv4 object with the input from the
		 * given string. See setAddressFromString for the appropriate format
		 * requirements for the address string.
		 * @param sAddress The address to use (similar to setAddressFromString)
		 * @returns A pointer to a valid NetAddress object if the allocation
		 *  is successful and the address is formatted correctly
		 */
		static NetAddressIPv4 * createFromString (string sAddress);		
		
		/** Is the address a broadcast address in any
		 * sense per the IETF definition? Loosely speaking,
		 * we define an address as being all 1's from the
		 * right side with a minimum of a full byte. 
		 * @returns True if the address is a broadcast address,
		 *    false otherwise
		 */
		bool	isBroadcast ();

		/** Is the address a multicast address in any
		 * sense per the IETF definition (non-SSM variants).
		 * Informally, the addresses between the 224 space and
		 * the 255 space are considered multicast by this
		 * function.   
		 * @returns True if the address is a multicast address,
		 *    false otherwise
		 */
		bool	isMulticast ();
		
		/** Is the address part of the defined private network
		 * space (192, 168 net or 10 net)? 
		 * @returns True if the address is in the private space,
		 *   false otherwise
		 */
		bool	isPrivateAddress ();
		
	private:
		// Nothing in the private space
};


#endif
