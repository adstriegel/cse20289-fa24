/* MemPoolObject.h
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
 * $Revision: 1.4 $  $Date: 2008/04/11 22:03:19 $
 **********************************************************
 */

#ifndef MEMPOOLOBJECT_H_
#define MEMPOOLOBJECT_H_

#include <string>
using namespace std;

/** The enumerated list of memory pool object types.  All object
 * types should be included here. The tradeoff of using an enum
 * was selected to avoid significant cost on fetching and releasing
 * from/to the global memory pool. 
 */
enum MemPoolType {
	MEMPOOL_OBJ_NONE = -1,			// Unmanaged mem pool object
	MEMPOOL_OBJ_DOMNODE,			// XML DOM Node
	MEMPOOL_OBJ_DOMELEM,			// XML DOM Element
	MEMPOOL_OBJ_PACKET,
	MEMPOOL_OBJ_PACKETGROUP,
	MEMPOOL_OBJ_PKTSIGMD5,
	MEMPOOL_OBJ_PKTSIGXOR,	
	MEMPOOL_OBJ_TIMEREVENT,				// Individual timer event
	MEMPOOL_OBJ_NETADDRESS_BASE,		// Base network address
	MEMPOOL_OBJ_NETADDRESS_ETHERNET,	// Network address - Ethernet
	MEMPOOL_OBJ_NETADDRESS_IPV4,		// Network address - IPv4
	MEMPOOL_OBJ_NETADDRESS_IPV4SUBNET,	// Network address - IPv4 subnet w/mask length
	MEMPOOL_OBJ_LAST				// The last object in the master list
};

// Note that we do not use the enum below to allow for the eventual
// inclusion of a MemPool_Custom.h file that extends the MemPoolType
// enum.  The use of the enum would not allow us to extend the list of
// types for the purpose of separating user extended and the base code. 


/** The base class for objects that can be managed by the
 * memory pool
 */
class MemPoolObject {
	public:
		MemPoolObject ();
		
		/** Construct a new MemPoolObject with a type defined. This is the
		 * recommended constructor although the empty constructor and 
		 * the setType function can be employed
		 * @param nType   The type of the newly constructed object
		 */
		MemPoolObject (int nType);
		
		/** Release an object back into the pool
		 */
		bool            release ();
		
		/** Map the base ID to class type for the memory pool status
		 * dump
		 */
		static string   mapIDtoClassType (int nType);
		
		/** Retrieve the type of the object of this MemPool object.  The 
		 * returned value is a hint to the actual class or base class of the
		 * object itself held in the memory pool.
		 * @returns The type of the memory object in question
		 */
		int             getType ();
		
		/** Set the type of the object.  Note that this function can be
		 * especially hazardous unless it is used solely for initialization.
		 * It is generally recommended to use the constructor rather than
		 * the setType function.
		 * @param newType  The new type for the memory pool object
		 */
		void            setType (int newType);
	
		/** Return a string describing the class info. This is similar to
		 * the Java basic dump that describes the class name. This function
		 * should be overridden in all derived classes.  
		 * @returns A valid string containing the class information
		 */
		virtual	string  getClassInfo ();
				
		/** Clean the object before giving it back to the pool as
		 * appropriate.  Special care should be taken in derived functions
		 * to make this as minimal as possible.
		 */
		virtual void    clean () = 0;

		/** Clean core information associated with the base of the class
		 * that for now only includes ownership information tracking
		 */
		void            cleanBase ();
				
		/** Apply the necessary background changes for removing the
		 * object from the pool.  For the base object, the only change
		 * is marking the flag for the packet as being allocated (in the wild).
		 */
		void	        markAllocated ();
			
		/** Apply the necessary background changes for noting that the
		 * packet is again part of the pool
		 */
		void            markInPool ();	

		/** Check to see if this packet is already in the pool (allocated flag is false)
		 * @return True if already in pool, false otherwise
		 */
		bool            isAlreadyInPool ();
			
		/** Set the owner of this particular object.  It may be NULL
		 * @param New owner of the object
		 */
		void  	        setOwner (void * pOwner);

		/** Get the owner of this particular object.
		 * @returns A pointer to the owner for this particular object
		 */
		void* 	        getOwner ();
			
	private:
		// The type of the memory pool object itself.  The type itself is
		//  drawn from the list of MemPoolType enums above
		int    m_nType;
		
		// Is this memory object allocated, i.e. is it no longer in
		//  the pool?		
		bool   m_bInPool;

		// Memory address of the object / owner that is currently in charge of this
		//  particular object.  If it is NULL, we don't know.  Otherwise, it should
		//  be something valid
		void*  m_pOwner;
};


MemPoolObject * 	allocateManagedMemObject (int nType); 

/** Check to see if the input type falls within a valid range for
 * requesting either additional allocations, release, or an object
 * from the pool 
 * @param nType The type of the object in question
 * @returns 1 (true) if a valid type, 0 (false) otherwise
 */
char		checkValidMemObjectType (int nType);

#endif /*MEMPOOLOBJECT_H_*/
