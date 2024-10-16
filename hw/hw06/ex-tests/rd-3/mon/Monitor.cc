/* Monitor.cc
 **********************************************************
 * This code is part of the Scalability in a Box (ScaleBox)
 * system developed at the University of Notre Dame. 
 * 
 * For additional information and use restrictions, please 
 * visit the NetScale laboratory webpage at:
 *    http://netscale.cse.nd.edu
 *
 * PI:       Dr. Aaron Striegel    striegel@nd.edu
 * 
 * Students: Dave Salyers (lead)   Yingxin Jiang
 *           Xiaolong Li           Jeff Smith 			 
 * 
 **********************************************************
 * $Revision: 1.5 $  $Date: 2008/04/09 22:03:01 $
 **********************************************************
 */

#include <iostream>
using namespace std;


#include "Monitor.h"

#include "../pkt/Adapter.h"

#include "../pkt/IOModule.h"

#include "Thread_Timer.h"

#include "../stat/StatManager.h"

#include "../mem/MemoryPool.h"
#include "../xml/NodeDOM.h"

/** The global monitor which acts a centralized clearinghouse for adapters to 
 * discover each other and to keep track of various allocated items as well */
Monitor		g_Monitor;

Monitor::Monitor () {
	m_bKeepRunning = true;	
}

Monitor::~Monitor () {
	// Add in clean up code although the only time we are doing this is on shutdown
}

bool Monitor::startup () {
	g_TimerManager.startup();	
	
	g_StatManager.startup();
	
	return true;
}


void Monitor::addAdapter (Adapter * pAdapter) {
	if(pAdapter == NULL) {
		cerr << "Error! Attempted to push a NULL adapter onto the monitor adapter list" << endl;
		cerr << "   Ignoring attempt and continuing onwards" << endl;
		return;
	}
	
	m_Adapters.push_back(pAdapter);
}

void Monitor::addModule (IOModule * pModule) {
	if(pModule == NULL) {
		cerr << "Error! Attempted to push a NULL module onto the monitor module list" << endl;
		cerr << "   Ignoring attempt and continuing onwards" << endl;
		return;
	}
	
	m_Modules.push_back(pModule);
}


Adapter * Monitor::getAdapter (int nAdapter) {
	if(nAdapter < 0 || nAdapter >= m_Adapters.size()) {
		return NULL;
	} else {
		return m_Adapters[nAdapter];
	}
}

IOModule * Monitor::getModule (int nModule) {
	if(nModule < 0 || nModule >= m_Modules.size()) {
		return NULL;
	} else {
		return m_Modules[nModule];
	}
}

int Monitor::getCountAdapters () {
	return m_Adapters.size();
}

int Monitor::getCountModules () {
	return m_Modules.size();
}

CommandResult	Monitor::processCommand (const vector<string> & theCommands, int nOffset) {
	int		j;
	CommandResult	theResult;
	
	// Does the command want an adapter or an upper level module?

	if(theCommands.size() <= nOffset) {
		return PROCESS_CMD_SYNTAX;
	}
	
	if(theCommands[nOffset] == "adp") {
		for(j=0; j<m_Adapters.size(); j++) {
			theResult = m_Adapters[j]->processCommand(theCommands,nOffset+1);
			
			if(theResult >= PROCESS_CMD_ERROR) {
				return theResult;
			}
		}
	} else if(theCommands[nOffset] == "mod") {
		cout << "Module detected" << endl;
		
		for(j=0; j<m_Modules.size(); j++) {
			theResult = m_Modules[j]->processCommand(theCommands,nOffset+1);
			
			cout << "Result from module " << j << " is " << theResult << endl;
			cout << "  Module name was " << m_Modules[j]->getName() << endl;
			
			if(theResult >= PROCESS_CMD_ERROR) {
				return theResult;
			}
		}
	} else if(theCommands[nOffset] == "timer") {
		return g_TimerManager.processCommand(theCommands,nOffset+1);
	} else if(theCommands[nOffset] == "statstream") {
		return g_StatManager.processCommand(theCommands,nOffset+1);
	} else if(theCommands[nOffset] == "mempool") {
		return g_MemPool.processCommand(theCommands,nOffset+1);
	} else if(theCommands[nOffset] == "showstats") {
		// Gather the stats
		NodeDOM *		pDOM;
		pDOM = gatherStats();
		pDOM->displayConsole();
		pDOM->release();
	} else if(theCommands[nOffset] == "help") {
		cout << "help: Help for the monitor / module interactions" << endl;
		cout << "  cmd listmods     List of the names of all accessible / queryable modules" << endl;
		cout << "  cmd mempool XXX  Interact with the memory pool" << endl;
		cout << "  cmd showstats    Display the statistics (current values)" << endl;
		return PROCESS_CMD_SUCCESS;
	} else if(theCommands[nOffset] == "listmods") {
		cout << "listmods: " << m_Modules.size() << " modules enabled" << endl;

		for(j=0; j<m_Modules.size(); j++)
		{
			cout << "    Module " << j << ": " << m_Modules[j]->getName() << endl;
		}

		return PROCESS_CMD_SUCCESS;
	}
	
	return PROCESS_CMD_UNKNOWN;	
}


void Monitor::dumpStat 	 (string sName) {
	int j;

	for(j=0; j<getCountAdapters(); j++) {
		Adapter * 	pTestAdp;
				
		pTestAdp = m_Adapters[j];	
				
		pTestAdp->dumpStats(sName);					
	}		
}

bool Monitor::shouldKeepRunning () {
	return m_bKeepRunning;	
}
		
void Monitor::shutdown () {
	m_bKeepRunning = false;
	
	// TODO - Add in code that contacts the individual modules and adapters
}

NodeDOM * Monitor::gatherStats () {
	NodeDOM 	* pRoot;
	NodeDOM  * pNode;
	int			nNumAdapters;
	int			nNumModules;
	int			j;
	
	pRoot = (NodeDOM *) g_MemPool.getObject(MEMPOOL_OBJ_DOMNODE);
	
	pRoot->setTag("ScaleBox:Stats");
	
	nNumAdapters = getCountAdapters();
	nNumModules = getCountModules();

	pNode = (NodeDOM *) g_MemPool.getObject(MEMPOOL_OBJ_DOMNODE);
	pNode->setTag("Stats:Adapters");
	pRoot->addChild(pNode);
	
	for(j=0; j<nNumAdapters; j++) {
		m_Adapters[j]->gatherStats(pNode);		
	}

	pNode = (NodeDOM *) g_MemPool.getObject(MEMPOOL_OBJ_DOMNODE);
	pNode->setTag("Stats:Modules");
	pRoot->addChild(pNode);
	
	for(j=0; j<nNumModules; j++) {
		m_Modules[j]->getStats(pNode);
	}
		
	return pRoot;	
}




