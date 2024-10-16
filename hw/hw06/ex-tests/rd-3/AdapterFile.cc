/* AdapterFile.h
 **********************************************************
 * This code is part of the Scalability in a Box (ScaleBox)
 * system developed at the University of Notre Dame. 
 * 
 * For additional information, please visit the NetScale
 * laboratory webpage at:
 *
 * 	 http://netscale.cse.nd.edu/
 *
 * PI:       Prof. Aaron Striegel    striegel@nd.edu
 */

#include <iostream>
using namespace std;

#include "AdapterFile.h"
#include "Packet.h"
#include "../util/ip-utils.h"
#include "../util/mgmt.h"
#include "../core/LogEvent.h"

#include "../xml/NodeDOM.h"
#include "../xml/NodeElem.h"

#include "../core/Timestamp.h"

/////////////////////////////////////////////////////////////////

StatsAdapterFile::StatsAdapterFile () {
	Allocate(STAT_ADPFILE_END);		
}

/////////////////////////////////////////////////////////////////		
		
void StatsAdapterFile::Get_Title (int nStat, char * szTitle) {
	switch(nStat) {
		case STAT_ADPFILE_PKTCOUNT:
			sprintf(szTitle, "PktCount");
			break;
			
		case STAT_ADPFILE_PKTDATA:
			sprintf(szTitle, "PktData");
			break;
	
		case STAT_ADPFILE_FILESPARSED:
			sprintf(szTitle, "FilesParsed");
			break;

		case STAT_ADPFILE_LOOPSFINISHED:
			sprintf(szTitle,"LoopsFinished");
			break;
			
		case STAT_ADPFILE_LOOPSLEFT:
			sprintf(szTitle, "LoopsLeft");
			break;
	
		case STAT_ADPFILE_BADCAPLEN:
			sprintf(szTitle, "BadCaptureLength");
			break;
			
		case STAT_ADPFILE_TOOLARGE:
			sprintf(szTitle, "PktsTooLarge");
			break;

		case STAT_ADPFILE_EXCEEDBACKLOG:
			sprintf(szTitle, "PktBacklogExceeded");			
			break;
			
		case STAT_ADPFILE_EXCEEDBACKLOG_DURATION:
			sprintf(szTitle, "PktBacklogExceededYields");
			break;
			
		default:
			sprintf(szTitle, "F%03d", nStat);
			break;
	}	
}

/////////////////////////////////////////////////////////////////

AdapterFile::AdapterFile () {
	
	// File settings
	m_sFilePath = "";			// Relative to current directory by default
	m_bFlipEndian = false;
	m_nCurrentFileIndex = 0;
	
	m_bPlaybackDone = false;

	// Loop settings
	m_nLoopCount = DEFAULT_ADAPTERFILE_LOOPCOUNT;
	m_nRemainingLoops = m_nLoopCount;
	m_bShouldLoop = DEFAULT_ADAPTERFILE_SHOULDLOOP;	
	
	// Playback rate acceleration
	m_bAccelPlayback = false;
	m_fPlaybackRate =  DEFAULT_ADAPTERFILE_PLAYBACKRATE;	
	
	// Offset all times relative to when the file was opened
	m_bAdjustTime = true;
	
	// Do not pause between reads
	m_bNoPause = true;
	
	// Debug info to display?
	m_nDebugLevel = 0;
	
	// Default maximum size for backlog	
	m_nMaxPacketBacklog = DEFAULT_ADAPTERFILE_MAXBACKLOG; 

	m_bAlignTime_Files = false;
	m_Time_Align_Accumulated.clearTime();
	m_Time_CurrentFileAdjust.clearTime();

	// Do we allow for partial packets?
	m_bAllowPartial = true;
}

/////////////////////////////////////////////////////////////////

AdapterFile::~AdapterFile () {
	
}

/////////////////////////////////////////////////////////////////

void	AdapterFile::dumpBasicInfo  () {
	cout << "AdapterFile" << endl;
}

/////////////////////////////////////////////////////////////////
		
bool	AdapterFile::extractExtendedDOM (NodeDOM * pNode) {
	int		j;	
	
//	cout << "AdapterFile::extractExtendedDOM" << endl;

	for(j=0; j<pNode->getNumChildren(); j++) {
		cout << " Tag was " << pNode->getChild(j)->getTag() << endl;

		if(pNode->getChild(j)->getTag() == "file") {
			addFile(pNode->getChild(j)->getData());
			cout << " Adding a file to the mix of " << pNode->getChild(j)->getData() << endl;
		}	
		else if(pNode->getChild(j)->getTag() == "debug") {
			setDebugSetting(pNode->getChild(j)->getData());
		}
		else if(pNode->getChild(j)->getTag() == "noPause") {
			setFlag_NoPause(translateStringToBoolean(pNode->getChild(j)->getData()));
		}
		else if(pNode->getChild(j)->getTag() == "MaxBacklog") {
			setValue_MaxBacklog(translateStringToInt(pNode->getChild(j)->getData()));
		}
		else if(pNode->getChild(j)->getTag() == "alignTimes")
		{
			setFlag_AlignTimes(translateStringToBoolean(pNode->getChild(j)->getData()));
		}
	}
	
	// TODO Mod not to give an error if we are doing only output
	if(getFileCount() == 0) {
		logFailureEvent("No files specified for the adapter in the XML config, kind of defeats the purpose of using it, eh?", "AdapterFile", "extractExtendedDOM");		
		return false;
	}
	
//	cout << " Finished parsing the DOM successfully!" << endl;

	return true;		
}

/////////////////////////////////////////////////////////////////

bool AdapterFile::setDebugSetting (string sInfo) {
	if(sInfo == "yes") {
		m_nDebugLevel = 1;
	}
	
	return true;	
}

void AdapterFile::setFlag_AlignTimes (bool bShouldAlign)
{
	m_bAlignTime_Files = bShouldAlign;
}

bool AdapterFile::getFlag_AlignTimes ()
{
	return m_bAlignTime_Files;
}

/////////////////////////////////////////////////////////////////

void AdapterFile::readPacket () {	
	if(m_bPlaybackDone) {
		yieldCPU();
		return;
	}
	
	// Read the packet from the file
	// Figure out when the next one is coming (if at all)
		
	Packet *		pPacket;
	timeval			curArrivalTime;
	timeval			nextArrivalTime;
	
	Timestamp 		lastReadTime;

	// Read the packet from the file
	//	time_t struct		Seconds, microseconds (each 32 bits)
	//  Capture Length		32 bits
	//	Actual Length		32 bits
		
	if(m_CurrentFile.good()) {
		// Fetch me a new packet
		
		timeval		*		pArrival;
		unsigned int		nCapLen;
		unsigned int		nActLen;
		
		// Get a packet object from the pool or create one
		pPacket = Packet::createPacket();
		
		pArrival = pPacket->getArrivalTime();
			
		m_CurrentFile.read((char *) &(pArrival->tv_sec),4);
		m_CurrentFile.read((char *)  &(pArrival->tv_usec),4);
		m_CurrentFile.read((char *)  &nCapLen,4);
		m_CurrentFile.read((char *)  &nActLen,4);
		
		if(m_bFlipEndian) {
			pArrival->tv_sec = endianfixl(pArrival->tv_sec);
			pArrival->tv_usec = endianfixl(pArrival->tv_usec);
			nCapLen = endianfixl(nCapLen);
			nActLen = endianfixl(nActLen);
		}

		if(m_nDebugLevel) {
			cout << "  File Timestamp: " << pArrival->tv_sec << "." << pArrival->tv_usec << endl;
		}

		Timestamp 	adjustTS;
		
		// Subtract the current time (if appropriate)
		adjustTS.setTime(*(pPacket->getArrivalTime()));
		adjustTS = adjustTS - m_Time_CurrentFileAdjust;

		// lastReadTime captures how deep we are relatively speaking into a file
		lastReadTime = adjustTS;

/*		if(m_nDebugLevel)
		{
			cout << "  File Timestamp: " << pArrival->tv_sec << "." << pArrival->tv_usec << endl;
			cout << "  Adj  Timestamp: " << adjustTS.getSeconds() << "." << adjustTS.getMicroseconds() << endl;
		}
*/

		// Update our current accumulated time
		adjustTS = adjustTS + m_Time_Align_Accumulated;

		/*
		if(m_nDebugLevel)
		{
			cout << "  CFA  Timestamp: " << m_Time_CurrentFileAdjust.getSeconds() << "." << m_Time_CurrentFileAdjust.getMicroseconds() << endl;
		}
		*/


		// Update the packet
		pPacket->setArrivalTime(adjustTS.getCoreTime());

		/*
		if(m_nDebugLevel)
		{
			cout << "  PFAD Timestamp: " << pArrival->tv_sec << "." << pArrival->tv_usec << endl;
			cout << "  TA   Timestamp: " << m_Time_Align_Accumulated.getSeconds() << "." << m_Time_Align_Accumulated.getMicroseconds() << endl;
		}
		*/

		// Write this back to the packet
		curArrivalTime = *pArrival;		

		// Is the capture length equal to the actual length?  This should be the case the vast majority of time.  
		//  If it is not, we have a dilemma regarding the missing info, i.e. the dump file does not have the
		//  entirety of the packet. For now, we will just skip it in the interest of not fouling things up but
		//  it may be desirable in certain situations (packet scheduling) to simpy fan out dummy data. 
		
		if(nCapLen != nActLen && m_bAllowPartial == false) {
			logInformEvent("Packet capture size recorded in dump file not equal to actual size (per dump file)", "AdapterFile","readPacket");
			m_Stats.Add_Stat(STAT_ADPFILE_BADCAPLEN, 1);
			pPacket->release();			
		} else if (nCapLen < pPacket->getMaxSize()){
			m_Stats.Add_Stat(STAT_ADPFILE_PKTCOUNT, 1);
			m_Stats.Add_Stat(STAT_ADPFILE_PKTDATA, nCapLen);
						
			pPacket->setLength(nCapLen);			
			m_CurrentFile.read(pPacket->getData(),nCapLen);

			addInputQueue(pPacket);			
		} else {
			// Too big, we can't handle the truth :)
			m_Stats.Add_Stat(STAT_ADPFILE_TOOLARGE,1);
			pPacket->release();
		}
	} else {
		// Problems with the file, hmmm, yield and bail out
		//cout << "  Bad read" << endl;
		yieldCPU();
		return;
	}
	
	// Wait for the next packet (if applicable)
	
	//   For now, we'll do this a bit less efficient (read twice on the initial timer info) but this can be optimized
	//    for those that really need performance

	m_CurrentFile.read((char *)  &(nextArrivalTime.tv_sec),4);
	m_CurrentFile.read((char *)  &(nextArrivalTime.tv_usec),4);

	if(m_CurrentFile.good()) {	
		if(m_bFlipEndian) {
			nextArrivalTime.tv_sec = endianfixl(nextArrivalTime.tv_sec);
			nextArrivalTime.tv_usec = endianfixl(nextArrivalTime.tv_usec);
		}
		
		// Pretend like we never read in the time
		m_CurrentFile.seekg(-8,ios_base::cur);		
		
		// Compute the difference in time
		unsigned int		nTimeDiff;
		
		//cout << " NAT: ";
		//displayTime(&nextArrivalTime);
		//cout << endl;
		
		//cout << " CAT: ";		
		//displayTime(&curArrivalTime);
		//cout << endl;
		
		nTimeDiff = calcTimeDiff(&nextArrivalTime,&curArrivalTime);	
		
		// Apply mod to playback gap if applicable
		//   Rounding to the nearest microsecond which is kind of a stretch anyway with faster playback rates
		if(m_bAccelPlayback) {
			//cout << " Applying playback speed" << endl;
			nTimeDiff = (unsigned int) ((float) nTimeDiff * m_fPlaybackRate);
		} 
		
		// TODO Check for microsleep overflow
		//cout << "  Sleeping for " << nTimeDiff << " microseconds" << endl;
		
		if(getFlag_NoPause()) {
			yieldCPU();

			// Make sure we do not get too far ahead.  There are internal limits of how
			// big the input queue should get when we are roaring through things

			// The first test is simply to count it, the second one is to gauge how bad
			//  the backlog is in terms of the number of CPU yields that occur
			
			if(getInputQueueSize() >= getValue_MaxBacklog()) {
				m_Stats.Add_Stat(STAT_ADPFILE_EXCEEDBACKLOG, 1);
			}
			
			while(getInputQueueSize() >= m_nMaxPacketBacklog) {
				m_Stats.Add_Stat(STAT_ADPFILE_EXCEEDBACKLOG_DURATION, 1);
				yieldCPU();
			}
		} else {		
			// Sleep for that time
			//   We are making the assumption that the sleep is within a reasonable amount of time.  We should sometime
			//   add in a check for this if at all possible
			microsleep(nTimeDiff);	
		}
	} 
	
	if (m_CurrentFile.eof()) {
		// Close this file
		m_CurrentFile.close();
		
		m_Time_Align_Accumulated = m_Time_Align_Accumulated + lastReadTime;

		cout << "AdapterFile: End of file reached" << endl;
		cout << "    Parsed Time:     " << lastReadTime.getSeconds() << "." << lastReadTime.getMicroseconds() << endl;
		cout << "    Total Read Time: " << m_Time_Align_Accumulated.getSeconds() << "." << m_Time_Align_Accumulated.getMicroseconds() << endl;


		m_Stats.Add_Stat(STAT_ADPFILE_FILESPARSED,1);
		
		// Open the next one if we still have one
		m_nCurrentFileIndex++;
		
		cout << " We have " << getFileCount() << " files in the list" << endl;

		if(m_nCurrentFileIndex >= getFileCount()) {
			m_Stats.Add_Stat(STAT_ADPFILE_LOOPSFINISHED,1);
			
			if(getFlag_Loop()) {
				cout << "Yes, we should loop" << endl;
				m_nCurrentFileIndex = 0;

				if(getRemainingLoopCount() == -1) {
					// Infinite loop
					openFile(m_nCurrentFileIndex);	
					m_Stats.Add_Stat(STAT_ADPFILE_LOOPSLEFT,-1);
				} else if (getRemainingLoopCount() > 0){
					m_nRemainingLoops--;
					openFile(m_nCurrentFileIndex);				
					m_Stats.Set_Stat(STAT_ADPFILE_LOOPSLEFT,m_nRemainingLoops);					
				}								
			}
			else
			{
				cout << "Finished playback" << endl;
				m_bPlaybackDone = true;
			}
		}
		else {
			cout << "Opening file " << m_nCurrentFileIndex << endl;
			openFile(m_nCurrentFileIndex);
		}
	}
}

/////////////////////////////////////////////////////////////////

void AdapterFile::startDevice () {
	// Open the first file
	if(!openFile(0)) {
		logFailureEvent("Failed to open initial file for adapter file", "AdapterFile", "startDevice");
	}
	
	// Start the read, process, and write threads
	startThreads();
}


/////////////////////////////////////////////////////////////////

void AdapterFile::writePacket (Packet * pPacket) {
	// Nothing to do here for now, we are not supporting writing to disk
}

/////////////////////////////////////////////////////////////////

bool AdapterFile::setValue_PlaybackRate (float fVal) {
	if(fVal <= 0) {
		logFaultMediumEvent("Warning: Playback set to illegal rate, must be >= 0", "AdapterFile", "setValue_PlaybackRate");		
		return false;
	}	
	
	m_fPlaybackRate = fVal;
	
	if(m_fPlaybackRate != 1.0) {
		m_bAccelPlayback = true;
	}
	
	return true;
}

/////////////////////////////////////////////////////////////////

float AdapterFile::getValue_PlaybackRate () {
	return m_fPlaybackRate;
}

/////////////////////////////////////////////////////////////////

void AdapterFile::setFlag_Loop (bool bLoop) {
	m_bShouldLoop = bLoop;
}

/////////////////////////////////////////////////////////////////

bool AdapterFile::getFlag_Loop () {
	return m_bShouldLoop;
}

/////////////////////////////////////////////////////////////////

void AdapterFile::setValue_LoopCount (unsigned int nLoopCount) {
	m_nLoopCount = nLoopCount;
	
	if(m_nLoopCount != 0) {
		m_nRemainingLoops = m_nLoopCount;
	}
}

/////////////////////////////////////////////////////////////////

unsigned int	AdapterFile::getValue_LoopCount () {
	return m_nLoopCount;
}

/////////////////////////////////////////////////////////////////

int	AdapterFile::getRemainingLoopCount () {
	if(getValue_LoopCount() == 0) {
		return -1; 
	} else {
		return m_nRemainingLoops;
	}
}

/////////////////////////////////////////////////////////////////
 
bool AdapterFile::addFile (string sFile) {
	// TODO Thread safety
	// TODO Validation of the existence of the file and that we can read it
	m_FileNames.push_back(sFile);
	return true;
}

/////////////////////////////////////////////////////////////////

void AdapterFile::setFilePath (string sPath) {
	// TODO - Validate?
	m_sFilePath = sPath;
}

/////////////////////////////////////////////////////////////////

string AdapterFile::getFilePath () {
	return m_sFilePath;
}

/////////////////////////////////////////////////////////////////

int	AdapterFile::addFilesWithPattern (string sPattern) {
	logFaultHighEvent("Warning: Adding files with pattern not yet supported", "AdapterFile", "addFilesWithPattern");	
	return -1;
}

/////////////////////////////////////////////////////////////////

int AdapterFile::getFileCount () {
	return m_FileNames.size();
}

/////////////////////////////////////////////////////////////////

string	AdapterFile::getFile (int nFile) {
	if(nFile < 0 || nFile >= m_FileNames.size()) {
		logFaultMediumEvent("Warning: Attempted to retrieve file for parsing beyond index", "AdapterFile", "getFile");
		return "";
	}
	
	return m_FileNames[nFile];
}

/////////////////////////////////////////////////////////////////

bool AdapterFile::openFile (int nFile) {
	// Cleanup from previous operations
	
	
	string		sFile;
	
	// Open the new one
	sFile = getFilePath() + getFile(nFile);
	
	cout << " Opened " << sFile << " for processing" << endl;

	m_CurrentFile.open(sFile.c_str());
	
	if(!m_CurrentFile.good()) {
		logFaultHighEvent("Warning: Unable to open file at location " + sFile, "AdapterFile", "openFile");	
		return false;
	} 

	// tcpdump header processing
	//
	//  Reference of file info available at:
	//   http://lists.linux-wlan.com/pipermail/linux-wlan-devel/2003-September/002701.html
	//
	//  Also see:
	//	 http://wiki.wireshark.org/Development/LibpcapFileFormat
	
	// Also will have one posted to the wiki
	
	int					nMagicNum;
	unsigned short		nMajor;
	unsigned short		nMinor;
	unsigned int		nSnapshotLen;
	unsigned int		nMediumType;
	
	// 32 bit magic number
	// 16 bit Major
	// 16 bit Minor
	// Timezone offset (ignore) - 32 bit
	// Timezone accuracy (ignore) - 32 bit
	// Snapshot length - 32 bit
	// Link layer type - 32 bit
	
	m_CurrentFile.read((char *) &nMagicNum,4);
	m_CurrentFile.read((char *) &nMajor,sizeof(unsigned short));
	m_CurrentFile.read((char *) &nMinor,sizeof(unsigned short));
	
	if(nMagicNum == 0xa1b2c3d4) {
		m_bFlipEndian = false;
	} else if (nMagicNum == 0xd4c3b2a1) {
		m_bFlipEndian = true;
	} else {
		cout << "Warning: Non-standard magic number at beginning of TCP dump file" << endl;

		// Major version should be roughly 4, if it is not, probably endian-ness is different
		//  than this particular architecture
		if (nMajor > 255) { 
			cout << "  Guessing endian-ness of file is requiring a flip" << endl;
			m_bFlipEndian = true; 
		} else {
			cout << "  Guessing endian-ness of file is OK" << endl;			
			m_bFlipEndian = false;
		}	
	}
					
	// Ignore time zone and TZ accuracy
	m_CurrentFile.read((char *) &nMagicNum,4);
	m_CurrentFile.read((char *) &nMagicNum,4);
	
	m_CurrentFile.read((char *) &nSnapshotLen,4);
	m_CurrentFile.read((char *) &nMediumType,4);
	
	if(m_bFlipEndian) {
		nSnapshotLen = endianfixl(nSnapshotLen);
		nMediumType = endianfixl(nMediumType);
	}

	if(m_nDebugLevel) {
		cout << "Tcpdump file debug information via AdapterFile" << endl;
		cout << "   Maj / Min Version:   " << nMajor << "." << nMinor << endl;
		cout << "   Endian Flip:         ";
		if(m_bFlipEndian) {
			cout << "Yes" << endl;
		} else {
			cout << "No" << endl;
		}
		
		cout << "   Snapshot Len:        " << nSnapshotLen << endl;
		cout << "   Medium Type:         " << nMediumType << endl;
	}
	
	
	timeval			curArrivalTime;	
	
	// Read in the arrival time of the first packet
	m_CurrentFile.read((char *) &(curArrivalTime.tv_sec),4);
	m_CurrentFile.read((char *)  &(curArrivalTime.tv_usec),4);
	
	// Endian for the timestamp?
	if(m_bFlipEndian) {
		curArrivalTime.tv_sec = endianfixl(curArrivalTime.tv_sec);
		curArrivalTime.tv_usec = endianfixl(curArrivalTime.tv_usec);
	}
	
	Timestamp		currentTime;
	Timestamp		fileTime;
	
	fileTime.setTime(curArrivalTime);
	
	// Are we using aligned times?
	if(!m_bAlignTime_Files)
	{
		// Retrieve / set the system time
		currentTime.setToCurrentTime();

		m_AdjustTime = currentTime - fileTime;
	
		if(m_nDebugLevel) {
			cout << "   File Arrival:        " << fileTime.getSeconds() << "." << fileTime.getMicroseconds() << " s" << endl;
			cout << "   Current Time:        " << currentTime.getSeconds() << "." << currentTime.getMicroseconds() << " s" <<  endl;
			cout << "   Offset      :        " << m_AdjustTime.getSeconds() << "." << m_AdjustTime.getMicroseconds() << " s" << endl;
		}
	}
	else
	{
		m_Time_CurrentFileAdjust = fileTime;
	}
			
	// Pretend like we never read in the time
	m_CurrentFile.seekg(-8,ios_base::cur);		
					
	// Fill is not all set to read
	if(!m_CurrentFile.good()) {
		logFaultHighEvent("Warning: File has insufficient data (no more after header info) at " + sFile, "AdapterFile", "openFile");	
		return false;		
	}

	return true;
}

/////////////////////////////////////////////////////////////////

CommandResult AdapterFile::processExtCommand (const vector<string> & theCommands, int nOffset)
{
	int		j;

//	cout << "Command (fileAdapter -> " << getName() << ")" << endl;

	if(theCommands[nOffset] == "help") {
		cout << "  ScaleBox AdapterFile for parsing pcap files" << endl;
		cout << endl;

		cout << "  listFiles              List the files in the processing list" << endl;
		cout << "  stats                  Dump statistics for the adapter" << endl;
		cout << "  addFile                Add a file to the list for processing" << endl;
		cout << "  clearFiles             Clear all files from the processing list" << endl;
		cout << "  start                  Start the processing list (again)" << endl;

		return PROCESS_CMD_SUCCESS;
	}
	else if (theCommands[nOffset] == "listFiles")
	{
		cout << "[" << getName() << "] Files to Process: " << m_FileNames.size() << endl;

		for(j=0; j<m_FileNames.size(); j++)
		{
			cout << "  File " << j << ": " << m_FileNames[j] << endl;
		}

		return PROCESS_CMD_SUCCESS;
	}
	else if (theCommands[nOffset] == "stats")
	{
		cout << "[" << getName() << "]: Statistics for the adapter" << endl;
		m_Stats.Dump();
		return PROCESS_CMD_SUCCESS;
	}
	else if (theCommands[nOffset] == "addFile")
	{
		if(theCommands.size() <= nOffset+1)
		{
			cout << "* Error: Cannot add a file when a file is not specified" << endl;
			return PROCESS_CMD_SYNTAX;
		}
		else
		{
			addFile(theCommands[nOffset+1]);
			return PROCESS_CMD_SUCCESS;
		}
	}
	else if (theCommands[nOffset] == "clearFiles")
	{
		cout << "[" << getName() << "]: Clearing files" << endl;
		cout << "    " << m_FileNames.size() << " cleared" << endl;
		m_FileNames.clear();
		return PROCESS_CMD_SUCCESS;
	}
	else if (theCommands[nOffset] == "start")
	{
		if(!m_bPlaybackDone)
		{
			cout << " Error: Cannot start playback as current playback is not complete" << endl;
		}
		else
		{
			m_nCurrentFileIndex = 0;
			openFile(0);
			m_bPlaybackDone = false;
		}

		return PROCESS_CMD_SUCCESS;
	}
	else
	{
		cout << "[" << getName() << "] --> Error: Unknown command " << theCommands[nOffset] << endl;
		cout << "   Type cmd adp " << getName() << " help for a list of commands" << endl;
		return PROCESS_CMD_SYNTAX;
	}


	return PROCESS_CMD_UNKNOWN;
}

/////////////////////////////////////////////////////////////////

bool AdapterFile::gatherStatsExt (NodeDOM * pParent) {
	return m_Stats.getStats(pParent, "AdapterFile");
}

/////////////////////////////////////////////////////////////////

void AdapterFile::setFlag_NoPause (bool bNoPause) {
	m_bNoPause = bNoPause;
}

/////////////////////////////////////////////////////////////////

bool AdapterFile::getFlag_NoPause () {
	return m_bNoPause;
}

/////////////////////////////////////////////////////////////////

bool AdapterFile::setValue_MaxBacklog (int nBack) {
	if(nBack <= 0) {
		logFaultHighEvent("Warning: Bad value passed in for maximum backlog, ignoring", "AdapterFile", "setValue_MaxBacklog");			
		return false;
	} else {
		m_nMaxPacketBacklog = nBack;
	}

	return true;
}

/////////////////////////////////////////////////////////////////

int AdapterFile::getValue_MaxBacklog () {
	return m_nMaxPacketBacklog;
}

/////////////////////////////////////////////////////////////////
