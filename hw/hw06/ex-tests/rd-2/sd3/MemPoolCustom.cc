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
 * $Revision: 1.4 $  $Date: 2008/04/11 22:03:19 $
 **********************************************************
 */

#include "MemPoolCustom.h"

#include "../RIPPS/RIPPS_Support.h"

MemPoolObject * 	allocateManagedCustomMemObject (int nType) {
	if(nType >= MEMPOOL_OBJ_CUSTLAST || nType == MEMPOOL_OBJ_CUSTSTART) {
		return NULL;
	}

	switch(nType) {
		case MEMPOOL_OBJ_RIPPS_MONPKT:			
			return mapObjectFromName_RIPPS ("RIPPS_MonitorPkt");
	}
	
/*	switch(nType) {
		case YOUR_CUSTOM_TYPE_HERE:
			return new YOUR CLASS HERE;
	} */
	
	return NULL;
	
}

string		mapCustomObjectNameToID (int nType) {

	switch(nType) {
		case MEMPOOL_OBJ_RIPPS_MONPKT:	
			return "RIPPS_MonitorPkt";
		default:
			return "Unknown";
	}

}
