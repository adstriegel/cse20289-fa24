/* MemPoolCustom.h
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
 * $Revision: 1.3 $  $Date: 2008/04/11 22:03:19 $
 **********************************************************
 */

#ifndef MEMPOOLCUSTOM_H_
#define MEMPOOLCUSTOM_H_

#include <string>
using namespace std;

#include "MemPoolObject.h"

// Any changes to the custom list will requite a full re-compile of the code
//   (make clean, make)

enum MemPoolTypeCustom {
	MEMPOOL_OBJ_CUSTSTART=MEMPOOL_OBJ_LAST,		// Line this with the other list
	// User customized objects go here
	MEMPOOL_OBJ_RIPPS_MONPKT,		// Packet monitored by the RIPPS subsystem
	MEMPOOL_OBJ_CUSTLAST			// Do not delete this. It should always be at the
										// end of the list	
};

/** Allocate a managed memory object from the user customized space
 * @param nType A valid type drawn from the user custom pool in MemPoolCustom.h
 * @returns NULL if an invalid type was given, a valid non-NULL object otherwise 
 */
MemPoolObject * 	allocateManagedCustomMemObject (int nType); 

/** Map a custom object ID to an actual string name for listing the status
 * in the memory pool
 */
string		mapCustomObjectNameToID (int nType);

#endif /*MEMPOOLCUSTOM_H_*/
