/* Thread_IO.cc
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

#include "Thread_IO.h"

#include <iostream>
using namespace std;

#include <pthread.h>

#include "Adapter.h"
#include "Packet.h"
#include "../mem/MemoryPool.h"

#include "../util/mgmt.h"

void * Thread_Input (void * pArg) {
	Adapter		*	pAdapter;
	
	pAdapter = (Adapter *) pArg;

	while(pAdapter->shouldKeepRunning()) {	

		if(pAdapter->getFlagEnabled()) {	
			// The read packet function reads in a packet from the adapter
			// and places it into the input queue for consumption by the
			// processing thread	
			pAdapter->readPacket();
		} else {
			//sleep(0);
			yieldCPU();
		}		
	}
	
	return NULL;
}

void * Thread_Process (void * pArg) {
	Adapter * pAdapter;
	Packet  * pPacket;

	
	pAdapter = (Adapter *) pArg;

	while(pAdapter->shouldKeepRunning()) {	

		if(pAdapter->getFlagEnabled()) {	
			pPacket = pAdapter->popInputPriorityQueue();
			
			if(pPacket == NULL) {
				pPacket = pAdapter->popInputQueue();
			}
			
			if(pPacket == NULL) {
				// Nothing in the queue, relinquish control
				//sleep(0);
				yieldCPU();
			} else {
				// There is something in the queue, give it to the input modules
				//
				//   If the result is true, somebody gobbled it and the packet is
				//   not our problem any more :)
				//
				//   If the result is false, it means that one of the input modules
				//   did not grab it and nobody wants it. Hence, we need to give it
				//   back to the global memory pool
				//
				//   Nominally, a dual-homed host should have an InputPassthrough
				//	  module in place at the last point in the chain to always forward
				//   packets onwards
				if(pAdapter->doInputChain(pPacket)) {
					// Somebody gobbled it, excellent	
				} else {
					// Doh, nobody wanted the packet
					g_MemPool.releaseObject(pPacket);	
				}
			}
		} else {
			//sleep(0);
			yieldCPU();
		}		
	}
	
	return NULL;	
}

void * Thread_Output (void * pArg) {
	Adapter * pAdapter;
	Packet  * pPacket;

	
	pAdapter = (Adapter *) pArg;

	while(pAdapter->shouldKeepRunning()) {	

		if(pAdapter->getFlagEnabled()) {
			pPacket = pAdapter->popOutputPriorityQueue();
			
			if(pPacket == NULL) {								
				pPacket = pAdapter->popOutputQueue();
			}
			
			if(pPacket == NULL) {
				// Nothing in the queue, relinquish control
				//sleep(0);
				yieldCPU();
			} else {
				// There is something in the queue, give it to the output modules
				//
				//   If the result is true, somebody gobbled it and the packet is
				//   not our problem any more :)
				//
				//   If the result is false, it means that one of the output modules
				//   did not grab it and nobody wants it. Hence, we need to give it
				//   back to the global memory pool
				//
				if(pAdapter->doOutputChain(pPacket)) {
					// Somebody gobbled it, excellent	
				} else {
					// Doh, nobody wanted the packet
					g_MemPool.releaseObject(pPacket);	
				}
			}
		} else {
			//sleep(0);
			yieldCPU();
		}		
	}
	
	return NULL;		
}


