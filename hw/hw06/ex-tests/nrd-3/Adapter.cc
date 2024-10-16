/* Adapter.cc
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
 * $Revision: 1.9 $  $Date: 2008/04/11 22:03:58 $
 **********************************************************
 */

#include <iostream>
using namespace std;

#include "../mem/MemoryPool.h"
#include "Adapter.h"
#include "IOModule.h"
#include "InputModule.h"
#include "OutputModule.h"
#include "Packet.h"
#include "Thread_IO.h"
#include "../xml/NodeDOM.h"
#include "../xml/NodeElem.h"

#include "PktQueue.h"
#include "PktQueueFIFO.h"

Adapter::Adapter () {
	m_bEnabled = 0;
	m_bKeepRunning = 1;
	m_sName = "Adapter";
	
	// Configure the maximum queue sizes
	m_nMaxInQueueSize = DEFAULT_ADAPTER_MAXQUEUE;
	m_nMaxOutQueueSize = DEFAULT_ADAPTER_MAXQUEUE;
	
	// By default, allocate the basic FIFO queue unless otherwise replaced
	m_pOutQueue = new PktQueueFIFO();
	
	// Input queue, almost always FIFO as it would be extremely rare to put
	//  restrictions on in-bound traffic
	m_pInQueue = new PktQueueFIFO();
	
	// Priority queues whose primary purpose is to hold resume after packets
	m_pInPriorityQueue = new PktQueueFIFO();
	m_pOutPriorityQueue = new PktQueueFIFO();
}

Adapter::~Adapter () {
	if(m_pOutQueue != NULL) {
		delete m_pOutQueue;
		m_pOutQueue = NULL;
	}
	
	if(m_pInQueue != NULL) {
		delete m_pInQueue;
		m_pInQueue = NULL;
	}
	
	if(m_pOutPriorityQueue != NULL) {
		delete m_pOutPriorityQueue;
		m_pOutPriorityQueue = NULL;
	}
	
	if(m_pInPriorityQueue != NULL) {
		delete m_pInPriorityQueue;
		m_pInPriorityQueue = NULL;
	}	
}



void Adapter::setFlagEnabled (char bFlag) {
	m_bEnabled = bFlag;
}
		
char Adapter::getFlagEnabled () {
	return m_bEnabled;	
}
	

char Adapter::doOutputChain (Packet * pPacket) {
	IOModule		* 	pChain;
	int				j;
	bool			bResumeProcessing;
		
	bResumeProcessing = false;	
		
	// Ignore if the packet is NULL
	if(pPacket == NULL) {
		return 0;
	}

	if(pPacket->getResumeAfterModule() != NULL) {
		bResumeProcessing = true;
	}

	if(pPacket->isTraceEnabled()) {	
		cout << "**Trace Pkt***** doOutputChain " << getName() << " ******************" << endl;	
		printf("    Pkt = 0x%p\n", pPacket);
	}
		
	for(j=0; j<m_OutputModuleList.size(); j++) {
		if(bResumeProcessing == true) {
			//cout << " --> Resuming after processing from an earlier module" << endl;
			
			// If we are resuming, keep searching until we find our
			// winner in the list of the processing chain
			if(m_OutputModuleList[j] == pPacket->getResumeAfterModule()) {
				bResumeProcessing = false;
				pPacket->setResumeAfterModule(NULL);
				//printf(" -*- Cleared resume info on packet 0x%X (Output Chain) \n", pPacket);				
				continue;
			}
		} else {			
			//cout << "  Output module " << j << " processing..." << endl;
			if(!m_OutputModuleList[j]->passFilter(pPacket)) {
				continue;
			}
				
			if(m_OutputModuleList[j]->processPacket(pPacket)) {
				//cout << "*END***** doOutputChain " << getName() << " ******************" << endl;					
				return 1;	
			}
		}
	}
	
	if(bResumeProcessing) {
		cerr << "Error: Unable to find module on adapter OUTPUT chain for resuming processing" << endl;
	}	
		
	
	return 0;
}

char Adapter::doInputChain (Packet * pPacket) {
	IOModule		* 	pChain;
	int				j;
	
	bool			bResumeProcessing;
				
	// Ignore if the packet is NULL
	if(pPacket == NULL) {
		return 0;
	}
	
	if(pPacket->getResumeAfterModule() != NULL) {
		bResumeProcessing = true;
	} else {
		bResumeProcessing = false;
	}
	
	if(pPacket->isTraceEnabled()) {	
		cout << "**Trace Pkt***** doInputChain " << getName() << " ******************" << endl;	
		printf("    Pkt = 0x%p\n", pPacket);
	}
	
	for(j=0; j<m_InputModuleList.size(); j++) {
		//cout << "  Checking module " << j << endl;
		if(bResumeProcessing) {
			//cout << " --> Resuming after processing from an earlier module" << endl;
			//cout << "      Comparing ";
			//printf("entry %d of 0x%X vs. 0x%X\n", j, m_InputModuleList[j], pPacket->getResumeAfterModule()); 
			
			// If we are resuming, keep searching until we find our
			// winner in the list of the processing chain
			if(m_InputModuleList[j] == pPacket->getResumeAfterModule()) {
				bResumeProcessing = false;
				pPacket->setResumeAfterModule(NULL);		
				//printf(" -*- Cleared resume info on packet 0x%X (Input Chain) \n", pPacket);
				continue;
			}
		} else {								
			//cout << "  Input module " << j << " processing..." << endl;
			if(!m_InputModuleList[j]->passFilter(pPacket)) {
				continue;
			}
								
			if(m_InputModuleList[j]->processPacket(pPacket)) {
				//cout << "*END***** doInputChain " << getName() << " ******************" << endl;	
				return 1;	
			}
		}
	}
	
	if(bResumeProcessing) {
		cout << "Error: Unable to find module on adapter INPUT chain for resuming processing" << endl;
	}
		
	
	return 0;
}

void Adapter::stopRunning () {
	m_bKeepRunning = 0;	
}
		
char Adapter::shouldKeepRunning () {
	return m_bKeepRunning;	
}

pthread_t *	Adapter::getThreadInput () {
	return &m_ThreadInput;
}

pthread_t * Adapter::getThreadProcess () {
	return &m_ThreadProcess;
}

pthread_t * Adapter::getThreadOutput () {
	return &m_ThreadOutput;
}


void Adapter::appendInputModule (IOModule * pModule) {	
	if(pModule == NULL) {
		cerr << "Warning: Attempted to pass a NULL module to attach to the input chain" << endl;
		return;
	}
	
	// Link this module with the adapter and get the underlying mechanisms
	//  started and initialized as appropriate
	pModule->setAdapter(this);
	pModule->initialize();
	
	m_InputModuleList.push_back(pModule);
}

void Adapter::appendOutputModule (IOModule * pModule) {
	if(pModule == NULL) {
		cerr << "Warning: Attempted to pass a NULL module to attach to the output chain" << endl;
		return;
	}
	
	// Link this module with the adapter and get the underlying mechanisms
	//  started and initialized as appropriate
	pModule->setAdapter(this);
	pModule->initialize();

	m_OutputModuleList.push_back(pModule);	
}

string Adapter::getName			() {
	return m_sName;
}
		
void	Adapter::setName (string sName) {
	m_sName = sName;
}

void  Adapter::startThreads  () {
	int			nResult;
	
	// Initialize the mutexes
	pthread_mutex_init(getMutexInput(), NULL);
	pthread_mutex_init(getMutexOutput(), NULL);
	pthread_mutex_init(getMutexInputPriority(), NULL);
	pthread_mutex_init(getMutexOutputPriority(), NULL);

	
	// Start up the input thread
	nResult = pthread_create(getThreadInput(), NULL, Thread_Input, this);
	if(nResult) {
		cerr << "* Error creating input thread for adapter " << getName() << endl;
		cerr << "    Code: " << nResult << endl;
		
		exit(-1);
	}
	
	// Start up the process thread
	nResult = pthread_create(getThreadProcess(), NULL, Thread_Process, this);
	if(nResult) {
		cerr << "* Error creating process thread for adapter " << getName() << endl;
		cerr << "    Code: " << nResult << endl;
		
		exit(-1);
	}	
	
	// Start up the output thread
	nResult = pthread_create(getThreadOutput(), NULL, Thread_Output, this);
	if(nResult) {
		cerr << "* Error creating output thread for adapter " << getName() << endl;
		cerr << "    Code: " << nResult << endl;
		
		exit(-1);
	}		
	
}

bool 	Adapter::addInputQueue (Packet * pPacket) {	
	bool		bSuccess;
	
	if(pPacket == NULL) {
		return 0;
	}
	
	bSuccess = false;
	
	if(pPacket->getResumeAfterModule() != NULL) {
		//cout << "  Input Queue Pkt w/Priority on Packet ";
		//printf("0x%X", pPacket);
		//cout << " on adapter " << getName() << endl;
		bSuccess =  m_pInPriorityQueue->enqueue(pPacket);		
	} else {
		bSuccess =  m_pInQueue->enqueue(pPacket);
	}	
	
	if(!bSuccess) {
		cout << "* Warning: Queue insertion failed on INPUT queues" << endl;
	}
		
	return bSuccess;		
}

bool 	Adapter::addOutputQueue (Packet * pPacket) {
	bool		bSuccess;
	
	if(pPacket == NULL) {
		return 0;
	}
	
	if(pPacket->getResumeAfterModule() != NULL) {
		//cout << "  OUTPUT Queue Pkt w/Priority on Packet ";
		//printf("0x%X", pPacket);
		//cout << " on adapter " << getName() << endl;
		bSuccess = m_pOutPriorityQueue->enqueue(pPacket);		
	} else {
		bSuccess = m_pOutQueue->enqueue(pPacket);
	}	
	
	if(!bSuccess) {
		cout << "* Warning: Queue insertion failed on INPUT queues" << endl;
	}
	
	return bSuccess;		
}

pthread_mutex_t * Adapter::getMutexInputPriority () {
	return &m_MutexInPriorityQueue;
}

pthread_mutex_t * Adapter::getMutexOutputPriority () {
	return &m_MutexOutPriorityQueue;
}

pthread_mutex_t * Adapter::getMutexInput () {
	return &m_MutexInQueue;	
}
		
pthread_mutex_t * Adapter::getMutexOutput () {
	return &m_MutexOutQueue;
}

int Adapter::getInputQueueSize () {
	int		nSize;
	
	return m_pInQueue->getNumPackets();
	
/*	
	pthread_mutex_lock(&m_MutexInQueue);
	nSize = m_InPackets.size();
	pthread_mutex_unlock(&m_MutexInQueue);
	
	return nSize;*/
}


int Adapter::getOutputQueueSize () {
	int		nSize;
	
	return m_pOutQueue->getNumPackets();

/*	
	pthread_mutex_lock(&m_MutexOutQueue);
	nSize = m_InPackets.size();
	pthread_mutex_unlock(&m_MutexOutQueue);
	
	return nSize;	*/
}

char Adapter::setMaxInputQueueSize (int nSize) {
	// Validate nSize
	if(nSize <= 0) {
		return 0;
	}

	// Update the size
	pthread_mutex_lock(&m_MutexInQueue);
	m_nMaxInQueueSize = nSize;
	pthread_mutex_unlock(&m_MutexInQueue);	
	return 1;
}

char Adapter::setMaxOutputQueueSize (int nSize) {
	// Validate nSize
	if(nSize <= 0) {
		return 0;
	}

	// Update the size
	pthread_mutex_lock(&m_MutexOutQueue);
	m_nMaxOutQueueSize = nSize;
	pthread_mutex_unlock(&m_MutexOutQueue);	
	return 1;		
}

Packet * Adapter::popInputPriorityQueue () {
	Packet * pPacket;

	pPacket = m_pInPriorityQueue->popAndGetPacket();
		
	return pPacket;
}

Packet * Adapter::popInputQueue () {
	Packet * pPacket;

	pPacket = m_pInQueue->popAndGetPacket();
		
	return pPacket;
}

Packet * Adapter::popOutputPriorityQueue () {
	Packet * pPacket;

	pPacket = m_pOutPriorityQueue->popAndGetPacket();
		
	return pPacket;
}


Packet * Adapter::popOutputQueue () {
	Packet * pPacket;
			
	pPacket = m_pOutQueue->popAndGetPacket();
		
	return pPacket;
}

bool	Adapter::extractBaseDOM  (NodeDOM * pNode) {
	int			j;
	
	cout << "Adapter::extractBaseDOM" << endl;

	for(j=0; j<pNode->getNumChildren(); j++) {
		if(pNode->getChild(j)->getTag() == "name") {
			setName(pNode->getChild(j)->getData());
		}		
		
		if(pNode->getChild(j)->getTag() == "InputModule") {
			IOModule * 	pModule;
			string			sModName;
			NodeElem *		pElem;
			
			pElem = pNode->getChild(j)->findElement("type");
			
			if(pElem == NULL) {
				cerr << "Input module in DOM without type definition\n";			
				continue;
			}
			
			pModule = mapModuleFromName(pElem->getValue());			
			
			if(pModule == NULL) {
				cerr << "Input module type of " << pElem->getValue() << " not defined!\n";
				continue;
			}
			
			if(!pModule->extractBaseDOM(pNode->getChild(j))) {
				cerr << "Error processing XML configuration for input module of type " << pElem->getValue() << endl;
				delete pModule;
				continue;
			}

			if(!pModule->extractExtendedDOM(pNode->getChild(j))) {
				cerr << "Error processing extended XML configuration for input module of type " << pElem->getValue() << endl;
				delete pModule;
				continue;
			}
			
			pModule->setChainDirection(IOMODULE_CHAIN_INCOMING);			
			
			cout << "   Added an input module of type " << pElem->getValue() << endl;
			appendInputModule(pModule);
			continue;
		}
		
		if(pNode->getChild(j)->getTag() == "OutputModule") {
			IOModule * 	pModule;
			string			sModName;
			NodeElem *		pElem;
			
			pElem = pNode->getChild(j)->findElement("type");
			
			if(pElem == NULL) {
				cerr << "Output module in DOM without type definition\n";			
				continue;
			}
			
			pModule = mapModuleFromName(pElem->getValue());
			
			if(pModule == NULL) {
				cerr << "Output module type of " << pElem->getValue() << " not defined!\n";
				continue;
			}
			
			if(!pModule->extractBaseDOM(pNode->getChild(j))) {
				cerr << "Error processing XML configuration for output module of type " << pElem->getValue() << endl;
				delete pModule;
				continue;
			}

			if(!pModule->extractExtendedDOM(pNode->getChild(j))) {
				cerr << "Error processing extended XML configuration for output module of type " << pElem->getValue() << endl;
				delete pModule;
				continue;
			}
			
			pModule->setChainDirection(IOMODULE_CHAIN_OUTGOING);
			
			cout << "   Added an output module of type " << pElem->getValue() << endl;
			appendOutputModule(pModule);
			continue;
		}
		
	}
	
	cout << " finishing base, going onto Extended" << endl;
	return extractExtendedDOM(pNode);
}


void	Adapter::startDevice () {
}

void	Adapter::dumpBasicInfo  () {
	
}

bool	Adapter::startModules () {
	IOModule		* 	pChain;
	int				j;
	bool				bSuccess;
	
	bSuccess = true;
				
	for(j=0; j<m_InputModuleList.size(); j++) {	
		if(!m_InputModuleList[j]->startModule()) {
			bSuccess = false;
		}
	}

	for(j=0; j<m_OutputModuleList.size(); j++) {	
		if(!m_OutputModuleList[j]->startModule()) {
			bSuccess = false;
		}
		
	}
	
	return bSuccess;
}

void	Adapter::writePacket (Packet * pPacket) {
	// We are generic, gobble the packet by giving it back to the global memory manager
	g_MemPool.releaseObject(pPacket);	
}

void Adapter::dumpStats (string sName) {
	IOModule		* 	pChain;
	int				j;
	NodeDOM 	*		pStats;	
	int				nStatCount;		
				
	NodeDOM 	*		pStatRoot;
	
	pStatRoot = NULL;
	nStatCount = 0;
	
	pStatRoot = (NodeDOM *) g_MemPool.getObject(MEMPOOL_OBJ_DOMNODE);	
	
	if(pStatRoot == NULL) {
		cerr << "Error: Out of memory - unable to allocate stat object" << endl;
		return;
	}			
	
	pStatRoot->setTag("Stat");
				
	for(j=0; j<m_InputModuleList.size(); j++) {	
		if(m_InputModuleList[j]->getName() == sName) {
			pStats = m_InputModuleList[j]->getStats(pStatRoot);
			
			if(pStats != NULL) {
				nStatCount++;
			}
		}			
	}

	for(j=0; j<m_OutputModuleList.size(); j++) {	
		if(m_OutputModuleList[j]->getName() == sName) {
			pStats = m_OutputModuleList[j]->getStats(pStatRoot);			
			
			if(pStats != NULL) {
				nStatCount++;
			}
		}
	}	
	
	// Only display a result if we actually got a hit
	if(nStatCount) {
		pStatRoot->dumpConsole();
	}
	
	pStatRoot->clean();
	g_MemPool.releaseObject(pStatRoot);	
}

void	Adapter::listModules () {
	IOModule		* 	pChain;
	int				j;
	NodeDOM 	*		pStats;	

	cout << "Adapter (" << getName() << ")" << endl;

	cout << " Input Modules" << endl;				
	for(j=0; j<m_InputModuleList.size(); j++) {	
		cout << " " << m_InputModuleList[j]->getType() << "   Name: " << m_InputModuleList[j]->getName() << endl;
	}

	cout << " Output Modules" << endl;				
	for(j=0; j<m_OutputModuleList.size(); j++) {	
		cout << " " << m_OutputModuleList[j]->getType() << "   Name: " << m_OutputModuleList[j]->getName() << endl;
	}			
}

/////////////////////////////////////////////////////////////////////////

CommandResult Adapter::processCommand (const vector<string> & theCommands, int nOffset) {	
	// Check if the size is sufficient
	if(theCommands.size() <= nOffset) {
		return PROCESS_CMD_SYNTAX;
	}
	
	// See if the name field is for us	
	if(!matchName(theCommands[nOffset])) {
		return PROCESS_CMD_DIFFTARGET;
	}
	
	// OK, we have at least the name we think	
	return processExtCommand(theCommands, nOffset+1);
}

/////////////////////////////////////////////////////////////////////////

CommandResult Adapter::processExtCommand (const vector<string> & theCommands, int nOffset) {
	return PROCESS_CMD_UNKNOWN;	
}

/////////////////////////////////////////////////////////////////////////

bool Adapter::matchName (string sSearch) {
	if(sSearch == "*") {
		return true;
	}
	
	if(sSearch == m_sName) {
		return true;
	} else {
		return false;
	}	
}

/////////////////////////////////////////////////////////////////////////

bool Adapter::gatherStats (NodeDOM * pParent) {
	NodeDOM  * pNode;
	NodeDOM 	* pChainNode;
	int			j;
	
	pNode = (NodeDOM *) g_MemPool.getObject(MEMPOOL_OBJ_DOMNODE);	
	
	pNode->setTag("Adapter");
	
	pParent->addChild(pNode);
	
	// Do the input chain
	pChainNode = (NodeDOM *) g_MemPool.getObject(MEMPOOL_OBJ_DOMNODE);	
	pChainNode->setTag("Input");
	pNode->addChild(pChainNode);
	
	for(j=0; j<m_InputModuleList.size(); j++) {
		m_InputModuleList[j]->getStats(pChainNode);
	}

	pChainNode = (NodeDOM *) g_MemPool.getObject(MEMPOOL_OBJ_DOMNODE);		
	pChainNode->setTag("Output");
	pNode->addChild(pChainNode);

	for(j=0; j<m_OutputModuleList.size(); j++) {
		m_OutputModuleList[j]->getStats(pChainNode);
	}
	
	return gatherStatsExt(pParent);
}

/////////////////////////////////////////////////////////////////////////

bool Adapter::gatherStatsExt (NodeDOM * pParent) {
	return true;
}

/////////////////////////////////////////////////////////////////////////





