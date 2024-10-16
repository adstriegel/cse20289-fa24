/* NetAddress.h
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

#ifndef __NETADDRESS_H
#define __NETADDRESS_H

#include <string>
using namespace std;

#include <arpa/inet.h>

#include "../../mem/MemPoolObject.h"
#include "../../core/StdHeaders.h"

#define		MAX_ADDRESS_LENGTH		1024

enum NetAddressType {
	NETADDRESS_TYPE_NONE=0,			// Default case
	NETADDRESS_TYPE_ETHERNET,		// 802.3 aka Ethernet, 6 byte MAC
	NETADDRESS_TYPE_IPV4,			// IPv4 - 4 bytes
	NETADDRESS_TYPE_IPV4SUBNET,		// IPv4 Subnet w/Mask
	NETADDRESS_TYPE_IPV6,			// IPv6 - 16 bytes
	NETADDRESS_TYPE_PORT,			// Layer 4 port - 2 bytes
	NETADDRESS_TYPE_DNS				// DNS hostname
};

/** The base class from which all network address objects
 * are derived.  The core form of the network address is
 * simply an array of bytes allocated as appropriate for
 * the child classes
 */
class NetAddress : public MemPoolObject {
	public:
		/** Construct a NetAddress object defining the type of the object and
		 * the global memory pool type 
		 */
		NetAddress (unsigned short nType, unsigned short nPoolType);
		
		/** Clean up and delete the appropriate memory allocations
		 */
		~NetAddress ();
	
		/** Create a new NetAddress or child object by either pulling
		 * an address from the global memory pool or by allocating a
		 * new object. 
		 * @returns A pointer to a valid NetAddress object if successful
		 */
		static  NetAddress *	create ();
		
		/** Release the object
		 */
		void	release ();
		
		/** Clean the object before giving it back to the pool as
		 * appropriate.  Special care should be taken in derived functions
		 * to make this as minimal as possible.
		 */
		virtual void clean ();		
		
		/** Convert the network address to a human readable string
		 * that by default will be a hexadecimal output of the values
		 * @returns A human readable string
		 */
		virtual string 	toString ();
		
		/** Convert the network address to a hexadecimal string without
		 * a leading 0x in front of the hex string
		 * @returns A hexadecimal representation of the string
		 */
		virtual string	toHexString ();
		
		/** Create a debug string containing a full breakdown of the
		 * internal class information
		 * @returns A debug-rich string
		 */
		virtual string	toDebugString ();
		
		/** Set the address based on the incoming string. 
		 * @param sAddress The address to convert
		 * @returns True if sucessful, false otherwise
		 */
		virtual bool	setAddressFromString (string sAddress);
		
		/** Create a new NetAddress object with the input from the
		 * given string.
		 * @param sAddress The address to use (similar to setAddressFromString)
		 * @returns A pointer to a valid NetAddress object if the allocation
		 *  is successful and the address is formatted correctly
		 */
		static NetAddress * createFromString (string sAddress);
		
		/** Retrieve a pointer to block of memory containing the
		 * address. Note that no guarantees are made with regards
		 * to thread safety as this is a raw pointer.
		 * @returns A pointer to the address
		 */
		unsigned char *		getAddress ();
		
		/** Change the address to a new value. The address to copy
		 * in must fall within the existing size allocation in order
		 * to succeed. 
		 * @param pAddr A pointer to a valid block containing the address
		 * @param nSize The size of the address to copy
		 * @returns True if successful, false otherwise
		 */
		bool	setAddress (unsigned char * pAddr, unsigned int nSize);
		
		/** Change the address to a new value and if necessary, allocate
		 * new space subject to the maximum address size (sanity check)
		 * @param pAddr A pointer to a valid block containing the address
		 * @param nSize The size of the address to copy
		 * @returns True if successful, false otherwise
		 */		
		bool	setAddressWithGrow (unsigned char * pAddr, unsigned int nSize);
		
		/** Retrieve the type associated with this class
		 * @returns The type of the address
		 */
		unsigned short	getAddressType ();
		
		/** Copy the contents of this object into an existing
		 * object where the internal byte array is appropriately copied
		 * as necessary
		 * @param rhs Right hand side of the equal sign
		 * @returns The newly copied object
		 */
		NetAddress & operator = (const NetAddress rhs);
		
		/** Compare the two addresses to check for equality.  By default,
		 * equality occurs when the arrays are of the same size and the
		 * contents of the array are identical in a byte-wise sense.
		 * @param rhs Right hand side of the equality (whom we are comparing to)
		 * @returns True if equal, false otherwise
		 */
		virtual bool	operator == (NetAddress rhs);

		/** Compare the address versus a byte array to check for equality.  
		 * The byte array is assumed to be of the unsigned char variety with a
		 * similar construction to the internal byte array unless otherwise
		 * stated. This function is not encouraged but can be signficantly faster
		 * than bringing the other address into a NetAddress object. It is assumed
		 * that the byte array points directly to point to start for comparison.
		 * @warning Note that this function is not buffer safe! While it does not
		 *  write over anything, it can read beyond the edge of an array as it
		 *  will compare byte by byte for the length of the address by default.
		 * @param pComp The array to peek into for comparison
		 * @returns True if equal, false otherwise
		 */
		virtual bool	operator == (unsigned char * pComp);		
		
		/** Compare the address versus a byte array for equality.  See the unsigned
		 * char * base function for more information
		 * @param pComp The byte array to compare
		 * @returns True if equal, false otherwise
		 */
		virtual bool	operator == (char * pComp);
		
		/** Compare the two address to check for inequality.  By default,
		 * equality occurs when the arrays are of the same size and the
		 * contents of the array are identical in a byte-wise sense. Inequality
		 * is simply the inverse of that condition.
		 * @param rhs Right hand side of the inequality (whom we are comparing to)
		 * @returns True if not equal, false otherwise
		 */
		bool	operator != (NetAddress rhs);

		/** Compare the two address to check for a less than relationship.
		 * Comparison begins by default from the left (MSB) to the right and
		 * will be satisfied only if this NetAddress has a lesser value than
		 * the reference / comparison address.  
		 * @param rhs Right hand side of the comparison (whom we are comparing to)
		 * @returns True if less than, false otherwise
		 */
		bool	operator < (NetAddress rhs);

		/** Compare the two address to check for a greater than relationship.
		 * Comparison begins by default from the left (MSB) to the right and
		 * will be satisfied only if this NetAddress has a greater value than
		 * the reference / comparison address.  
		 * @param rhs Right hand side of the comparison (whom we are comparing to)
		 * @returns True if greater than, false otherwise
		 */
		bool	operator > (NetAddress rhs);

		/** Compare the two address to check for a greater than or equal relationship.
		 * Comparison begins by default from the left (MSB) to the right and
		 * will be satisfied only if this NetAddress has a greater value than
		 * or equal value versus the reference / comparison address.  
		 * @param rhs Right hand side of the comparison (whom we are comparing to)
		 * @returns True if greater than or equal to, false otherwise
		 */		
		bool	operator >= (NetAddress rhs);

		/** Compare the two address to check for a less than or equal to relationship.
		 * Comparison begins by default from the left (MSB) to the right and
		 * will be satisfied only if this NetAddress has a lesser or equal value than
		 * the reference / comparison address.  
		 * @param rhs Right hand side of the comparison (whom we are comparing to)
		 * @returns True if less than or equal to, false otherwise
		 */
		bool	operator <= (NetAddress rhs);
		
		/** Retrieve the size of this address
		 * @returns The size of the address (in bytes) 
		 */
		virtual unsigned int	getSize ();		
		
		/** Retrieve the allocated buffer size to this address
		 * @returns The size of the allocated space (in bytes)
		 */
		virtual unsigned int	getAllocatedSize ();
		
	protected:
		/** Check the internal values for normal sanity which for the
		 * basic case means the byte array and its length
		 */ 
		virtual bool		checkIntegrity ();
		
		
		/** Change the type associated with this address in terms
		 * of the derived class.  It is not recommended to call this
		 * function outside of the constructor for the class.  The
		 * preferred method would be to pass the type value to via
		 * the parent constructor (this class).
		 * @param nType	The new type identifier
		 * @returns True if successful (valid type), false otherwise
		 */
		bool	setAddressType (unsigned short nType);
		
	
		/** Allocate the internal byte array to a new
		 * set of bytes, deleting the previous allocation
		 * if it existed.
		 * @param nSize	The new size of the address in bytes
		 * @returns True if successful, false otherwise		
		 */
		bool	allocate (unsigned int nSize);		
	
		/** Grow the size of the internal allocation as
		 * specified.
		 * @param nIncrease The number of bytes to increase
		 * @returns True if successful, false otherwise 
		 */
		bool	grow (unsigned int nIncrease);
		
		/** Convert the internal type into a string describing the
		 * class of the address type
		 * @returns The translated name of the type as a string
		 */
		string	convertTypeToString ();
		
	private:
		unsigned char * m_pAddress;
		unsigned short 	m_nType;		
		unsigned int	m_nSize;
	
};


#endif

