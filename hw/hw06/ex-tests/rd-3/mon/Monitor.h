/* Monitor.h
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
 * $Revision: 1.4 $  $Date: 2008/04/04 21:45:41 $
 **********************************************************
 */

#ifndef MONITOR_H_
#define MONITOR_H_

#include "../core/Console.h"			// For definition of CommandResult

#include <vector>
using namespace std;

#include <string>
using namespace std;

class Adapter;
class IOModule;
class NodeDOM;

/** The monitor is the global master class that keeps together
 * the various adapters and other components for a centrally
 * accessible global object
 */
class Monitor {
	public:
		Monitor();
		~Monitor ();
		
		/** Add an adapter to the list of internal adapters */
		void		addAdapter (Adapter * pAdapter);
		
		/** Add a module to the list that is not adapter specific
		 * @param pModule The module to add to the monitor
		 */
		void		addModule (IOModule * pModule);
		
		/** Retrieve a specific adapter */
		Adapter *	getAdapter (int j);
		
		/** Retrieve a specific module */
		IOModule *	getModule (int j);
		
		
		/** Retrieve the number of adapters registered at the monitor */
		int			getCountAdapters ();

		/** Retrieve the number of multi-interface adapters at the monitor */
		int			getCountModules ();
		
		/** Process a command from the console or external control
		 * mechanism.  The command will be checked against monitor level
		 * operations and then as applicable, passed down to the adapters
		 * and modules themselves. The command functionality is referenced by specifying
		 * the exact module name or module chain of an adapter followed by a
		 * set of arguments separated by spaces.  
		 * 
		 * Show the stats for the pcap-eth1 adapter on the input chain:
		 * 
		 *   cmd adp pcap-eth1 ic show stats
		 * 
		 * Show the stats for the RIPPS module
		 *  
		 *   cmd mod ripps show stats
		 * 
		 * @param pCommands The vector of commands to process
		 * @param nOffset The offset at which to begin processing. The offset allows
		 *   us to pass the vector itself with modifications to the underlying
		 *   modules
		 * @returns Enumerated value representing result of command processing
		 */ 
		CommandResult	processCommand (const vector<string> & theCommands, int nOffset);
		
		/** Dump the statistics of a specific adapter or module name.  Any modules
		 * that match the name will have their statistics dumped. Wildcards are
		 * currently not supported at this time
		 * @param sName The name of the objects to dump their statistics 
		 */
		void			dumpStat 	 (string sName);
		
		/** Start the monitor and its underlying thread(s)
		 */
		bool			startup ();
		
		/** Determine if the underlying threads and adapters should keep running
		 * in the global sense. The monitor represents the global state and a
		 * value that is false denotes that the entire application is shutting
		 * down
		 * @returns True if the everything is still running, false if shutdown
		 *           is underway
		 */
		bool			shouldKeepRunning ();
		
		/** Change the state of the monitor to note that the monitor and all of
		 * the various modules should shut down
		 */
		void			shutdown ();
		
		/** Poll all of the underlying modules to extract the statistics for
		 * streaming to clients listening to the statistic stream
		 */
		NodeDOM *	gatherStats ();
		
	private:
		// A vector of adapter pointers that denotes where the setup is
		// getting its various inputs from
		vector<Adapter *> 	m_Adapters;
	
		// A vector of IOModule pointers that contains the list of modules
		// that are not adapter-specific, usually adapters that need multiple
		// vantage points to function
		vector<IOModule *> 	m_Modules;
				
		bool						m_bKeepRunning;
};

extern Monitor		g_Monitor;

#endif /*MONITOR_H_*/
