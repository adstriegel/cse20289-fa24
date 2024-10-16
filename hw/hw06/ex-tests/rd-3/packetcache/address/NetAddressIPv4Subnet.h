/* NetAddressIPv4Subnet.h
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

#ifndef __NETADDRESS_IPV4SUBNET_H_
#define __NETADDRESS_IPV4SUBNET_H_

#include "NetAddress.h"
#include "NetAddressIPv4.h"

// Default to a /24 address space
#define ADDRESS_IPV4SUBNET_MASK_DEFAULT			24

/** A simple extension of the NetAddress class for IPv4
 * based addressing that simply overrides the standard
 * output functionality but is otherwise nearly identical
 * to the base class to capture a subnet.
 */
class NetAddressIPv4Subnet : public NetAddress {
	public:
		NetAddressIPv4Subnet ();

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
		static  NetAddressIPv4Subnet *	create ();		
		
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
		 * 129.74.20.0/24 as with standard IPv4 subnet listings. Host names
		 * are not acceptable as an input.  
		 * @param sAddress The address to convert
		 * @returns True if sucessful, false otherwise
		 */
		virtual bool	setAddressFromString (string sAddress);			
		
		/** Create a new NetAddressIPv4Subnet object with the input from the
		 * given string. See setAddressFromString for the appropriate format
		 * requirements for the address string.
		 * @param sAddress The address to use (similar to setAddressFromString)
		 * @returns A pointer to a valid NetAddress object if the allocation
		 *  is successful and the address is formatted correctly
		 */
		static NetAddressIPv4Subnet * createFromString (string sAddress);		
		
		/** Retrieve the length of the bitmask for this particular
		 * subnet
		 * @returns The length of the subnet mask in bits
		 */
		unsigned char		getMaskLength ();

		/** Set the length of the bitmask for this particular subnet
		 * @param nMask The new length of the mask (in bits)
		 * @returns True if successful, false otherwise
		 */
		bool		setMaskLength (unsigned char nMask);
		
	private:
		unsigned char		m_nMaskLength;		
};


#endif
