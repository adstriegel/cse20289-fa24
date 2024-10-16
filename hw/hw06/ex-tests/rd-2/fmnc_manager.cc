/*
 * fmnc_manager.cc
 *
 *  Created on: Apr 1, 2014
 *      Author: striegel
 */


// For AF_INET and inet_pton
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdexcept>

#include <chrono>
#include <thread>


#include "../util/PracticalSocket.h" 
#include "fmnc_manager.h"
#include "fmnc_connection_tcp_slice.h"

#include "../mon/Thread_Archive.h"
#include "../mon/Thread_Timer.h"

#include "../xml/NodeDOM.h"
#include "../xml/NodeElem.h"

#include "../util/ip-utils.h"
#include "../util/ParamDictionary.h"

#include "fmnc_test_sequence.h"
/* #include "fmnc_measurement_pair.h" */
#include "fmnc_measurement_packet.h"

/////////////////////////////////////////////////////////////////////////

FMNC_ManagerStats::FMNC_ManagerStats () : Stats()
{
	  Allocate(FMNC_MANAGER_STAT_LAST);
}

/////////////////////////////////////////////////////////////////////////

void	FMNC_ManagerStats::Get_Title	(int nStat, char * szTitle)
{
	switch(nStat)
	{
		default:
			sprintf(szTitle, "F%d", nStat);
			break;
	}
}

/////////////////////////////////////////////////////////////////////////

FMNC_Manager::FMNC_Manager ()
{
	// Initialize appropriate values

	m_nPacketID = 1;
	m_pServerAddress = NULL;
	m_pWebSocket = NULL; //lsong

	setPort_Web(7100);
	setPort_XML(1);
	setPort_NI(1);
	setPort_Result(40080);

	m_lSessionID = 0;

	m_pTimer_Cleanup = NULL;

	m_nInterval_Cleanup = FMNC_DEFAULT_INTERVAL_CLEANUP;
	m_nOld_Connection = FMNC_DEFAULT_OLD_CONNECTION;
	m_nDelay_SendFIN = FMNC_DEFAULT_DELAY_FIN;
	m_nTestChoice = FMNC_DEFAULT_TEST_CHOICE;
	m_nDelay_Initial = FMNC_DEFAULT_DELAY_INITIAL;


	// Slice default settings
	m_nSliceSize = FMNC_DEFAULT_SLICE_SIZE;
	m_nSliceSpace_Sec = FMNC_DEFAULT_SLICE_SPACE_SEC;
	m_nSliceSpace_MicroSec = FMNC_DEFAULT_SLICE_SPACE_MICROSEC;

    /* Initialize the clean up thread */
    m_CleanThread = new CleanThread(m_CleanQueue);

}

/////////////////////////////////////////////////////////////////////////

FMNC_Manager::~FMNC_Manager ()
{

}

void		FMNC_Manager::incrementSessionID ()
{
	m_lSessionID++;
}

/////////////////////////////////////////////////////////////////////////

uint32_t	FMNC_Manager::getSessionID()
{
	return m_lSessionID;
}

/////////////////////////////////////////////////////////////////////////

char FMNC_Manager::initialize ()
{
        /* start running the clean up thread */
    m_CleanThread->start();
	enableTimers();

	// Add the timer for the garbage cleanup
	m_pTimer_Cleanup = TimerEvent::createTimer();
	m_pTimer_Cleanup->setData(NULL);
	m_pTimer_Cleanup->setTimerID(FMNC_TIMER_CLEANUP);
	m_pTimer_Cleanup->setInterval_ms(m_nInterval_Cleanup*1000);
	m_pTimer_Cleanup->armTimer();

	if(!addTimerEvent(m_pTimer_Cleanup))
	{
		cerr << "* Error: Unable to add timer for cleanup during initialization" << endl;
		m_pTimer_Cleanup->release();
		m_pTimer_Cleanup = NULL;
		return 0;
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////

uint16_t FMNC_Manager::getPacketID ()
{
	return m_nPacketID;
}

/////////////////////////////////////////////////////////////////////////

void FMNC_Manager::incrementPacketID ()
{
	m_nPacketID++;
}

/////////////////////////////////////////////////////////////////////////

void FMNC_Manager::stampPacketID (Packet * pPacket)
{
	setIP_PacketID(pPacket->getData(), m_nPacketID);
	incrementPacketID();
}

/////////////////////////////////////////////////////////////////////////

void FMNC_Manager::stampTTL 		  (Packet * pPacket)
{
	// TODO: Make this better with less fixed information
	pPacket->getData()[L2_OFFSET+OFFSET_IPV4_TTL] = 64;
}

/////////////////////////////////////////////////////////////////////////

void FMNC_Manager::recomputeChecksums (Packet * pPacket)
{
	updateIPv4_Checksum(pPacket->getData(), pPacket->getLength());
	updateTCP_Checksum(pPacket->getData(), pPacket->getLength());
	updateIPv4_Checksum(pPacket->getData(), pPacket->getLength());
}

bool FMNC_Manager::extractExtendedDOM (NodeDOM * pNode) {
	int		j;
	string	sTag;
	string	sData;

	for(j=0; j<pNode->getNumChildren(); j++) {
		sTag = pNode->getChild(j)->getTag();
		sData = pNode->getChild(j)->getData();

		if(sTag == "serverAddress") {
			setServerAddress(sData);
		}
	}

	return true;
}

string	FMNC_Manager::convertTimerIDtoString (TimerEvent * pEvent)
{
	string		sVal;

	sVal = "";

	switch(pEvent->getTimerID())
	{
		case FMNC_TIMER_START_SLICING:
			sVal = "Timer:StartSlicing";
			break;
		case FMNC_TIMER_DO_SLICING:
			sVal = "Timer:DoSlicing";
			break;
		case FMNC_TIMER_CLEANUP:
			sVal = "Timer:Cleanup";
			break;
		case FMNC_TIMER_SEND_FIN:
			sVal = "Timer:SendFIN";
			break;
		case FMNC_TIMER_RETRANS:
			sVal = "Timer:Retrans";
			break;
		case FMNC_TIMER_FINISH:
			sVal = "Timer:Finish";
			break;
		default:
			sVal = "Timer:Unknown";
			break;
	}

	return sVal;
}

bool FMNC_Manager::processTimer (TimerEvent * pEvent)
{
	//cout << "*** FMNC: Processing a timer of value (" << pEvent->getTimerID() << ") " << convertTimerIDtoString(pEvent) << endl;

	switch(pEvent->getTimerID())
	{
		case FMNC_TIMER_START_SLICING:
			return doTimer_StartSlicing(pEvent);
		case FMNC_TIMER_DO_SLICING:
			return doTimer_Slicing(pEvent);
		case FMNC_TIMER_CLEANUP:
			return doTimer_Cleanup(pEvent);
		case FMNC_TIMER_RETRANS:
			return doTimer_Retrans(pEvent);
		case FMNC_TIMER_SEND_FIN:
			return doTimer_SendFIN(pEvent);
        case FMNC_TIMER_FINISH:
			return doTimer_Finish(pEvent);

	}

	return false;
}
bool FMNC_Manager::doTimer_Finish(TimerEvent * pEvent)
{
        FMNC_Connection* pConn;
        pConn = (FMNC_Connection*) pEvent->getData();
        FMNC_Test_Analysis *	pTestAnalysis;
        pTestAnalysis = new FMNC_Test_Analysis();
        pConn->setTestAnalysis(pTestAnalysis);
        if(pConn->conductAnalysis() && m_Connections.removeConnection(pConn))
        {

                string		sDumpName;
                string		sDumpInfo;
                char			szTemp[40];

                sDumpInfo = pConn->getXML_Summary();

                sprintf(szTemp, "%d-%ld.xml", pConn->getSessionID(), pConn->getTime_Creation()->tv_sec);

                sDumpName = szTemp;

                //TODO:  For Lixing Wed 13 Sep 2017 02:06:25 PM EDT.
                //This is for test purpose
                /* sDumpName = "test.xml"; */

                cout << "File Name: data/fmnc/"<< sDumpName << endl;

                std::string str("data/fmnc/"+ sDumpName);

                /* Add the task to clean */
                CleanTask* p = new CleanTask(str,sDumpInfo);
                m_CleanQueue.add(p);

        }
        else
        {
                cout << "**No connection found. Give up Analysis "<<endl;
                // Get rid of the connection
                /* delete pConn; */
        }

        return true;

}
bool FMNC_Manager::doTimer_Retrans (TimerEvent * pEvent)
{
        
	FMNC_Connection_TCP_Slice *	pSlicedConnection;

	// Create a test sequence
	FMNC_Test_Sequence	*	pTestSequence;

//	cout << "FMNC_Manager::doTimer_Slicing --> Continuing the slicing process" << endl;

	pTestSequence = (FMNC_Test_Sequence *) pEvent->getData();
//	printf("    pTestSequence: 0x%X\n", pTestSequence);

	pSlicedConnection = (FMNC_Connection_TCP_Slice *) pTestSequence->getConnection();
//	printf("    pSlicedConnection: 0x%X\n", pSlicedConnection);


//	cout << "  Continuing the slicing process" << endl;


	// Make a new packet pre-populated from our reference packet.  The reference packet is
	//  saved since we need to track what the inbound / outbound L2 headers are.  We also save
	//  Layer 3 to help us out as well

	Packet 	*	pSlicePacket;

	if(pSlicedConnection->getReferencePacket() == NULL)
	{
		cerr << "* ERROR: Reference packet not set (doTimer_Slicing)" << endl;
		// TODO: Appropriate cleanup
		return false;
	}

	pSlicePacket = pSlicedConnection->getReferencePacket()->cloneFromPool();

    uint32_t a;
    pSlicedConnection->clearOptions_TCP(pSlicePacket);
	pTestSequence->populatePacketWithNextSend(pSlicePacket, &(m_nSliceSpace_MicroSec));
    cout<<"Retran the "<<pTestSequence->getNextToSend()<<endl;

	pSlicePacket->zeroPad(100);

	// Zero out the ACK number since our ACK flag is not set (per Wireshark analysis - and indirectly tcpdump)
	//writeTCP_AckNumber(0, pSlicePacket->getData(), -1);


	writeTCP_AckNumber(pSlicedConnection->getSeqNum_Client(), pSlicePacket->getData(), -1);
	setTCPFlag_ACK (pSlicePacket->getData(), pSlicePacket->getLength(), 1);
	updateTCP_Checksum(pSlicePacket->getData(), pSlicePacket->getLength());

	// TODO: Does timer processing need the timer released?
	pSlicePacket->setArrivalTime(pEvent->getInvocationTime());

	FMNC_Measurement_Packet_TCP	*	pMeasureData;

	pMeasureData = new FMNC_Measurement_Packet_TCP();
	pMeasureData->instrumentPacket(pSlicePacket);

	// Do we add the sequence number to the mix in the metadata?
	pSlicedConnection->getMeasurementPkts_Sent()->addPacket(pMeasureData);

	pSlicedConnection->adjustPackets_Sent(1);
	pSlicedConnection->adjustBytes_Sent(pSlicePacket->getLength());

	// Register a callback to precisely capture when the packet is actually sent out
	pSlicePacket->registerWriteCallback(this, pMeasureData, FMNC_WRITE_CALLBACK_MEASUREMENTPKT);

	// Write that packet out the adapter
	writePacket(pSlicePacket);

	pEvent->release();
	return true;
}
// TODO: This may in fact need to leave the normal timer thread and get its own thread
bool FMNC_Manager::doTimer_Cleanup (TimerEvent * pEvent)
{
	// Scan for any entries that are too old
	if(m_Connections.populateOldList(getAge_PurgeConnection()))
	{
		FMNC_Connection	 *	pConn;

		while( (pConn = m_Connections.popOldEntry()) != NULL)
		{
            // Conduct the analysis
            FMNC_Test_Analysis *	pTestAnalysis;

            pTestAnalysis = new FMNC_Test_Analysis();
            pConn->setTestAnalysis(pTestAnalysis);
            cout << "Start to Analysis "<<endl;
            if(pConn->conductAnalysis())
		    {
                    //cout << "Finish to Analysis "<<endl;

                    // Put it into disk

                    /* ofstream 	theWebFile; */

                    string		sDumpName;
                    string		sDumpInfo;
                    char			szTemp[40];

                    sDumpInfo = pConn->getXML_Summary();

                    sprintf(szTemp, "%d-%ld.xml", pConn->getSessionID(), pConn->getTime_Creation()->tv_sec);

                    sDumpName = szTemp;

                    //TODO:  For Lixing Wed 13 Sep 2017 02:06:25 PM EDT.
                    //This is for test purpose
                    /* sDumpName = "test.xml"; */
                    

                    //NetAddressIPv4 source_ip = *(pConn -> getAddress_Source());
                    //std::string fstr("data/fmnc/"+source_ip.toString());
                    //struct stat st = {0};

                    //if (stat(fstr.c_str(), &st) == -1) {
                    //            mkdir(fstr.c_str(), 0777);

                    //}
                    //cout << "File Name: data/fmnc/"<<source_ip.toString()<<"/"<< sDumpName << endl;
                    cout << "File Name: data/fmnc/"<< sDumpName << endl;

                    std::string str("data/fmnc/"+ sDumpName);
                    //std::string str("data/fmnc/"+source_ip.toString() +"/"+ sDumpName);
                    
                    /* Add the task to clean */
                    CleanTask* p = new CleanTask(str,sDumpInfo);
                    m_CleanQueue.add(p);


                    //D3JS file

                    //  sDumpInfo = pConn->getTestAnalysis()->GetGraphD3JS();

                    //  sprintf(szTemp, "%d-%ld.html", pConn->getSessionID(), pConn->getTime_Creation()->tv_sec);

                    //  sDumpName = szTemp;

                    //  cout << "Graph File Name: data/fmnc/" << sDumpName << endl;

                    //  str = "data/fmnc/" + sDumpName;
                    //  theWebFile.open(str.c_str());

                    //  if(theWebFile.is_open())
                                  //  {
                                  //          theWebFile << sDumpInfo << endl;
                  //          //cout << sDumpInfo << endl;
                  //  }

                  //  theWebFile.close();
		    }
            else
            {
                    cout << " Give up Analysis "<<endl;
            }


			  // Get rid of the connection
			  delete pConn;
		}
	}

	// Re-arm the timer for the next cleanup
	m_pTimer_Cleanup->armTimer();
	if(!addTimerEvent(m_pTimer_Cleanup))
	{
		cerr << "* Error: Unable to add the garbage cleanup timer back into the mix" << endl;
		m_pTimer_Cleanup->release();
		m_pTimer_Cleanup = NULL;
		return false;
	}

	return true;
}

static void* runThread(void* arg)
{
    return ((Thread*)arg)->run();
}

/* The thread class needed for implementing the clean up queue */

Thread::Thread() : m_tid(0), m_running(0), m_detached(0) {}

Thread::~Thread()
{
    if (m_running == 1 && m_detached == 0) {
        pthread_detach(m_tid);
    }
    if (m_running == 1) {
        pthread_cancel(m_tid);
    }
}

int Thread::start()
{
    int result = pthread_create(&m_tid, NULL, runThread, this);
    if (result == 0) {
        m_running = 1;
    }
    return result;
}

int Thread::join()
{
    int result = -1;
    if (m_running == 1) {
        result = pthread_join(m_tid, NULL);
        if (result == 0) {
            m_detached = 0;
        }
    }
    return result;
}

int Thread::detach()
{
    int result = -1;
    if (m_running == 1 && m_detached == 0) {
        result = pthread_detach(m_tid);
        if (result == 0) {
            m_detached = 1;
        }
    }
    return result;
}

pthread_t Thread::self() {
    return m_tid;
}
void CleanTask::clean()
{
        ofstream theOutFile;
        theOutFile.open(m_filename.c_str());
        if(theOutFile.is_open())
        {
                theOutFile << m_xml << endl;
                cout<<" The filename "<<m_filename<<" The string "<<m_xml.length()<<endl;
                
        }
        theOutFile.close();
        //Call the parsing script and output the file
        /* cout<<"Output to file"<<endl; */
        /* std::stringstream stream; */
        /* stream <<"/home/lsong2/ScaleBox/src/data/fmnc/goResult.sh "<<m_filename; */
        /* system(stream.str().c_str()); */
}

string exec(const char* cmd) 
{
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

bool check_port_traffic()
{
    string re = exec("iptables -nvxL|grep 'dpt:7100'|awk '{print $1}'");
    uint32_t traffic_before = atoi(re.c_str());
    re = exec("iptables -nvxL|grep 'dpt:7000'|awk '{print $1}'");
    uint32_t traffic_before1 = atoi(re.c_str());
    std::this_thread::sleep_for (std::chrono::seconds(1));
    re = exec("iptables -nvxL|grep 'dpt:7100'|awk '{print $1}'");
    uint32_t traffic_after = atoi(re.c_str());
    re = exec("iptables -nvxL|grep 'dpt:7000'|awk '{print $1}'");
    uint32_t traffic_after1 = atoi(re.c_str());
    if(traffic_after - traffic_before + traffic_after1 - traffic_before1 > 5)
            return true;
    else
            return false;
}

bool FMNC_Manager::doTimer_StartSlicing (TimerEvent * pEvent)
{
	FMNC_Connection_TCP_Slice *	pSlicedConnection;

	// We know the data payload is the connection that we are associated with
	pSlicedConnection = (FMNC_Connection_TCP_Slice *) pEvent->getData();

//	cout << "  Starting the slicing process" << endl;

	// Create a test sequence
	FMNC_Test_Sequence	*	pTestSequence;


	pTestSequence = createTestSequence_PreDef(pSlicedConnection);



	// The connection is "hot" now for data
	pSlicedConnection->setState(FMNC_TCP_STATE_ACTIVE);

    while(check_port_traffic())
            cout<<"Waiting Cross Traffic to be finished......."<<endl;
	// Adjust the packet timing
	TimerEvent 	*	pFirstTimer;

	pFirstTimer = TimerEvent::createTimer();
	pFirstTimer->setData(pTestSequence);
	pFirstTimer->setTimerID(FMNC_TIMER_DO_SLICING);
	pFirstTimer->setInterval_ms(0);
	pFirstTimer->armTimer();

	if(!addTimerEvent(pFirstTimer))
	{
		cerr << "* Error: Unable to add timer" << endl;
		pFirstTimer->release();
		pEvent->release();
		return false;
	}

	pEvent->release();
	return true;
}


bool FMNC_Manager::writePacket (Packet * pPacketToWrite)
{
	stampTTL(pPacketToWrite);
	stampPacketID(pPacketToWrite);
	recomputeChecksums(pPacketToWrite);

	// Write that packet out the adapter
	if(!writeBackViaTap(FMNC_TAP_WAN, pPacketToWrite))
	{
		// Ruh roh!
		cerr << "* Error: Failed to write back via the tap" << endl;
		pPacketToWrite->release();
		return false;
	}
       // cout << " Examine the sent packet flag" << endl;
       // cout<< unsigned( getTCP_Flags(pPacketToWrite->getData()))<<endl;

	return true;
}


bool FMNC_Manager::doTimer_Slicing (TimerEvent * pEvent)
{
	FMNC_Connection_TCP_Slice *	pSlicedConnection;

	// Create a test sequence
	FMNC_Test_Sequence	*	pTestSequence;

//	cout << "FMNC_Manager::doTimer_Slicing --> Continuing the slicing process" << endl;

	pTestSequence = (FMNC_Test_Sequence *) pEvent->getData();
//	printf("    pTestSequence: 0x%X\n", pTestSequence);

	pSlicedConnection = (FMNC_Connection_TCP_Slice *) pTestSequence->getConnection();
//	printf("    pSlicedConnection: 0x%X\n", pSlicedConnection);


//	cout << "  Continuing the slicing process" << endl;


	// Make a new packet pre-populated from our reference packet.  The reference packet is
	//  saved since we need to track what the inbound / outbound L2 headers are.  We also save
	//  Layer 3 to help us out as well

	Packet 	*	pSlicePacket;

	if(pSlicedConnection->getReferencePacket() == NULL)
	{
		cerr << "* ERROR: Reference packet not set (doTimer_Slicing)" << endl;
		// TODO: Appropriate cleanup
		return false;
	}

	pSlicePacket = pSlicedConnection->getReferencePacket()->cloneFromPool();

    pSlicedConnection->clearOptions_TCP(pSlicePacket);
    /* We want to keep timestamp as the reference to infer loss. */
	/* pSlicedConnection->clearOptions_TCP_TsOptOn(pSlicePacket); */
	pTestSequence->populatePacketWithNextSend(pSlicePacket, &(m_nSliceSpace_MicroSec));

	pSlicePacket->zeroPad(100);

	// Zero out the ACK number since our ACK flag is not set (per Wireshark analysis - and indirectly tcpdump)
	//writeTCP_AckNumber(0, pSlicePacket->getData(), -1);


	writeTCP_AckNumber(pSlicedConnection->getSeqNum_Client(), pSlicePacket->getData(), -1);
	setTCPFlag_ACK (pSlicePacket->getData(), pSlicePacket->getLength(), 1);
	updateTCP_Checksum(pSlicePacket->getData(), pSlicePacket->getLength());
    /* Write timestamp with TSval as seq number */
    /* writeTCP_TimestampOption(pSlicedConnection->getPackets_Sent(), 0, pSlicePacket->getData(), -1); */

	// TODO: Does timer processing need the timer r
	pSlicePacket->setArrivalTime(pEvent->getInvocationTime());

	FMNC_Measurement_Packet_TCP	*	pMeasureData;

	pMeasureData = new FMNC_Measurement_Packet_TCP();
	pMeasureData->instrumentPacket(pSlicePacket);

    pSlicedConnection->UpdateExp_ACK(getTCP_SeqNumber(pSlicePacket->getData(),pSlicePacket->getLength()));

	// Do we add the sequence number to the mix in the metadata?
	pSlicedConnection->getMeasurementPkts_Sent()->addPacket(pMeasureData);

	pSlicedConnection->adjustPackets_Sent(1);
	pSlicedConnection->adjustBytes_Sent(pSlicePacket->getLength());

	// Register a callback to precisely capture when the packet is actually sent out
	pSlicePacket->registerWriteCallback(this, pMeasureData, FMNC_WRITE_CALLBACK_MEASUREMENTPKT);

	// Write that packet out the adapter
	writePacket(pSlicePacket);
    
	if(!pTestSequence->isDone())
	{
		TimerEvent 	*	pNextTimer;

		pNextTimer = TimerEvent::createTimer();
		pNextTimer->setData(pTestSequence);
		pNextTimer->setTimerID(FMNC_TIMER_DO_SLICING);
        /* if (pTestSequence->getNextToSend() % 25 == 0){ */
        /*         pNextTimer->setInterval_us(40000); */
        /* } */
        /* else */
                pNextTimer->setInterval_us(m_nSliceSpace_MicroSec);
        //else if (pTestSequence->getNextToSend() < 61)
        //        pNextTimer->setInterval_us(100);
        //else if (pTestSequence->getNextToSend() < 71)
        //        pNextTimer->setInterval_us(140);
        //else if (pTestSequence->getNextToSend() < 81)
        //        pNextTimer->setInterval_us(206);
        //else if (pTestSequence->getNextToSend() < 91)
        //        pNextTimer->setInterval_us(417);
        //else
        //        pNextTimer->setInterval_us(2107);
        

        /* cout<<"The spacing at manager "<<m_nSliceSpace_MicroSec<<"size "<<pSlicePacket->getLength()<<endl; */

		pNextTimer->armTimer();

		if(!addTimerEvent(pNextTimer))
		{
			cerr << "* Error: Unable to add the timer for the next iteration for the test sequence" << endl;
			pNextTimer->release();
			pEvent->release();
			return false;
		}
	}
	else
	{
		// Set the state that we are waiting for packets
		pSlicedConnection->setState(FMNC_TCP_STATE_WAITING);

		// Adjust the packet timing
		TimerEvent 	*	pTimerFIN;

		pTimerFIN = TimerEvent::createTimer();
		pTimerFIN->setData(pTestSequence);
		pTimerFIN->setTimerID(FMNC_TIMER_SEND_FIN);
		pTimerFIN->setInterval_ms(getDelay_SendFIN());
		pTimerFIN->armTimer();

		if(!addTimerEvent(pTimerFIN))
		{
			cerr << "* Error: Unable to add timer for sending the FIN packet" << endl;
			pTimerFIN->release();
			return false;
		}
	}

	pEvent->release();
	return true;
}


bool FMNC_Manager::sendPacket_PureACK (FMNC_Connection_TCP_Slice * pSlicedConnection, uint32_t lSeqNum, uint32_t lAckNum, string sMeta)
{
	// Send the pure ACK packet
	Packet * pACK_Packet;

	pACK_Packet = pSlicedConnection->getReferencePacket()->cloneFromPool();

	// This is a zero byte FIN packet - no length, nothing
	//pACK_Packet->setLength(OFFSET_IPV4_DATA);
	//setPacketLength(pACK_Packet->getData(), L3_IPV4_OFFSET+L4_TCP_LENGTH_NO_OPTIONS);

    pSlicedConnection->clearOptions_TCP(pACK_Packet);
	clearTCP_Flags(pACK_Packet->getData());

	setTCPFlag_ACK (pACK_Packet->getData(), pACK_Packet->getLength(), 1);

	writeTCP_AckNumber(lAckNum, pACK_Packet->getData(), pACK_Packet->getLength());
	writeTCP_SeqNumber(lSeqNum, pACK_Packet->getData(), pACK_Packet->getLength());

	pACK_Packet->forceUnusedZero();

	updateTCP_Checksum(pACK_Packet->getData(), pACK_Packet->getLength());

	FMNC_Measurement_Packet_TCP	*	pMeasureData;

	pMeasureData = new FMNC_Measurement_Packet_TCP();
	pMeasureData->instrumentPacket(pACK_Packet);
	pMeasureData->setMetaData(sMeta);

	// Do we add the sequence number to the mix in the metadata?
	pSlicedConnection->getMeasurementPkts_Setup()->addPacket(pMeasureData);
	pACK_Packet->registerWriteCallback(this, pMeasureData, FMNC_WRITE_CALLBACK_MEASUREMENTPKT);

	// Write that packet out the adapter
	writePacket(pACK_Packet);

	return true;
}

bool FMNC_Manager::sendPacket_RST (FMNC_Connection_TCP_Slice * pSlicedConnection, FMNC_Test_Sequence *	pTestSequence)
{
	//mimic the FIN implementation
	Packet * pRST_Packet;

	pRST_Packet = pSlicedConnection->getReferencePacket()->cloneFromPool();

    pSlicedConnection->clearOptions_TCP(pRST_Packet);

	setTCPFlag_ACK (pRST_Packet->getData(), pRST_Packet->getLength(), 1);
	setTCPFlag_RST (pRST_Packet->getData(), pRST_Packet->getLength(), 1);
    //cout<<"FIN Packet size: "<<pFIN_Packet->getLength()<<endl;

    writeTCP_AckNumber(pSlicedConnection->getSeqNum_Client()+1, pRST_Packet->getData(), pRST_Packet->getLength());

	writeTCP_SeqNumber(pTestSequence->getTestLength()+pSlicedConnection->getSeqNum_Server(), pRST_Packet->getData(), pRST_Packet->getLength());
	pRST_Packet->forceUnusedZero();

	updateTCP_Checksum(pRST_Packet->getData(), pRST_Packet->getLength());

	FMNC_Measurement_Packet_TCP	*	pMeasureData;

	pMeasureData = new FMNC_Measurement_Packet_TCP();
	pMeasureData->instrumentPacket(pRST_Packet);
	pMeasureData->setMetaData("RST-Server");

	// Do we add the sequence number to the mix in the metadata?
	pSlicedConnection->getMeasurementPkts_Setup()->addPacket(pMeasureData);
	pRST_Packet->registerWriteCallback(this, pMeasureData, FMNC_WRITE_CALLBACK_MEASUREMENTPKT);


	// Write that packet out the adapter
	writePacket(pRST_Packet);

	return true;
}
bool FMNC_Manager::sendPacket_FIN (FMNC_Connection_TCP_Slice * pSlicedConnection, FMNC_Test_Sequence *	pTestSequence, bool bRespondFIN)
{
	// Send the FIN packet (if we should)
	Packet * pFIN_Packet;

	pFIN_Packet = pSlicedConnection->getReferencePacket()->cloneFromPool();

    pSlicedConnection->clearOptions_TCP(pFIN_Packet);

	// This is a zero byte FIN packet - no length, nothing
	//pFIN_Packet->setLength(OFFSET_IPV4_DATA);
	//setPacketLength(pFIN_Packet->getData(), L3_IPV4_OFFSET+L4_TCP_LENGTH_NO_OPTIONS);//lsong: this is critical

	setTCPFlag_ACK (pFIN_Packet->getData(), pFIN_Packet->getLength(), 1);
	setTCPFlag_FIN (pFIN_Packet->getData(), pFIN_Packet->getLength(), 1);
    //cout<<"FIN Packet size: "<<pFIN_Packet->getLength()<<endl;

	if(bRespondFIN)
	{
		writeTCP_AckNumber(pSlicedConnection->getSeqNum_Client()+1, pFIN_Packet->getData(), pFIN_Packet->getLength());
	}
	else
	{
		writeTCP_AckNumber(pSlicedConnection->getSeqNum_Client(), pFIN_Packet->getData(), pFIN_Packet->getLength());
	}

	writeTCP_SeqNumber(pTestSequence->getTestLength()+pSlicedConnection->getSeqNum_Server(), pFIN_Packet->getData(), pFIN_Packet->getLength());
	pFIN_Packet->forceUnusedZero();

	updateTCP_Checksum(pFIN_Packet->getData(), pFIN_Packet->getLength());

	FMNC_Measurement_Packet_TCP	*	pMeasureData;

	pMeasureData = new FMNC_Measurement_Packet_TCP();
	pMeasureData->instrumentPacket(pFIN_Packet);
	pMeasureData->setMetaData("FIN-Server");

	// Do we add the sequence number to the mix in the metadata?
	pSlicedConnection->getMeasurementPkts_Setup()->addPacket(pMeasureData);
	pFIN_Packet->registerWriteCallback(this, pMeasureData, FMNC_WRITE_CALLBACK_MEASUREMENTPKT);


	// Write that packet out the adapter
	writePacket(pFIN_Packet);

	return true;
}

bool FMNC_Manager::doTimer_SendFIN (TimerEvent * pEvent)
{
	FMNC_Connection_TCP_Slice *	pSlicedConnection;
	FMNC_Test_Sequence		  *	pTestSequence;

	cout << "FMNC_Manager::doTimer_SendFIN --> Wrapping up the process" << endl;

	pTestSequence = (FMNC_Test_Sequence *) pEvent->getData();
	pSlicedConnection = (FMNC_Connection_TCP_Slice *) pTestSequence->getConnection();

	// Make a new packet pre-populated from our reference packet.  The reference packet is
	//  saved since we need to track what the inbound / outbound L2 headers are.  We also save
	//  Layer 3 to help us out as well

	if(pSlicedConnection->getReferencePacket() == NULL)
	{
		cerr << "* ERROR: Reference packet not set (doTimer_SendFIN)" << endl;
		// TODO: Appropriate cleanup
		pEvent->release();
		return false;
	}

	// Set the state that we are waiting for packets
	pSlicedConnection->setState(FMNC_TCP_STATE_WAITING_FIN_ACK);

	sendPacket_FIN(pSlicedConnection, pTestSequence, false);

	pEvent->release();
	return true;
}

NodeDOM * FMNC_Manager::getStats (NodeDOM * pRoot)
{
	return NULL;
}

CommandResult FMNC_Manager::processExtCommand (const vector<string> & theCommands, int nOffset)
{
	if(theCommands.size() <= nOffset) {
		return PROCESS_CMD_SYNTAX;
	}

//	cout << " Processing a command at the TWiCE module" << endl;

	if(theCommands[nOffset] == "help") {

		cout << " FMNC Module Help" << endl;
		cout << endl;
		cout << "Status Requests:   Current state of the manager" << endl;
		cout << "   Connections      Show the current connections open / active via the manager" << endl;
		cout << endl;
		cout << "Settings: In most cases, leave off the value display the current setting" << endl;
		cout << "          One can also write help in some cases for more information" << endl;
		cout << endl;
		cout << "   TestChoice X     Change the default test choice to use X" << endl;
		cout << "   SliceSize  X     Change the base slice size to X bytes for the payload size" << endl;
		cout << "   DebugWebRequest  Display the normal (base) test sequence for testing " << endl;

		return PROCESS_CMD_SUCCESS;
	}
	else if(theCommands[nOffset] == "Connections")
	{
		m_Connections.displayToConsole();
	}
	else if(theCommands[nOffset] == "TestChoice")
	{
		if(theCommands.size() < nOffset+1)
		{
			cout << " Current test choice is " << getTestChoice() << endl;
			return PROCESS_CMD_SUCCESS;
		}
		else if (theCommands[nOffset+1] == "help")
		{
			cout << " Help: TestChoice" << endl;
			cout << endl;
			cout << " The TestChoice setting represents the base pre-defined test that is selected whenever unspecified by a client" << endl;
			return PROCESS_CMD_SUCCESS;
		}
		else
		{
			int		nTest;

			nTest = atoi(theCommands[nOffset+1].c_str());

			if(nTest >= 0)
			{
				setTestChoice(nTest);
			}

			cout << "  Modified default test choice to test " << nTest << endl;
			return PROCESS_CMD_SUCCESS;
		}
	}
	else if(theCommands[nOffset] == "SliceSize")
	{
		if(theCommands.size() < nOffset+1)
		{
			cout << " Current slice size is " << getSliceSize() << endl;
			return PROCESS_CMD_SUCCESS;
		}
		else if (theCommands[nOffset+1] == "help")
		{
			cout << " Help: SliceSize" << endl;
			cout << endl;
			cout << " The SliceSize represents the maximum payload size allowed for slices (aka the MSS)" << endl;
			return PROCESS_CMD_SUCCESS;
		}
		else
		{
			int		nTest;

			nTest = atoi(theCommands[nOffset+1].c_str());

			if(nTest > 0)
			{
				setSliceSize(nTest);
			}
			else
			{
				cout << " Error: Slice size must be larger than zero" << endl;
			}

			cout << " New slice size is " << getSliceSize() << endl;
			return PROCESS_CMD_SUCCESS;
		}
	}
	else if (theCommands[nOffset] == "DebugWebRequest")
	{
		FMNC_Test_Sequence *	pTestSequence;

		pTestSequence = new FMNC_Test_Sequence();

		pTestSequence->setConnection(NULL);

		// Set up the default parameters for slicing / spacing
		pTestSequence->setSliceSize(getSliceSize());
		pTestSequence->setSliceSpacing(getSliceSpacing_Sec(), getSliceSpacing_MicroSec());

		pTestSequence->setTestConfig("<Predefined WebDirect=\"base\" Fixed=\"50\" Multi=\"10\" Loops=\"2\" />");
		pTestSequence->createTest_WebDemo (50,2,10,100);

		if(theCommands.size() < nOffset+2)
		{
			pTestSequence->getTransmitBuffer()->dump_TransmitBuffer(false);
		}
		else
		{
			if (theCommands[nOffset+1] == "help")
			{
				cout << "The DebugWebRequest command allows for one to dump the likely web request" << endl;
				cout << "content that is generated in the base case as well as the resulting test sequence" << endl;
				cout << endl;
				cout << "  DebugWebRequest on its own will dump just high level information" << endl;
				cout << "  DebugWebRequest Yes    will dump the content of the actual transmit buffers" << endl;
				return PROCESS_CMD_SUCCESS;
			}
			else
			{
				pTestSequence->getTransmitBuffer()->dump_TransmitBuffer(true);
			}
		}

		pTestSequence->dump_TransmitSequence();

		delete pTestSequence;

		return PROCESS_CMD_SUCCESS;
	}


	return PROCESS_CMD_UNKNOWN;
}

bool FMNC_Manager::processWriteCallback (Packet * pPacket, void * pData, int nType)
{
	if(pData != NULL)
	{
		switch(nType)
		{
			case FMNC_WRITE_CALLBACK_MEASUREMENTPKT:
				if(pData != NULL)
				{
					FMNC_Measurement_Packet *	pMeasureInfo;

					pMeasureInfo = (FMNC_Measurement_Packet *) pData;

					// The arrival time of the packet on a write callback contains the actual time
					// that the packet was written
					pMeasureInfo->setTime(pPacket->getArrivalTime());
					return true;
				}
				else
				{
					cout << "* Warning: Expected a valid piece of data when parsing a FMNC write callback, data was NULL, ignoring ...." << endl;
					return false;
				}

			default:
				cerr << "* Error: Unknown type on write callback (" << nType << ")" << endl;
				return false;
		}
	}

	return true;
}

bool FMNC_Manager::processWriteCallback (Packet * pPacket, void * pData) {
//	cout << " * Processing a write callback" << endl;

//	cout << "    L2: ";
//	pPacket->showHex(0, L2_OFFSET);
//	cout << endl;

//	cout << "    L3: ";
//	pPacket->showHex(L2_OFFSET, 20+L2_OFFSET);
//	cout << endl;

//	cout << "    L4: ";
//	pPacket->showHex(34, 20+14+20);
//	cout << endl;

//	dumpTCP_brief (pPacket->getData());

	//cout << "    Payload: ";
	//pPacket->showText(54, pPacket->getLength());
	//cout << endl;

	return true;
}


char FMNC_Manager::processTapPacket (Packet * pPacket, int nID)
{
	switch(nID)
    {
    	case FMNC_TAP_WAN:
    		return processPacket_WAN(pPacket);
    }

	return 0;
}

int FMNC_Manager::mapNameToID (string sName)
{

  if(sName == "WAN")
    {
      return FMNC_TAP_WAN;
    }
  else
    {
      return FMNC_TAP_NONE;
    }
}

char FMNC_Manager::isNewConnection (Packet * pPacket)
{
	//
	return 1;
}

char FMNC_Manager::mapConnectionTypeFromPort (Packet * pPacket)
{
	// Get the inbound port number (Destination)
	unsigned short	nDestPort;

	nDestPort = getTCP_DstPort(pPacket->getData());

	if(nDestPort == getPort_NI())
	{
		return FMNC_CONNTYPE_NI;
	}
	else if(nDestPort == getPort_Web())
	{
		return FMNC_CONNTYPE_WEB;
	}
	else if(nDestPort == getPort_XML())
	{
		return FMNC_CONNTYPE_XML;
	}
	else
	{
		return FMNC_CONNTYPE_UNKNOWN;;
	}
}

char FMNC_Manager::shouldAcceptNewConnection (char byType)
{
	return 1;
}

char FMNC_Manager::handleTCP_Existing (FMNC_Connection * pConnection, Packet * pPacket)
{
	// What state are we in?
	FMNC_Connection_TCP_Slice *	pSlicedConnection;
	int		nPayloadSize;

    TimerEvent 	*	pTimer = NULL;
    FMNC_Measurement_Pair * pPair = NULL;
	pSlicedConnection = (FMNC_Connection_TCP_Slice * ) pConnection;

	// Do a double check that indeed the connection is valid
	if(pConnection == NULL)
	{
		cerr << "* Error: handleTCP_Existing call had a NULL connection" << endl;
		return PROCESSPKT_RESULT_CONTINUE;
	}

	// Mark the connection as having one more packet
	pSlicedConnection->adjustPackets_Received(1);
	pSlicedConnection->adjustBytes_Received(pPacket->getLength());
	pSlicedConnection->setTime_LastUsed(pPacket->getArrivalTime());

	// Make sure this packet does not have the SYN flag set.  If it does, we are in
	//  trouble
	if(getTCPFlag_SYN(pPacket->getData(), pPacket->getLength()))
	{
		cerr << "* Error: Unexpected SYN later than expected" << endl;
		m_Stats.Add_Stat(FMNC_MANAGER_COUNT_UNEXPECTED_SYN, 1);
		return PROCESSPKT_RESULT_CONTINUE;
	}

	//cout << "* TCP-EXISTING: Handling existing TCP connection" << endl;

	// How big is the payload (data portion) of the inbound TCP packet?
	nPayloadSize = getTCP_PayloadSize(pPacket->getData());
	m_Stats.Add_Stat(FMNC_MANAGER_KNOWN_TCP_CONN_VOLUME_PAYLOAD, nPayloadSize);

	// What do we think is the state of the connection?
	switch(pSlicedConnection->getState())
	{
		// We just sent the SYN-ACK to the client and are waiting for the likely
		//  zero data ACK from the client
		case FMNC_TCP_STATE_SYNACK:
			// Looking for a zero length TCP ACK of our ACK number
			if(getTCPFlag_ACK(pPacket->getData(), pPacket->getLength()))
			{
				// Process this packet as part of the setup
				FMNC_List_Measurement_Packets	*	pMeasureList;

				pMeasureList = pSlicedConnection->getMeasurementPkts_Setup();

				FMNC_Measurement_Packet_TCP * pMeasurePkt;

				pMeasurePkt = new FMNC_Measurement_Packet_TCP();
				pMeasurePkt->instrumentPacket(pPacket);
				pMeasurePkt->appendMetaData("Ack3Way");
				pMeasureList->addPacket(pMeasurePkt);

				// The client is now active and engaged
				pSlicedConnection->setState(FMNC_TCP_STATE_WAITDATA);

				// Did they send us data with the ACK?
				if(nPayloadSize == 0)
				{
					// This is what we are looking for
				}
				else
				{
					// TODO: Deal with it when the first ACK has a payload
					m_Stats.Add_Stat(FMNC_MANAGER_COUNT_HANDSHAKE_ACK_HADPAYLOAD, 1);
				}

				// Let the packet get eaten later on the in module processing

				pPacket->release();
				return PROCESSPKT_RESULT_CONSUMED;
			}
			else
			{
				cerr << "* Error: Client did not ACK after the SYN-ACK" << endl;
				m_Stats.Add_Stat(FMNC_MANAGER_COUNT_HANDSHAKE_NO_ACK, 1);
				pPacket->release();
				return PROCESSPKT_RESULT_CONSUMED;
			}
			break;

		// We are waiting for the first piece of data from the client
		case FMNC_TCP_STATE_WAITDATA:

			cout << "** WaitData: Received a packet with a payload of an estimated " << getPacketLength(pPacket->getData(), pPacket->getLength()) - 40 << " bytes" << endl;

			/* cout <<"  Data dump: "; */
			/* dumpData_ASCII(pPacket->getData()+L2_OFFSET+40, 0, getPacketLength(pPacket->getData(), pPacket->getLength()) - 40); */
			/* cout << endl; */

			// Let's ACK this particular packet
			Packet 	*	pResponsePacket;

			// Capture / retrieve the object request (web)
			pSlicedConnection->setRequest(getWeb_Request(pPacket));

            pSlicedConnection->clearOptions_TCP(pPacket);
            /* pSlicedConnection->clearOptions_TCP_TsOptOn(pPacket); */
			pResponsePacket = pSlicedConnection->respondToData(pPacket);

			// The original packet is ours, we need to eat it
			pPacket->release();

			// Send the response packet
			if(pResponsePacket != NULL)
			{
				// The write stack will handle the packet from here on out
				pResponsePacket->registerWriteCallback(this, NULL);
				writePacket(pResponsePacket);
			}
			else
			{
				cerr << "* Error: Response packet creation for the data packet failed!" << endl;
				return PROCESSPKT_RESULT_CONSUMED;
			}

			// We got a data packet, now let us get things ready to rock and roll
			pSlicedConnection->setState(FMNC_TCP_STATE_ACTIVE);

			// Time to do some sending / engagement of the timer

			/* TimerEvent 	*	pTimer; */
			pTimer = TimerEvent::createTimer();
			pTimer->setTimerID(FMNC_TIMER_START_SLICING);
			pTimer->setData(pSlicedConnection);

			// How long until we start things?
			pTimer->setInterval_ms(getDelay_Initial());
			pTimer->repeatInterval();

			if(!addTimerEvent(pTimer))
			{
				cerr << "* Error: Unable to add timer" << endl;
				pTimer->release();
				pTimer = NULL;
			}

			return PROCESSPKT_RESULT_CONSUMED;


		// Active means that we are in the process of sending sliced / rearranged packets
		// Waiting means that we finished our sequence, getting ACKs, but have not yet sent the FIN
		case FMNC_TCP_STATE_ACTIVE:
		case FMNC_TCP_STATE_WAITING:
            
			// Instrument the received packet
			FMNC_Measurement_Packet_TCP	*	pMeasureAck;

			pMeasureAck = new FMNC_Measurement_Packet_TCP();
			pMeasureAck->instrumentPacket(pPacket);

			if(pSlicedConnection->getState() == FMNC_TCP_STATE_WAITING)
			{
				pMeasureAck->setMetaData("SEQ-DONE");
			}
			else
			{
				pMeasureAck->setMetaData("ACTIVE");
			}

            if(getTCPFlag_PSH(pPacket->getData(), -1))
            {
                    pSlicedConnection->getMeasurementPkts_Setup()->addPacket(pMeasureAck);
            }
            else
            {
                    pSlicedConnection->getMeasurementPkts_Received()->addPacket(pMeasureAck);
                    //TODO: The threshold hold for pre RTT is pending to finalize
                    //For Lixing Sat 17 Sep 2016 11:11:29 AM EDT.
                    
            }

            
            /* cout<<"(DEBUG) Check Ack "<<getTCP_AckNumber(pPacket->getData(),-1)<<" / "<<pSlicedConnection->getExp_ACK()<<endl; */
            //TODO: Adaptive Train
            //For Lixing Tue 17 Oct 2017 05:43:33 PM EDT.
            /* if( getTCP_AckNumber(pPacket->getData(),-1) > pSlicedConnection->getExp_ACK() && false &&// The bool value here is used to toggle adaptive trans. */
            /*                 pSlicedConnection->getRequest().compare(7,5,"train") == 0 && */
            /*                 pSlicedConnection->getRequest().compare(12,9,"?Cellular") == 0){ */

            /*         FMNC_Test_Analysis *	pTestAnalysis; */
            /*         pTestAnalysis = new FMNC_Test_Analysis(); */
            /*         pSlicedConnection->setTestAnalysis(pTestAnalysis); */
            /*         bool cont_probe = pSlicedConnection->hiccup_Analysis(); */
            /*         if(cont_probe){ // restrict the data cost within 1.5M */
            /*                 pSlicedConnection->setSeqNum_Server(getTCP_AckNumber(pPacket->getData(),-1)); */
            /*                 cout<<"(DEBUG) Let me send another train when ACK received "<<getTCP_AckNumber(pPacket->getData(),-1)<<endl; */
            /*                 cout<<"(DEBUG) The base seq is "<<pSlicedConnection->getSeqNum_Server()<<endl; */

            /*                 // We got a data packet, now let us get things ready to rock and roll */
            /*                 pSlicedConnection->setState(FMNC_TCP_STATE_ACTIVE); */

            /*                 // Time to do some sending / engagement of the timer */
            /*                 /1* TimerEvent 	*	pTimer; *1/ */
            /*                 pTimer = TimerEvent::createTimer(); */
            /*                 pTimer->setTimerID(FMNC_TIMER_START_SLICING); */
            /*                 pTimer->setData(pSlicedConnection); */

            /*                 // How long until we start things? */
            /*                 pTimer->setInterval_ms(200); */
            /*                 pTimer->repeatInterval(); */

            /*                 if(!addTimerEvent(pTimer)) */
            /*                 { */
            /*                         cerr << "* Error: Unable to add timer" << endl; */
            /*                         pTimer->release(); */
            /*                         pTimer = NULL; */
            /*                 } */

            /*         }else{ */


            /*                 // Adjust the packet timing */
            /*                 TimerEvent 	*	pTimerFIN; */
            /*                 FMNC_Test_Sequence *	pTestSequence; */
            /*                 pTestSequence = new FMNC_Test_Sequence(); */
            /*                 pTestSequence->setConnection(pSlicedConnection); */

            /*                 pTimerFIN = TimerEvent::createTimer(); */
            /*                 pTimerFIN->setData(pTestSequence); */
            /*                 pTimerFIN->setTimerID(FMNC_TIMER_SEND_FIN); */
            /*                 pTimerFIN->setInterval_ms(getDelay_SendFIN()); */
            /*                 pTimerFIN->armTimer(); */

            /*                 if(!addTimerEvent(pTimerFIN)) */
            /*                 { */
            /*                         cerr << "* Error: Unable to add timer for sending the FIN packet" << endl; */
            /*                         pTimerFIN->release(); */
            /*                         return false; */
            /*                 } */
            /*         } */

            /* }else if( getTCP_AckNumber(pPacket->getData(),-1) > pSlicedConnection->getExp_ACK() && */
            /*                 !(pSlicedConnection->getRequest().compare(7,5,"train") == 0 && false && // The bool value here is used to toggle adaptive trans. */
            /*                  pSlicedConnection->getRequest().compare(12,9,"?Cellular") == 0)){ */


            /*         cout<<"Send FINNNNNNNNNNNNNNNNNNNN"<<endl; */
            /*         // Adjust the packet timing */
            /*         TimerEvent 	*	pTimerFIN; */
            /*         FMNC_Test_Sequence *	pTestSequence; */
            /*         pTestSequence = new FMNC_Test_Sequence(); */
            /*         pTestSequence->setConnection(pSlicedConnection); */

            /*         pTimerFIN = TimerEvent::createTimer(); */
            /*         pTimerFIN->setData(pTestSequence); */
            /*         pTimerFIN->setTimerID(FMNC_TIMER_SEND_FIN); */
            /*         pTimerFIN->setInterval_ms(getDelay_SendFIN()); */
            /*         pTimerFIN->armTimer(); */

            /*         if(!addTimerEvent(pTimerFIN)) */
            /*         { */
            /*                 cerr << "* Error: Unable to add timer for sending the FIN packet" << endl; */
            /*                 pTimerFIN->release(); */
            /*                 return false; */
            /*         } */
            /* } */



            // We are eating the packet (none shall pass)
            pPacket->release();

			return PROCESSPKT_RESULT_CONSUMED;

		case FMNC_TCP_STATE_WAITING_FIN_ACK:

            pMeasureAck = new FMNC_Measurement_Packet_TCP();
            pMeasureAck->instrumentPacket(pPacket);

            pSlicedConnection->IncFINACKcounter();
            if(getTCPFlag_FIN(pPacket->getData(), -1) && pSlicedConnection->getFINACKcounter() < FMNC_MAX_NUM_FIN_ACK)
            {
                    cout<<"Waiting FIN ACK: No! Counter: "<<pSlicedConnection->getFINACKcounter()<<" Sending FIN ACK back ? "<<int( pSlicedConnection->getFINACKcounter() < FMNC_MAX_NUM_FIN_ACK )<<endl;
                    pMeasureAck->setMetaData("FIN-Client");
                    pSlicedConnection->getMeasurementPkts_Setup()->addPacket(pMeasureAck);

                    // Let us ACK them back to complete the cycle at Client SN + 1
                    sendPacket_PureACK(pSlicedConnection, pSlicedConnection->getSeqNum_Server()+pSlicedConnection->getTestSequence()->getTestLength(), pSlicedConnection->getSeqNum_Client()+1, "ACK-Client-FIN");

                    sendPacket_FIN (pSlicedConnection, pSlicedConnection->getTestSequence(), true);

            }
            else
            {
                    cout<<"Waiting FIN ACK: Yes! Counter:"<<pSlicedConnection->getFINACKcounter()<<"Sending RESET back ?"<<int( pSlicedConnection->getFINACKcounter() > 2*FMNC_MAX_NUM_FIN_ACK )<<endl; 
                    if(pSlicedConnection->getFINACKcounter() > 2*FMNC_MAX_NUM_FIN_ACK &&
                                    pSlicedConnection->getFINACKcounter() < 3*FMNC_MAX_NUM_FIN_ACK ){

                            sendPacket_RST(pSlicedConnection, pSlicedConnection->getTestSequence());
                    }

                    if(getTCPFlag_PSH(pPacket->getData(), -1))
                    {
                            pSlicedConnection->getMeasurementPkts_Setup()->addPacket(pMeasureAck);
                    }
                    else
                    {
                            pSlicedConnection->getMeasurementPkts_Received()->addPacket(pMeasureAck);
                    }

            }

            pPair = new FMNC_Measurement_Pair();
            pPair->setMeasurement_Sent((FMNC_Measurement_Packet_TCP*)pSlicedConnection->getMeasurementPkts_Sent()->getPacket(0));
            pPair->setMeasurement_Rcvd((FMNC_Measurement_Packet_TCP*)pSlicedConnection->getMeasurementPkts_Received()->getPacket(0));
            pPair->computeRTT();
            /* cout<<" Prelimimary Measurement of RTT "<<pPair->getRTT() + 1000<<endl; */
            /* TimerEvent 	*	pTimer; */

            pTimer = TimerEvent::createTimer();
            pTimer->setTimerID(FMNC_TIMER_FINISH);
            pTimer->setData(pSlicedConnection);
            cout<<" Set Timer to Clean Up Connection->"<<pSlicedConnection->getRequest()<<endl;

            // How long until we start things?
            pTimer->setInterval_ms(pPair->getRTT()+250);

            if(!addTimerEvent(pTimer))
            {
                    cerr << "* Error: Unable to add timer" << endl;
                    pTimer->release();
                    pTimer = NULL;
            }

            // We are eating the packet (none shall pass)
            pPacket->release();
            return PROCESSPKT_RESULT_CONSUMED;

        default:
			return PROCESSPKT_RESULT_CONTINUE;
	}

	return PROCESSPKT_RESULT_CONTINUE;
}

char FMNC_Manager::handleTCP (Packet * pPacket)
{
	FMNC_Connection * pConnection;

//	cout << "* FMCN::handleTCP" << endl;

	// Have we seen this connection (flow) before?
	pConnection = m_Connections.findConnection(pPacket);

	if(pConnection != NULL)
	{
		m_Stats.Add_Stat(FMNC_MANAGER_KNOWN_TCP_CONN_COUNT, 1);
//		cout << "** handleTCP: Existing TCP connection" << endl;
		return handleTCP_Existing(pConnection, pPacket);
	}
	else
	{
		//cout << " * Unknown TCP connection" << endl;

		m_Stats.Add_Stat(FMNC_MANAGER_UNKNOWN_TCP_CONN_COUNT, 1);

		// What are the pertinent pieces of information in the packet?
		//cout << "   Src IP: ";
		//dumpIPv4(pPacket->getData() + L2_OFFSET + OFFSET_IPV4_PROTOCOL_SRCIP);
		//cout << endl;
		//cout << "   Dst IP: ";
		//dumpIPv4(pPacket->getData() + L2_OFFSET + OFFSET_IPV4_PROTOCOL_DSTIP);
		//cout << endl;
	}

	// We have not seen the connection before.  The only kind we might think about is a SYN
	//  connection initiating a new connection

	// Is this a new connection (SYN is set)?
	if(pConnection == NULL && getTCPFlag_SYN(pPacket->getData(), pPacket->getLength()))
	{
		m_Stats.Add_Stat(FMNC_MANAGER_COUNT_SYN, 1);

		// Any additional filtering (SYN/FIN is set)?
		// TODO: Check extra filtering

		// At the SYN stage, we do not know who this is client-side due to various NAT
		// issues.  We just know this is a new connection

		char		byMode;

		byMode = mapConnectionTypeFromPort(pPacket);

		if(shouldAcceptNewConnection(byMode))
		{
			// Set up a new connection which is a TCP slicer on the outbound (for now)
			FMNC_Connection_TCP_Slice	*	pConnection;

			pConnection = new FMNC_Connection_TCP_Slice();

			pConnection->setSessionID(getSessionID());
			incrementSessionID();
			// Ask the connection object to populate the info from the packet
			pConnection->populateInformation(pPacket);

			// Set the TCP outbound sequence number (server-side).  We need to do it at the manager
			//  level depending on how sophisticated we want to be in case we want to separate out
			//  different sequence numbers on the response to prevent spoofing
			pConnection->setSeqNum_Server(getSequenceNumber(pPacket));

			// Add it to the mix
			m_Connections.add(pConnection);

			// Ask the connection object to help us draft a response
			Packet	*	pResponse;


			pResponse = pConnection->respondToSYN (pPacket);

			cout << "  New SYN recognized" << endl;


			// Did we get a good value?  Always check the results to make sure they are a minimum at
			//  least non-NULL
			if(pResponse == NULL)
			{
				cerr << "Error: Packet response after the SYN was NULL" << endl;
				cerr << "  handleTCP - fmnc_manager.cc" << endl;

				// Let the packet go
				return PROCESSPKT_RESULT_CONTINUE;
			}
			else
			{
//				cout << "Attempting to write packet back to the potential client" << endl;

				pResponse->registerWriteCallback(this, NULL);

			//	cout << " Original packet flag" << endl;
			//	//pPacket->showHex(0, pPacket->getLength());
            //    cout<< unsigned( getTCP_Flags(pPacket->getData()));
			//	cout << endl;

			//	cout << " Response packet flag" << endl;
			//	//pResponse->showHex(0, pResponse->getLength());
            //    cout<< unsigned( getTCP_Flags(pResponse->getData()));
			//	cout << endl;
				writePacket(pResponse);

				// Eat the packet noting that it is ours
				pPacket->release();
				return PROCESSPKT_RESULT_CONSUMED;
			}
		}
		else
		{
			// Cut it out
			return PROCESSPKT_RESULT_CONTINUE;
		}
	}
	else
	{
		//cout << "** handleTCP: Unknown TCP connection - SYN flag not set - ignoring" << endl;
	}

	return PROCESSPKT_RESULT_CONTINUE;
}

uint32_t	FMNC_Manager::getSequenceNumber (Packet * pOutbound)
{
	return 1000000;
}

char FMNC_Manager::handleUDP (Packet * pPacket)
{
	return PROCESSPKT_RESULT_CONTINUE;
}

char FMNC_Manager::processPacket_WAN (Packet * pPacket)
{
//	cout << "Got a packet of length " << pPacket->getLength() << endl;

	// Increment our packet count
	m_Stats.Add_Stat(FMNC_MANAGER_TOTAL_COUNT_PKTS, 1);
	m_Stats.Add_Stat(FMNC_MANAGER_TOTAL_VOLUME_PKTS, pPacket->getLength());

	// Is this packet even an IP packet?
	if(!isPacketIP(pPacket->getData(), pPacket->getLength()))
	{
		m_Stats.Add_Stat(FMNC_MANAGER_NONIP_COUNT_PKTS, 1);
		m_Stats.Add_Stat(FMNC_MANAGER_NONIP_VOLUME_PKTS, pPacket->getLength());
		return PROCESSPKT_RESULT_CONTINUE;
	}

	// Is this IPv4, IPv6, or who knows what?
	switch(getIPVersion(pPacket->getData(), pPacket->getLength()))
	{
		case 4:
			break;
		case 6:
			m_Stats.Add_Stat(FMNC_MANAGER_IPV6_COUNT_PKTS, 1);
			m_Stats.Add_Stat(FMNC_MANAGER_IPV6_VOLUME_PKTS, pPacket->getLength());
			return PROCESSPKT_RESULT_CONTINUE;

		default:
			break;
	}

	// Is this packet for us (i.e. inbound to this particular box)?
	if(filterDestinationAddress(pPacket))
	{
		m_Stats.Add_Stat(FMNC_MANAGER_FILTER_IP_COUNT_DESTADDR, 1);
		m_Stats.Add_Stat(FMNC_MANAGER_FILTER_IP_VOLUME_DESTADDR, pPacket->getLength());
		return PROCESSPKT_RESULT_CONTINUE;
	}

	if(filterDestinationPort(pPacket))
	{
		m_Stats.Add_Stat(FMNC_MANAGER_FILTER_PORT_COUNT_DESTADDR, 1);
		m_Stats.Add_Stat(FMNC_MANAGER_FILTER_PORT_VOLUME_DESTADDR, pPacket->getLength());
		return PROCESSPKT_RESULT_CONTINUE;
	}

	// Is this packet TCP or UDP?
	if(isTCP(pPacket->getData(), pPacket->getLength()))
	{
		return handleTCP(pPacket);
	}
	else if(isUDP(pPacket->getData(), pPacket->getLength()))
	{
		return handleUDP(pPacket);
	}
	else
	{
		// IPsec or something but we don't know what it is so ignore it but count it
	}

	return PROCESSPKT_RESULT_CONTINUE;
}

uint16_t	FMNC_Manager::getPort_NI ()
{
	return m_nPort_NI;
}

void 		FMNC_Manager::setPort_NI (uint16_t nPort)
{
	m_nPort_NI = nPort;
}

uint16_t	FMNC_Manager::getPort_Result ()
{
	return m_nPort_Result;
}

void 		FMNC_Manager::setPort_Result (uint16_t nPort)
{
	m_nPort_Result = nPort;
}
uint16_t	FMNC_Manager::getPort_Web ()
{
	return m_nPort_Web;
}

void		FMNC_Manager::setPort_Web (uint16_t nPort)
{
	m_nPort_Web = nPort;
//	try {
//		m_pWebSocket = new TCPServerSocket(m_nPort_Web);    
//  	} catch (SocketException &e) {
//    	cerr << e.what() << endl;
//    	return;
//  	}	
}

uint16_t	FMNC_Manager::getPort_XML ()
{
	return m_nPort_XML;
}

void		FMNC_Manager::setPort_XML (uint16_t nPort)
{
	m_nPort_XML = nPort;
}

bool	FMNC_Manager::filterDestinationAddress (Packet * pPacket)
{
	// If there is not a server address set, filtering on the basis of destination address is not enabled
	if(m_pServerAddress == NULL)
	{
		return false;
	}

	// Check the destination address
	//
	// TODO: This is pretty ugly, there really should be a better way to compare things
	if(*m_pServerAddress == pPacket->getData()+L2_OFFSET+OFFSET_IPV4_PROTOCOL_DSTIP)
	{
		return false;
	}
	else
	{
		return true;
	}

	return false;
}

bool	FMNC_Manager::filterDestinationPort	 (Packet * pPacket)
{
	uint16_t	nPort;

	nPort = 0;

	// Get the port out of the packet

	switch(getIPv4Protocol(pPacket->getData()))
	{
		case IP_PROTOCOL_TCP:
			nPort = getTCP_DstPort(pPacket->getData());
			break;

		case IP_PROTOCOL_UDP:
			break;
	}

	if(nPort == getPort_Web() || nPort == getPort_NI() || nPort == getPort_Result()|| nPort == getPort_XML())
	{
		return false;
	}
	else
	{
		return true;
	}
}

NetAddressIPv4 *	FMNC_Manager::getServerAddress ()
{
	return m_pServerAddress;
}

void FMNC_Manager::setServerAddress (NetAddressIPv4 * pAddress)
{
	m_pServerAddress = pAddress;
}

void FMNC_Manager::setServerAddress(string sAddress)
{
	NetAddressIPv4	*	pAddress;

	// First case is no filter which is the empty string
	if(sAddress == "")
	{
		setServerAddress(NULL);
		return;
	}

	pAddress = NetAddressIPv4::create();

	if(pAddress != NULL)
	{

		if(inet_pton(AF_INET, sAddress.c_str(), pAddress->getAddress()) == 1)
		{
			setServerAddress(pAddress);
		}
		else
		{
			cerr << "* Warning: Unable to enable destination filtering due a bad destination address filter specification" << endl;
			cerr << "   inet_pton returned a bad value and the filter setting is being ignored" << endl;
			cerr << "   Requested setting: " << sAddress << endl;
		}
	}
	else
	{
		cerr << "* Error: Unable to allocate IPv4 address when setting the server address.  You probably have" << endl;
		cerr << "   bigger fish to fry if we are failing at startup when this is typically set" << endl;
	}
}

void FMNC_Manager::setInterval_Cleanup (uint16_t nInterval)
{
	m_nInterval_Cleanup = nInterval;
}

uint16_t FMNC_Manager::getInterval_Cleanup ()
{
	return m_nInterval_Cleanup;
}

void	FMNC_Manager::setAge_PurgeConnection (uint16_t nAge)
{
	m_nOld_Connection = nAge;
}

uint16_t	FMNC_Manager::getAge_PurgeConnection ()
{
	return m_nOld_Connection;
}

void		FMNC_Manager::setDelay_SendFIN (uint32_t lDelay)
{
	m_nDelay_SendFIN = lDelay;
}

uint32_t	FMNC_Manager::getDelay_SendFIN ()
{
	return m_nDelay_SendFIN;
}

void		FMNC_Manager::setTestChoice (int nTest)
{
	m_nTestChoice = nTest;
}

int			FMNC_Manager::getTestChoice ()
{
	return m_nTestChoice;
}

uint16_t	FMNC_Manager::getSliceSize ()
{
	return m_nSliceSize;
}

void		FMNC_Manager::setSliceSize (uint16_t nSlice)
{
	m_nSliceSize = nSlice;
}

void		FMNC_Manager::setSliceSpacing (uint32_t lSpaceSec, uint32_t lSpaceMicroSec)
{
	m_nSliceSpace_Sec = lSpaceSec;
	m_nSliceSpace_MicroSec = lSpaceMicroSec;
}

uint32_t	FMNC_Manager::getSliceSpacing_Sec ()
{
	return m_nSliceSpace_Sec;
}

uint32_t	FMNC_Manager::getSliceSpacing_MicroSec ()
{
	return m_nSliceSpace_MicroSec;
}

void		FMNC_Manager::setDelay_Initial (uint32_t lDelay)
{
	m_nDelay_Initial = lDelay;
}

uint32_t	FMNC_Manager::getDelay_Initial ()
{
	return m_nDelay_Initial;
}

FMNC_Test_Sequence *	FMNC_Manager::createTestSequence_PreDef (FMNC_Connection_TCP_Slice *	pSlicedConnection)
{
	FMNC_Test_Sequence *	pTestSequence;

	pTestSequence = new FMNC_Test_Sequence();
	pTestSequence->setConnection(pSlicedConnection);
	pSlicedConnection->setTestSequence(pTestSequence);

	// Set up the default parameters for slicing / spacing
	pTestSequence->setSliceSize(getSliceSize());
	pTestSequence->setSliceSpacing(getSliceSpacing_Sec(), getSliceSpacing_MicroSec());

	cout << "Request: " << pSlicedConnection->getRequest() << endl;

	// Do we have a web request from the client (Length > = 0)?
	if(pSlicedConnection->getRequest().length() > 0)
	{
		// TODO: Eventually, this needs to get fixed.  We are doing it ugly style for now
		//       for the purposes of running demos but we need to eventually fix it

		if(pSlicedConnection->getRequest().compare(0,7,"/tests/") == 0)
		{
			// TODO: We are kind of hackering this together at the moment.  Fix this
			//  with proper modularization later
            m_nSliceSpace_MicroSec = FMNC_DEFAULT_SLICE_SPACE_MICROSEC;

            if(pSlicedConnection->getRequest().compare(7,4,"base") == 0)
			{
                cout << " Create Test WebDemo (base) !"<<endl;
				pTestSequence->setTestConfig("<Predefined WebDirect=\"base\" Fixed=\"50\" Multi=\"10\" Loops=\"2\" />");
				pTestSequence->createTest_WebDemo (50,2,10,pSlicedConnection->getSessionID());
			}
			else if(pSlicedConnection->getRequest().compare(7,3,"big") == 0)
			{
				pTestSequence->setTestConfig("<Predefined WebDirect=\"full\" Fixed=\"100\" Multi=\"14\" Loops=\"5\" />");
				pTestSequence->createTest_WebDemo (100,5,14,pSlicedConnection->getSessionID());
			}
			else if(pSlicedConnection->getRequest().compare(7,8,"inst.png") == 0)
			{
				pTestSequence->setTestConfig("<Predefined WebDirect=\"image\" Fixed=\"50\" Multi=\"10\" Loops=\"2\" />");
                pTestSequence->wrapFile_PNG (50,2,10);
            }
            else if(pSlicedConnection->getRequest().compare(7,5,"train") == 0 &&
                            pSlicedConnection->getRequest().compare(12,9,"?Cellular") == 0 &&
                            pSlicedConnection->getRequest().compare(21,6,"?Rmin=") == 0 &&
                            pSlicedConnection->getRequest().compare(30,6,"?Rmax=") == 0 &&
                            pSlicedConnection->getRequest().compare(39,7,"?Alpha=") == 0 
                   )
            {
                    uint16_t rmin=atoi(pSlicedConnection->getRequest().substr(27,3).c_str());
                    uint16_t rmax=atoi(pSlicedConnection->getRequest().substr(36,3).c_str());
                    double alpha=atof(pSlicedConnection->getRequest().substr(46,5).c_str());
                    pSlicedConnection->setSearch_Param(rmin,rmax);
                    pSlicedConnection->setTrainDuration(alpha);

                    pTestSequence->createTest_TrainFACellularWebDemo (rmin,rmax,alpha,pSlicedConnection->getSessionID());
            }
            else if(pSlicedConnection->getRequest().compare(7,5,"train") == 0 &&
                            pSlicedConnection->getRequest().compare(12,8,"?Length=") == 0 &&
                            pSlicedConnection->getRequest().compare(23,6,"?Step=") == 0 &&
                            pSlicedConnection->getRequest().compare(31,6,"?Rmin=") == 0 &&
                            pSlicedConnection->getRequest().compare(40,6,"?Rmax=") == 0 &&
                            pSlicedConnection->getRequest().compare(49,5,"?Gap=") == 0)
            {
                    uint16_t length=atoi(pSlicedConnection->getRequest().substr(20,3).c_str());
                    uint8_t step=atoi(pSlicedConnection->getRequest().substr(29,2).c_str());
                    uint16_t rmin=atoi(pSlicedConnection->getRequest().substr(37,3).c_str());
                    uint16_t rmax=atoi(pSlicedConnection->getRequest().substr(46,3).c_str());
                    uint16_t gap=atoi(pSlicedConnection->getRequest().substr(54,4).c_str());

                    pTestSequence->createTest_TrainFAWebDemo (length,step,rmin,rmax,gap,pSlicedConnection->getSessionID());
            }
            else if(pSlicedConnection->getRequest().compare(7,6,"custom") == 0 &&
                            pSlicedConnection->getRequest().compare(13,7,"?Fixed=") == 0 && 
                            pSlicedConnection->getRequest().compare(23,7,"?Multi=") == 0 &&
                            pSlicedConnection->getRequest().compare(33,7,"?Loops=") == 0 &&
                            pSlicedConnection->getRequest().compare(43,6,"?Swap=") == 0 &&
                            pSlicedConnection->getRequest().compare(52,9,"?Spacing=") == 0 &&
                            pSlicedConnection->getRequest().compare(65,11,"?SliceSize=") == 0)
            {
                    int fixed=atoi(pSlicedConnection->getRequest().substr(20,3).c_str());
                    int multi=atoi(pSlicedConnection->getRequest().substr(30,3).c_str());
                    int loops=atoi(pSlicedConnection->getRequest().substr(40,3).c_str());
                    int swap=atoi(pSlicedConnection->getRequest().substr(49,3).c_str());
                    int spacing=atoi(pSlicedConnection->getRequest().substr(61,4).c_str());
                    int slicesize=atoi(pSlicedConnection->getRequest().substr(76,4).c_str());

                    pTestSequence->setTestConfig("<Predefined WebDirect=\"custom\" Fixed=\""+pSlicedConnection->getRequest().substr(20,3)
                                    +"\" Multi=\""+pSlicedConnection->getRequest().substr(30,3)
                                    +"\" Loops=\""+pSlicedConnection->getRequest().substr(40,3)
                                    +"\" />");
                    cout << " DBG(lsong): Get the custom request of parameters :fixed="<<fixed<<
                            "  multi="<<multi<<
                            "  spacing="<<spacing<<
                            "  loops="<<loops<<endl;
                    setSliceSize(slicesize);
                    pTestSequence->setSliceSize(getSliceSize());
                    setSliceSpacing(FMNC_DEFAULT_SLICE_SPACE_SEC,spacing);
                    pTestSequence->setSliceSpacing(getSliceSpacing_Sec(), getSliceSpacing_MicroSec());
                    pTestSequence->createTest_WebDemo (fixed,loops,multi,pSlicedConnection->getSessionID());


            }
            else
            {
                    // Fall back to the default if we do not have a match
                    pTestSequence->populatePredefined(getTestChoice());
            }
		}
		else if(pSlicedConnection->getRequest().compare(0,4,"/tr/") == 0)
		{
			// Is this request for results?
			string		sSessionID;

			sSessionID = pSlicedConnection->getRequest().substr(4);

			uint32_t	lSessionID;

			if(sSessionID.length() > 0)
			{
				lSessionID = atoi(sSessionID.c_str());
			}
			else
			{
				lSessionID = 0;
			}

			FMNC_Connection * pConnection;
			ParamDictionary * pDict;

			pDict = NULL;

			pConnection = m_Connections.findConnection(lSessionID);

            cout << " The connection is analyzed : "<<pConnection->IsAnalysis()<<endl;

            if(true){


                    if(pConnection == NULL)
                    {
                        // Nothing to put into it - leave everything blank
                    }
                    else
                    {
                        cout << " Processing Result: Found an existing connection!!" << endl;

                       // while(!pConnection->IsAnalysis())
                       // {
                       //         ;
                       // }
                        if(pConnection->getDictionary() == NULL)
                        {
                            pConnection->allocateDictionary();
                        }

                        if(pConnection->getDictionary() != NULL)
                        {
                            pConnection->populateDictionary_Base();
                        }

                        pDict = pConnection->getDictionary();

                        pDict->fuseEntries(pConnection->getTestSequence()->getDictionary(), false, false);
                        cout << "Processing Result: Dictionary of Test Sequence" << endl;
                        pConnection->getTestSequence()->getDictionary()->dumpConsole();

                    //	pDict->fuseEntries(pConnection->getTestAnalysis()->getDictionary(), false, false);
                    //	cout << "Processing Result: Dictionary of Test Analysis" << endl;
                    //	pConnection->getTestAnalysis()->getDictionary()->dumpConsole();

                        // Get additional information from the connection itself

                        FMNC_Connection_TCP_Slice	* pSliceConn;

                        pSliceConn = (FMNC_Connection_TCP_Slice *) pConnection;

                        // Let's get a bit more information from the connection and the underlying
                        //  test sequence within the connection on what was actually sent to the client

                        // TODO: Add that information with regards to an analysis
                    }

                    pDict->dumpConsole();

                    pTestSequence->constructFileWithParams("fmnc/support/web/result-iframe.html", pDict, true);
                    //delete pConnection;
            }
            else
            {
                    cout << " The analysis has not finished. Waiting to response results. "<<endl;
                    TimerEvent 	*	pFirstTimer;

                    pFirstTimer = TimerEvent::createTimer();
                    pFirstTimer->setData(pSlicedConnection);
                    pFirstTimer->setTimerID(FMNC_TIMER_START_SLICING);
                    pFirstTimer->setInterval_ms(1000);
                    pFirstTimer->armTimer();

                    if(!addTimerEvent(pFirstTimer))
                    {
                            cerr << "* Error: Unable to add timer" << endl;
                            pFirstTimer->release();
                    }

            }
        }
		else if(pSlicedConnection->getRequest().compare("tests/default") == 0)
		{
			pTestSequence->populatePredefined(getTestChoice());
		}
		else if(pSlicedConnection->getRequest().compare(0,8,"results/") == 0)
		{
			// Is this request for results?
			string		sSessionID;

			sSessionID = pSlicedConnection->getRequest().substr(8);

			uint32_t	lSessionID;

			if(sSessionID.length() > 0)
			{
				lSessionID = atoi(sSessionID.c_str());
			}
			else
			{
				lSessionID = 0;
			}

			FMNC_Connection * pConnection;

			pConnection = m_Connections.findConnection(lSessionID);

			if(pConnection == NULL)
			{

			}
			else
			{

			}



			pTestSequence->constructFileWithParams("fmnc/support/web/result-iframe.html", NULL, true);
		}
		else
		{
			// Fall back to the default if we do not have a match
			pTestSequence->populatePredefined(getTestChoice());
		}
	}
	else
	{
		// Fill it up with the pre-defined info
		pTestSequence->populatePredefined(getTestChoice());
	}

	// Adjust the sequence numbers
	pTestSequence->adjustSeqNumber(pSlicedConnection->getSeqNum_Server());


	// Adjust the dispatch time to be based off of the current time
	timeval 	currentTime;
	gettimeofday(&currentTime, NULL);
	pTestSequence->adjustDispatchTime(&currentTime);

	return pTestSequence;
}

string	FMNC_Manager::getWeb_Request (Packet * pPacket)
{
	// Figure out the size of the TCP payload
	uint16_t	nPayloadSize;

	nPayloadSize = getTCP_PayloadSize(pPacket->getData());

	if(nPayloadSize == 0)
	{
		return "";
	}

	// Force the end of the packet data to be a null terminated string
	pPacket->zeroPad(1);

	string	sData;

	// Construct the string from the start of the TCP payload to the end of it
	sData = pPacket->getData()+getOffset_TCP_Payload (pPacket->getData(), pPacket->getLength());

//	cout << "  Offset was " << getOffset_TCP_Payload(pPacket->getData(), pPacket->getLength()) << endl;
//	cout << "  Data Payload: " << sData << endl;

	// Find the first carriage return
	size_t	nPos;

//	cout << "Carriage return check" << endl;
	nPos = sData.find("\n");

	if(nPos == std::string::npos)
	{
		nPos = sData.find("\r");

		if(nPos == std::string::npos)
		{
			return "";
		}
	}

	string	sRequest;

	sRequest = sData.substr(0,nPos);

	// Make sure there is a GET request
	cout << " GET check" << endl;

	nPos = sRequest.find("GET");

	if(nPos == std::string::npos)
	{
		return "";
	}

	string	sObject;

	sObject = sRequest.substr(nPos+3);

	//cout << "   Request = " << sRequest << endl;
	//cout << "   Object  = " << sObject << endl;

	// sObject now contains at its left the object
	//cout << " Space check" << endl;

	nPos = sObject.find_first_not_of(" ");

	if(nPos == std::string::npos)
	{
		// That is OK as the object is the object
	}
	else
	{
		//cout << "   Space located at " << nPos << endl;
		sObject = sObject.substr(nPos);
	}

	nPos = sObject.find(" ");

	if(nPos == std::string::npos)
	{
		// OK, means we were probably bumping up against the line feed
	}
	else
	{
		sObject = sObject.substr(0,nPos);
	}

	return sObject;
}




bool FMNC_Manager::saveConnectionInfo	 (FMNC_Connection * pConnection)
{
	return true;
}
