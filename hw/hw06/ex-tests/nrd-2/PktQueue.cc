/* PktQueue.cc
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
 * $Revision: 1.4 $  $Date: 2008/04/11 22:01:03 $
 **********************************************************
 */
 
#include "PktQueue.h"
 
PktQueue::PktQueue () {
 	
}
 
PktQueue::~PktQueue () {
 	
}
 
pthread_mutex_t * 	PktQueue::getMutexQueue () {
	return &m_MutexQueue;	
}

void PktQueue::dumpInfo () {

}
