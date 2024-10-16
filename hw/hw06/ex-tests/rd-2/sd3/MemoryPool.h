#ifndef MEMORYPOOL_H_
#define MEMORYPOOL_H_

#include <vector>
using namespace std;

#include <deque>
using namespace std;

#include <pthread.h>
#include <stdio.h>
#include "MemPoolObject.h"			// For the base MemPoolObject type
#include "MemPoolCustom.h"			// For the customized user types of MemPoolObject

#include "../core/Console.h"

// Default maximum number of objects

#define MEMPOOL_DEFAULT_MAXOBJECTS		4000000



/** A pool of objects controlled by a central location to allow 
 * for easy sharing by the multiple threads without the heavy
 * cost of new and delete operations. The core of the MemoryPool
 * comes from the MemPoolObject instances.  All classes that wish
 * to be included in the memory pool should include the MemPoolObject
 * as their base class.  
 * 
 * Each sub-pool of objects is guarded by a separate pthread mutex
 * with regards to releasing or requesting pooled objects. A separate
 * growth array is kept that will allocate a block of new objects if the
 * current pool is exhausted. Each pool also has a maximum size to
 * prevent runaway allocations from allocating too much memory. 
 */
class MemoryPool {
	public:
		MemoryPool ();
		
		~MemoryPool ();
	
		/** Check the memory pool to retrieve an instance of an object
		 * and if applicable, allocate a new one for the pool
		 * @param nType A valid object type value
		 * @returns NULL if invalid type, out of memory, or pool size exceeded.  
		 *          Non-NULL, valid MemPoolObject pointer otherwise
		 */
		MemPoolObject *		getObject (int nType);
	
		/** Release object back into the memory pool.  The type of the 
		 * object is extracted and then the object is added to the appropriate
		 * pool.  
		 * @param pObject The object to release back into the pool
		 * @returns True if successful, false otherwise
		 */
		bool						releaseObject (MemPoolObject * pObject);
	
		/** Dump the status of the memory pool to stdout
		 */
		void		dumpStatus ();
	
		/** Process a command from the console or external control
		 * mechanism.  
		 * 
		 * Show basic information for the timer manager
		 * 
		 *   cmd timer status
		 * 
		 * @param pCommands The vector of commands to process
		 * @param nOffset The offset at which to begin processing. The offset allows
		 *   us to pass the vector itself with modifications to the underlying
		 *   modules
		 * @returns Enumerated value representing result of command processing
		 */ 
		CommandResult	processCommand (const vector<string> & theCommands, int nOffset);


		/** Modify the maximum pool size for one specific kind of memory pool
		 * object
		 * @param nType The type to change the maximum number of allocatable object
		 * @param nNewMax The new maximum for that type of object
		 * @returns True if successful, false otherwise
		 */
		bool			changeMaxPoolSize (int nType, int nNewMax);
	
	protected:
		/** Increase the size of the pool for that particular object. It is
		 * assumed that the callee has already locked the mutex for the
		 * particular type in question
		 * @param nType The object in the pool to increase its allocation
		 * @returns 1 if successful, 0 if failed
		 */
		char				increasePool (int nType);
	
	private:	
		// The memory pool array
		//    In short, it is simply one vector for each possible MemoryPoolObject
		//    type.  		
		deque<MemPoolObject *> 		m_PoolArray[MEMPOOL_OBJ_CUSTLAST];
	
		// The pthreads associated with the memory objects
		pthread_mutex_t			m_PoolMutexes[MEMPOOL_OBJ_CUSTLAST];
		
		// The growth curves for the various objects if the current pool
		//   is exhausted (i.e. how many more should be allocated)
		int							m_nPoolGrowth[MEMPOOL_OBJ_CUSTLAST];
		
		// The maximum size allowed for each pool
		int							m_nMaxPoolSize[MEMPOOL_OBJ_CUSTLAST];
		
		// Number of the current object allocated. This is solely for
		//  discerning objects that are out in the wild and never return
		//  home
		int							m_nAllocated[MEMPOOL_OBJ_CUSTLAST];
};

// The memory pool itself is a global object to enable easy sharing
//  between the different threads and code base
extern	MemoryPool		g_MemPool;

#endif /*MEMORYPOOL_H_*/
