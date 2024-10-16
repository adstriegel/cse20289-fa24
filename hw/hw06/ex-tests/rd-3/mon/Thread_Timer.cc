/* Thread_Timer.cc
 **********************************************************
 * This code is part of the Scalability in a Box (ScaleBox)
 * system developed at the University of Notre Dame. 
 * 
 * For additional information, please visit the NetScale
 * laboratory webpage at:
 *    http://netscale.cse.nd.edu
 *
 * PI:       Dr. Aaron Striegel    striegel@nd.edu
 * 
 **********************************************************
 * $Revision: 1.13 $  $Date: 2008/04/11 22:03:46 $
 **********************************************************
 */
 
#include <iostream>
using namespace std;

#include <sys/time.h>
  
#include "Thread_Timer.h"
#include "Monitor.h"
#include "../util/mgmt.h"
#include "../util/ip-utils.h" 

#include "../mem/MemoryPool.h"

#include "../pkt/IOModule.h"
 
TimerManager	g_TimerManager;
 
////////////////////////////////////////////////////////// 
 
TimerEvent::TimerEvent () : MemPoolObject (MEMPOOL_OBJ_TIMEREVENT) {
	clean();
}

//////////////////////////////////////////////////////////

TimerEvent::~TimerEvent () {
	
}

//////////////////////////////////////////////////////////

TimerEvent * TimerEvent::createTimer () {
	TimerEvent 	*	pTimer;
	
	pTimer = (TimerEvent *) g_MemPool.getObject(MEMPOOL_OBJ_TIMEREVENT);
	
	return pTimer;
}

//////////////////////////////////////////////////////////

void TimerEvent::clean () {
	m_lTimerID = 0;
	m_pData = NULL;	
	m_bValidTime = false;
	
	m_nIntervalUS = 0;	
}

//////////////////////////////////////////////////////////

void	TimerEvent::setInterval_ms (unsigned int nMilliseconds) {
	m_nIntervalUS = nMilliseconds * 1000;
}
//////////////////////////////////////////////////////////

void	TimerEvent::setInterval_us (unsigned int nMicroseconds) {
	m_nIntervalUS = nMicroseconds;
}

//////////////////////////////////////////////////////////

void	TimerEvent::repeatInterval () {			
	// Sample the system clock
	gettimeofday(&m_invocationTime, NULL);		
	
	// Adjust it by the targeted interval
	adjustTime(&m_invocationTime, m_nIntervalUS);	
}

//////////////////////////////////////////////////////////

void	TimerEvent::armTimer () {
	repeatInterval();
}

//////////////////////////////////////////////////////////

unsigned int TimerEvent::getTimerID () {
	return m_lTimerID;	
}
		
//////////////////////////////////////////////////////////
		
void TimerEvent::setTimerID (unsigned int nID) {
	m_lTimerID = nID;
}
		
//////////////////////////////////////////////////////////		
		
void * TimerEvent::getData () {
	return m_pData;
}

//////////////////////////////////////////////////////////
		
void TimerEvent::setData (void * pData) {
	m_pData = pData;	
}

//////////////////////////////////////////////////////////

unsigned int TimerEvent::computeTimeToInvocation (struct timeval * pCurrentTime) {
	if(hasExpired(pCurrentTime)) {
		return 0;
	} else {
//		cout << "  CT: " << pCurrentTime->tv_sec << "." << pCurrentTime->tv_usec << endl;
//		cout << "  IT: " << m_invocationTime.tv_sec << "." << m_invocationTime.tv_usec << endl;
//		cout << "  TD: " << calcTimeDiff(&m_invocationTime, pCurrentTime);
//		cout << endl;
		return calcTimeDiff(&m_invocationTime, pCurrentTime);
	}
}

//////////////////////////////////////////////////////////	
 
bool TimerEvent::hasExpired (struct timeval * pCurrentTime) {
	if(pCurrentTime->tv_sec > m_invocationTime.tv_sec) {
		return true;
	} else if (pCurrentTime->tv_sec == m_invocationTime.tv_sec) {
		if(pCurrentTime->tv_usec >= m_invocationTime.tv_usec) { 
			return true;
		} else {
			return false;
		}
	} else {
		// The seconds portion of the timer is neither greater nor equal to the
		// invocation time which must mean that we still have time to wait
		return false;
	}	
}

//////////////////////////////////////////////////////////

struct timeval * 	TimerEvent::getInvocationTime () {
	return &m_invocationTime;
}

//////////////////////////////////////////////////////////

		
void TimerEvent::setInvocationTime (struct timeval * pTime) {
	if(pTime != NULL) {	
		m_bValidTime = true;
		m_invocationTime = *pTime;
	}
}

//////////////////////////////////////////////////////////
 
TimerAggregator::TimerAggregator () {
 	m_nInsertStrategy = TIMERAGG_INSERT_END;
 	pthread_mutex_init(&m_mutexTimers, NULL); 
 	m_pIOModule = NULL;	 	
}

//////////////////////////////////////////////////////////
 
TimerAggregator::~TimerAggregator () {
 	
 	
}

//////////////////////////////////////////////////////////

void	TimerAggregator::setIOModule (IOModule * pModule) {
	m_pIOModule = pModule;	
}

//////////////////////////////////////////////////////////

bool TimerAggregator::addTimerEvent (TimerEvent * pTimer) {
	// Make sure at least we think this is a good timer 
	if(pTimer == NULL) {
		return false;
	}
	
	// TODO Optimize timer insertion operation from a linear search
	
	// Check to see when the invocation time of the timer is
	
	int		j;
	deque<TimerEvent *>::iterator		pIterate;

	
	pthread_mutex_lock(&m_mutexTimers);

	// Is the list empty?	
	if(m_TimerEvents.size() == 0) {
		m_TimerEvents.push_back(pTimer);
		pthread_mutex_unlock(&m_mutexTimers);
		signalTimerChange();
		return true;
	}
	
	pIterate = m_TimerEvents.begin();

	//cout << "Iterating through the list " << endl;

	while( pIterate < m_TimerEvents.end() ) {
		if(!compareTime( pTimer->getInvocationTime(), (*pIterate)->getInvocationTime())) {
			// Found where to put it, we are less than this invocation time
			m_TimerEvents.insert(pIterate,pTimer);
			pthread_mutex_unlock(&m_mutexTimers);
			signalTimerChange();
			return true;
		} else {	
			pIterate++;
		}	
	}	

	//cout << " Did not find it in the list, adding it to the end" << endl;

	// We only get here if we reached the end of the deque
//	if(!compareTime( pTimer->getInvocationTime(), (*pIterate)->getInvocationTime())) {
//		m_TimerEvents.insert(pIterate,pTimer);
//	} else {
		m_TimerEvents.push_back(pTimer);
//	}

	pthread_mutex_unlock(&m_mutexTimers);
	signalTimerChange();
	return true;
}

//////////////////////////////////////////////////////////

void TimerAggregator::signalTimerChange () {
	g_TimerManager.setSignalTimerChange();	
}

//////////////////////////////////////////////////////////

TimerEvent * TimerAggregator::getNextEvent () {
	pthread_mutex_lock(&m_mutexTimers);
	
	if(m_TimerEvents.size() == 0) {
		pthread_mutex_unlock(&m_mutexTimers);
		return NULL;
	} else {
		TimerEvent * pEvent;
		pEvent =  m_TimerEvents[0];
		pthread_mutex_unlock(&m_mutexTimers);
		return pEvent;
	}	
}

//////////////////////////////////////////////////////////

bool TimerAggregator::popProcessTimer () {
	TimerEvent * pTimer;
	
	pthread_mutex_lock(&m_mutexTimers);
	pTimer = m_TimerEvents[0];		
	m_TimerEvents.pop_front();
	pthread_mutex_unlock(&m_mutexTimers);
		
	// Pop the front timer event off the aggregator stack	
	if(m_pIOModule == NULL) {
		return false;
	} else {
		return m_pIOModule->processTimer(pTimer);
	}
}

//////////////////////////////////////////////////////////

TimerManager::TimerManager () {
 	pthread_mutex_init(&m_MutexTimerChangeFlag, NULL);
 	pthread_mutex_init(&m_MutexAggregList, NULL);
 	m_bTimerChangeFlag = 0;
}
 
  
TimerManager::~TimerManager () {
 	
}

//////////////////////////////////////////////////////////

char * TimerManager::getSignalforTimerChange () {
	return &m_bTimerChangeFlag;	
}

//////////////////////////////////////////////////////////

void TimerManager::setSignalTimerChange () {
	pthread_mutex_lock(&m_MutexTimerChangeFlag);
	m_bTimerChangeFlag = 1;
	pthread_mutex_unlock(&m_MutexTimerChangeFlag);
}

//////////////////////////////////////////////////////////

void TimerManager::clearSignalTimerChange () {
	pthread_mutex_lock(&m_MutexTimerChangeFlag);
	m_bTimerChangeFlag = 0;
	pthread_mutex_unlock(&m_MutexTimerChangeFlag);	
}

//////////////////////////////////////////////////////////

bool TimerManager::registerTimerAggregator (TimerAggregator * pTimeAg) {
	int		j;
	
	// Make sure this is a valid pointer
	if(pTimeAg == NULL) {
		return false;
	}
	
	pthread_mutex_lock(&m_MutexAggregList);	
	
	// Make sure this is not a duplicate addition to the list
	for(j=0; j<m_listTimers.size(); j++) {
		if(m_listTimers[j] == pTimeAg) {
			pthread_mutex_unlock(&m_MutexAggregList);				
			return false;
		}
	}
	
	m_listTimers.push_back(pTimeAg);
	pthread_mutex_unlock(&m_MutexAggregList);					
	return true;
} 

//////////////////////////////////////////////////////////

bool TimerManager::unregisterTimerAggregator (TimerAggregator * pTimeAg) {
	int		j;
	
	vector<TimerAggregator *>::iterator		pIterate;
	
	// Make sure this is a valid pointer
	if(pTimeAg == NULL) {
		return false;
	}
	
	pthread_mutex_lock(&m_MutexAggregList);		

	pIterate = m_listTimers.begin();

	// Can we find it?	
	for(j=0; j<m_listTimers.size(); j++) {
		if(*pIterate == pTimeAg) {
			// Found a match!
			m_listTimers.erase(pIterate);
			pthread_mutex_unlock(&m_MutexAggregList);	
			return true;				
		}
		pIterate++;
	}
		
	// Couldn't find it
	pthread_mutex_unlock(&m_MutexAggregList);					
	return false;
} 

//////////////////////////////////////////////////////////

pthread_t *	TimerManager::getThreadTimer () {
	return &m_TimerThread;	
}

//////////////////////////////////////////////////////////

void	TimerManager::startup () {
	int		nResult;
	
	// Start up the timer thread
	nResult = pthread_create(getThreadTimer(), NULL, Thread_Timer, this);
	if(nResult) {
		cerr << "* Error creating timer thread for timer manager" << endl;
		cerr << "    Code: " << nResult << endl;
		cerr << "   Bailing out of startup due to critical error" << endl;		
		exit(-1);
	}		
}

//////////////////////////////////////////////////////////

unsigned int TimerManager::getCountAggregators () {
	return m_listTimers.size();
}

//////////////////////////////////////////////////////////

TimerAggregator * 	TimerManager::getAggregator (int nAggreg) {
	TimerAggregator 	*	pAggreg;
	
	if(nAggreg < 0 || nAggreg >= m_listTimers.size()) {
		return NULL;
	} else {
		pAggreg = m_listTimers[nAggreg];		
		return pAggreg;
	}	
}

//////////////////////////////////////////////////////////

CommandResult	TimerManager::processCommand (const vector<string> & theCommands, int nOffset) {
	
	struct timeval	currentTime;
	
	if(theCommands.size() <= nOffset) {
		return PROCESS_CMD_SYNTAX;
	}
	
	if(theCommands[nOffset] == "status") {
		int			j;
		TimerAggregator *	pAggreg;
		TimerEvent * pNextTimer;
		struct timeval *	pInvokeTime;
		
		cout << "Timer Manager Status (Sys Time = ";
		gettimeofday(&currentTime, NULL);		
		cout << currentTime.tv_sec;
		cout << ".";
		cout << currentTime.tv_usec;
		cout << ")" << endl;
		
		cout << "  Reg. Aggregators: " << getCountAggregators() << endl;
		
		pthread_mutex_lock(getMutexAggregList());
		
		for(j=0; j<getCountAggregators(); j++) {		
			cout << "   Aggregator " << j << ": ";
			
			pAggreg = getAggregator(j);
			
			pNextTimer = pAggreg->getNextEvent();				
			
			if(pNextTimer == NULL) {
				cout << "None pending" << endl;
			} else {
				pInvokeTime = pNextTimer->getInvocationTime();
				cout << "Next @ " << pInvokeTime->tv_sec << "." << pInvokeTime->tv_usec << endl;
			}
		}
		
		pthread_mutex_unlock(getMutexAggregList());
				
		return PROCESS_CMD_SUCCESS;
	}
	
	
	return PROCESS_CMD_UNKNOWN;	
}

//////////////////////////////////////////////////////////

pthread_mutex_t * TimerManager::getMutexAggregList () {
	return &m_MutexAggregList;
}

// There is a bit of an issue with this block of code when there is a large number
//  of time aggregation objects with relatively infrequent timers. This is perhaps
//  something to optimize in an upcoming mod to the timing infrastructure but the
//  key problem will be how to keep it relatively OS-agnostic, i.e. it is a bit too
//  easy to make it Linux-specific and no longer friendly to Mac OS X, FreeBSD, or
//  Solaris.   
 
void * Thread_Timer (void * pArg) {
	struct timeval currentTime;
	struct timeval finishLoopTime;
	
	TimerManager *  pManager;
	TimerAggregator *	pAggreg;
	
	int		j;
	TimerEvent	*	pNextTimer;
	
	bool			bDidTimerExpire;
	unsigned int	nNextTimer;
	unsigned int	nTimeToExpire;
	unsigned int	nLoopTime;
	
	// Not that we really need it since it is a global variable but just in
	//  case
	pManager = (TimerManager *) pArg;
	
	while(g_Monitor.shouldKeepRunning()) {
		// On each run through the loop, we will presume that no timers have
		//  yet gone off
		bDidTimerExpire = false;
		nNextTimer = 0;		
			
		// Sample the system clock
		gettimeofday(&currentTime, NULL);
		
		// Iterate through our current list of time aggregator objects
		//
		//  Dispatch at most one timer per aggregator object on each run through
		//  the loop
		
		
		// The lock prevents manipulation of the overall list of timer aggregator
		//  objects but it does not prevent manipulation of the timers within
		//  a given aggregator. While it is perhaps being a bit overly cautious
		//  to make the aggregator operation (a relatively rare operation) thread
		//  safe, it will keep out what would be an extremely odd bug to track
		//  down from occurring.
		pthread_mutex_lock(pManager->getMutexAggregList());
		
		for(j=0; j<pManager->getCountAggregators(); j++) {
			pAggreg = pManager->getAggregator(j);			

			if(pAggreg == NULL) {
				cerr << "Ruh roh, the aggregation object in the timer manager was" << endl;
				cerr << "  NULL, this is probably very BAD!" << endl;
				break;
			}
						
			pNextTimer = pAggreg->getNextEvent();	
			
			if(pNextTimer != NULL) {
				// Have a winner, did it expire?		
				nTimeToExpire = pNextTimer->computeTimeToInvocation(&currentTime);
				
				//cout << "Time to invocation: " << nTimeToExpire << endl;
				
				// If it is equal to zero, it means that it has expired
				if(nTimeToExpire == 0) {
					bDidTimerExpire = true;
					pAggreg->popProcessTimer();
				} else if (!bDidTimerExpire) {
					// If it has not expired and no other timers have been invoked,
					//  see if this one has the smallest amount of time to wait
					if(nNextTimer == 0) {
						nNextTimer = nTimeToExpire;
					} else if (nTimeToExpire < nNextTimer) {
						nNextTimer = nTimeToExpire;
					}	
				}			
			} 	
		}
		
		pthread_mutex_unlock(pManager->getMutexAggregList());
				
		// Sample the system clock to assess how long we were in the loop checking in
		// with each TimerAggregator object in the list
		gettimeofday(&finishLoopTime, NULL);
		
		nLoopTime = calcTimeDiff(&finishLoopTime, &currentTime);
				
		if(!bDidTimerExpire) {			
			// No timer expired, therefore give up the CPU
			
			// If we spent so much time in the loop sampling that the next timer is
			//  likely here, no sense giving back up the CPU and waiting
			if(nLoopTime >= nNextTimer) {		
				continue;
			}			
									
			// Sleep this thread (and this thread only) until the next timer
			
			// Yes, the char flag pointer is not pretty but it works
			
			//cout << "Microsleeping for " << nNextTimer - nLoopTime << endl; 
			//cout << "  Loop vs. Next Timer" << nLoopTime << nNextTimer << endl;
			microsleep(nNextTimer - nLoopTime, g_TimerManager.getSignalforTimerChange());
			
			// Always clear the flag since we are now going back to the main loop that is
			//  going to sample everything						
			g_TimerManager.clearSignalTimerChange();
																					
		} else {
			// No need to sleep, we have more timers to do more likely than not
		}
 	}
}


 
