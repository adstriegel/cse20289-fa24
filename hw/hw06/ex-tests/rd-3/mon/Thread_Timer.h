/* Thread_Timer.h
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
 * $Revision: 1.10 $  $Date: 2008/04/09 02:33:00 $
 **********************************************************
 */

#ifndef THREAD_TIMER_H_
#define THREAD_TIMER_H_

#include <vector>
using namespace std;

#include <deque>
using namespace std;

#include <string>
using namespace std;

#include <pthread.h>

#include "../core/Console.h"
#include "../mem/MemPoolObject.h"

class IOModule;

/** A class capturing an individual timer event
 */
class TimerEvent: public MemPoolObject {
public:
	TimerEvent();
	~TimerEvent();

	/** Attempt to retrieve a timer event object from the global management pool
	 * and if necessary, allocate a new object.
	 * @returns A pointer to a valid TimerEvent object
	 */
	static TimerEvent * createTimer();

	/** Clean the object before giving it back to the pool as
	 * appropriate.  Special care should be taken in derived functions
	 * to make this as minimal as possible.
	 */
	virtual void clean();

	/** Repeat the timer interval based on
	 * the targeted time for the timer to be invoked
	 */
	void repeatIntervalfromLastTargetTime();

	/** Repeat the timer interval from the current time
	 * in the system
	 */
	void repeatInterval();

	/** Arm the timer to occur at the interval time from the current
	 * system time.  This function is identical in behavior to
	 * repeatInterval (in fact it calls it).
	 */
	void armTimer();

	/** Set the interval for the timer in terms of milliseconds
	 * @param nMilliseconds The interval for the timer
	 */
	void setInterval_ms(unsigned int nMilliseconds);
	void setInterval_us(unsigned int nMicroseconds);

	/** Is the timer itself armed?
	 * @returns True if the timer is armed, false otherwise
	 */
	bool isArmed();

	/** Retrieve a pointer to the invocation time for the timer
	 * @returns A valid pointer to the internal invocation time
	 */
	struct timeval * getInvocationTime();

	/** Set the invocation time for the timer and arm the timer
	 * @param pTime The invocation time for the timer
	 */
	void setInvocationTime(struct timeval * pTime);

	/** Return the distance of the invocation time for this time relative
	 * to the system time to determine how long one can wait before needing
	 * to invoke the timer. The distance returned is only positive
	 * @param pCurrentTime A pointer to the current system time
	 * @returns 0 if the timer has expired, otherwise the distance in terms of
	 *  microseconds between the invocation time and the current time
	 */
	unsigned int computeTimeToInvocation(struct timeval * pCurrentTime);

	/** Has the timer expired relative to the given time. This function
	 * simply tests to see if the currentTime is greater than the invocation time.
	 * @param pCurrentTime The current perception of time
	 * @returns True if the timer has expired, false if it is still pending
	 */
	bool hasExpired(struct timeval * pCurrentTime);

	/** Compare this timer versus the other timer to see which has
	 * an earlier invocation time
	 * @param A pointer to a valid TimerEvent object
	 * @returns True if this object has an earlier invocation time, false otherwise
	 */
	bool hasEarlierInvocation(TimerEvent * pOtherTimer);

	/** Retrieve the timer ID associated with this timer. The timer ID need
	 * not necessarily be unique unless deemed necessary by the timer initiator
	 * @returns The value of the timer ID field
	 */
	unsigned int getTimerID();

	/** Set the ID to be associated with this particular timer instance. The
	 * timer ID does not have to be unique and uniqueness will not be enforced
	 * within the context of the larger TimerAggregator object
	 * @param nID The new ID for the timer
	 */
	void setTimerID(unsigned int nID);

	/** Retrieve the data pointer associated with this timer
	 * @returns The pointer for the uncast data associated with the timer
	 */
	void * getData();

	/** Set a pointer to data to be associated with this timer.
	 * Note that the timer does not assume ownership of the data with respect
	 * to releasing said memory block back to the OS upon invocation or
	 * deletion of the timer itself.
	 * @param pData The new data to point to for this timer
	 */
	void setData(void * pData);

private:
	/** Is the timer armed? If so, it will have a valid invocation time
	 */
	bool m_bValidTime;

	/** The invocation time for the timer itself
	 */
	struct timeval m_invocationTime;

	/** The interval between invocations of the timer (in microseconds)
	 */
	unsigned int m_nIntervalUS;

	/** The data associated with this timer */
	void * m_pData;

	/** The ID associated with this timer */
	unsigned int m_lTimerID;
};

enum TimerAggInsertStrategy {
	TIMERAGG_INSERT_END = 0, TIMERAGG_INSERT_BEGIN
};

/** A class capturing an aggregation of timer events for a particular object
 * active within the ScaleBox framework.  The aggregator reports only the earliest
 * event to the global centralized manager rather than reporting each individual
 * timer. It is important to note that this object does not do memory management
 * per se, rather it simply keeps a list of references and management of the particular
 * TimerEvent objects is left to the actual objects using the timers themselves. This
 * object should be thought of more so as a bridge rather than a trusted party responsible
 * for garbage collection.
 */
class TimerAggregator {
public:
	TimerAggregator();
	~TimerAggregator();

	/** Add a timer event to this aggregator and sort it with regards
	 * to its invocation time
	 * @param pTimer  The timer event to add
	 * @returns True if the timer was successfully added, false if it could
	 *  not be added
	 */
	bool addTimerEvent(TimerEvent * pTimer);

	/** Retrieve the next timer event for this aggregator to be invoked
	 * @returns A pointer to a valid TimerEvent object if there is anything
	 * in the queue, otherwise NULL if the queue is empty
	 */
	TimerEvent * getNextEvent();

	/** Attempt to pop the lead timer from the stack and process
	 * the timer. The process is atomic in that in ensures that the correct
	 * timer is removed from the queue provided that the queue always
	 * maintains its sorted nature
	 */
	bool popProcessTimer();

	/** Fetch the insertion strategy for timer events currently employed
	 * by this aggregator
	 * @returns The current insertion strategy, see header file for more details
	 */
	int getInsertStrategy();

	/** Change the insertion strategy from the default of working from
	 * the end forwards
	 * @param nInsertStrategy The new insertion strategy
	 */
	void setInsertStrategy(int nInsertStrategy);

	/** Link in the IO module for callback
	 * @param pModule The I/O module to invoke on the callback
	 */
	void setIOModule(IOModule * pModule);

protected:

	/** Signal a change to the master TimerManager object to break the
	 * timer thread loops out of a sleep in case a newer, faster timer
	 * event object has been created
	 */
	void signalTimerChange();

private:
	/** The set of sorted timer events that are yet to be invoked sorted
	 * with the next timer to occur at the front of the queue.
	 */
	deque<TimerEvent *> m_TimerEvents;

	/** Mutex to guard the timer event listing */
	pthread_mutex_t m_mutexTimers;

	int m_nInsertStrategy;

	/** The I/O module tied to this aggregator that will have its timer linkage
	 * function invoked whenever a timer goes off
	 */
	IOModule * m_pIOModule;

};

/** The TimerManager class acts as a centralized point from
 * which timer messages are dispatched. Currently, timers are
 * viewed as a large scale calendar type of queue whereby the
 * outlying objects register their TimerAggregator object with this
 * centralized object. The local thread will then wait the
 * appropriate amount of time and dispatch callbacks to that
 * set of TimerAggregator which will then pop its localized queue
 * for the particular TimerEvent.  
 */
class TimerManager {
public:
	TimerManager();
	~TimerManager();

	/** Register a particular timer aggregator with this centralized
	 * manager such that the time aggregator will then be considered
	 * as part of the timer thread for callbacks.
	 * @param pTimeAg A pointer to a valid TimerAggregator object
	 * @returns True if successful, false if unsuccessful
	 */
	bool registerTimerAggregator(TimerAggregator * pTimeAg);

	/** Unregister a particular timer aggregator with this centralized
	 * manager and no longer issue callbacks to this aggregator. Note
	 * that this will not intercept a timer callback already in progress.
	 * The object itself will only be removed from consideration and
	 * memory management will not be handled. The callee is still
	 * responsible for memory cleanup of the TimerAggregator object itself
	 * as this function only removes it from the list for polling.
	 * @param pTimeAg A pointer to a valid TimerAggregator object
	 * @returns True if the object was found and unregistered, false if
	 *          the object was invalid or unable to be found
	 */
	bool unregisterTimerAggregator(TimerAggregator * pTimeAg);

	/** Start up the timer manager which primarily refers to starting
	 * up the underlying thread or threads that are responsible for
	 * timing events
	 */
	void startup();

	/** Allow TimerAggregator objects to signal that one of their internal
	 * timers has been changed and that we should process the entire set of
	 * aggregators for newer, faster timer events.
	 */
	void setSignalTimerChange();

	/** Clear the signal denoting that a single TimerAggregator object has
	 * updated one of its timers and that any sleep operations should be
	 * interrupted
	 */
	void clearSignalTimerChange();

	/** Retrieve a pointer to the signal for timer change propogation. Note
	 * that this signal is strictly read only and manipulation should be done
	 * through the appropriate mutexes to avoid thrashing if possible
	 * @returns A pointer to a valid memory location for the signal
	 */
	char * getSignalforTimerChange();

	/** Return the count of aggregators registered with the time manager
	 * @returns Number of aggregators registered at the manager
	 */
	unsigned int getCountAggregators();

	/** Retrieve a specific aggregator from the list of aggregators
	 * registered at this manager
	 * @param nAggreg The aggregator to request
	 * @returns A pointer to a valid TimerAggregator if one exists in the
	 *  list, otherwise NULL if the list boundaries are exceeded)
	 */
	TimerAggregator * getAggregator(int nAggreg);

	/** Get the mutex for the aggregation list if one is doing
	 * sequential list parsing to prevent the removal of an aggregation
	 * object and its subsequent deletion from causing thread-safety
	 * issues.
	 * @returns A pointer to the mutex for the aggregation list to allow
	 *   an external entity / thread to lock the mutex while operating
	 */
	pthread_mutex_t * getMutexAggregList();

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
	CommandResult processCommand(const vector<string> & theCommands,
			int nOffset);

protected:

	/** Retrieve a pointer to the timer thread
	 * @returns A non-NULL pointer to the timer thread
	 */
	pthread_t * getThreadTimer();

private:
	/** A list of linkages to objects that wish to have timer callbacks
	 * enabled
	 */
	vector<TimerAggregator *> m_listTimers;

	/** The timer thread (for now just one) */
	pthread_t m_TimerThread;

	/** The shared memory location for a signal to any sleeping threads */
	char m_bTimerChangeFlag;

	/** Mutex to guard the update signal */
	pthread_mutex_t m_MutexTimerChangeFlag;

	// TODO - Resolve thread safety from the context of the Thread_Timer
	//  function.

	/** Timer aggregator list mutex */
	pthread_mutex_t m_MutexAggregList;

};

/** The global timer manager */
extern TimerManager g_TimerManager;

/** Thread for timer-based operations that is spawned by the primary monitor
 *  object
 */
void * Thread_Timer(void * pArg);

#endif /*THREAD_TIMER_H_*/
