/*
 * fmnc_test_sequence.cc
 *
 *  Created on: Jun 4, 2014
 *      Author: striegel
 */

#include <iostream>
#include <fstream>
#include <math.h>
#include <assert.h>
using namespace std;

#include "../util/ip-utils.h"
#include "../util/ParamDictionary.h"

#include "fmnc_manager.h"
#include "fmnc_test_sequence.h"
#include "fmnc_measurement_packet.h"

///////////////////////////////////////////////////

FMNC_Transmit_BufferBlock::FMNC_Transmit_BufferBlock ()
{
	m_lLength = 0;
	m_pData = NULL;
	m_bOwnData = true;
	m_lPosition = 0;
}

///////////////////////////////////////////////////

void FMNC_Transmit_BufferBlock::setData (char * pData)
{
	m_pData = pData;
}

///////////////////////////////////////////////////

void FMNC_Transmit_BufferBlock::populateSimple (int nMode)
{
	// Giant comment for the purposes of rendering

	// Assume we know the size

	// TODO: Make this faster (maybe)

	m_pData[0] = '\n';
	m_pData[1] = '<';
	m_pData[2] = '!';
	m_pData[3] = '-';
	m_pData[4] = '-';
	m_pData[5] = ' ';

	int		j;

	for(j=6; j<m_lLength-5; j++)
	{
		m_pData[j] = 'A' + ((j-5)%26);
	}

	m_pData[m_lLength-5] = ' ';
	m_pData[m_lLength-4] = '-';
	m_pData[m_lLength-3] = '-';
	m_pData[m_lLength-2] = '>';
	m_pData[m_lLength-1] = '\n';
}

///////////////////////////////////////////////////

char * FMNC_Transmit_BufferBlock::getData ()
{
	return m_pData;
}

///////////////////////////////////////////////////

void FMNC_Transmit_BufferBlock::setLength (uint32_t lLength)
{
	m_lLength = lLength;
}

///////////////////////////////////////////////////

uint32_t FMNC_Transmit_BufferBlock::getLength ()
{
	return m_lLength;
}

///////////////////////////////////////////////////

void FMNC_Transmit_BufferBlock::setFlag_Ownership (bool bOwnData)
{
	m_bOwnData = bOwnData;
}

///////////////////////////////////////////////////

bool FMNC_Transmit_BufferBlock::getFlag_Ownership ()
{
	return m_bOwnData;
}

///////////////////////////////////////////////////

bool FMNC_Transmit_BufferBlock::allocateBlock (uint32_t lLength)
{
	m_pData = new char[lLength];

	if(m_pData == NULL)
	{
		m_lLength = 0;
		return false;
	}
	else
	{
		m_lLength = lLength;
		return true;
	}
}

///////////////////////////////////////////////////

void FMNC_Transmit_BufferBlock::setPosition (uint32_t lPosition)
{
	m_lPosition = lPosition;
}

///////////////////////////////////////////////////

uint32_t FMNC_Transmit_BufferBlock::getPosition ()
{
	return m_lPosition;
}



FMNC_Transmit_Buffer::FMNC_Transmit_Buffer ()
{
	m_bFillGarbage = true;
	m_nFillMode = 0;
}

FMNC_Transmit_Buffer::~FMNC_Transmit_Buffer ()
{
	int		j;

	for(j=0; j<m_Data.size(); j++)
	{
		if(m_Data[j]->getFlag_Ownership() && m_Data[j]->getData() != NULL)
		{
			delete m_Data[j]->getData();
			m_Data[j]->setData(NULL);
		}
	}

}

void FMNC_Transmit_Buffer::updatePositions ()
{
	int		j;
	uint32_t	lTotal;

	lTotal = 0;

	for(j=0; j<m_Data.size(); j++)
	{
		m_Data[j]->setPosition(lTotal);
		lTotal += m_Data[j]->getLength();
	}
}

uint32_t FMNC_Transmit_Buffer::copyData (char * pDestination, uint32_t lStart, uint32_t lLength)
{
	// TODO: Consider removing this later
	updatePositions();

	// Do this recursively if appropriate

	int 	nPos;

	nPos = findBufferWithPosition(lStart);

	if(nPos < 0)
	{
		// Error, we copied nothing
		cerr << " * Error: Could not copy starting at byte " << lStart << " of length " << lLength << endl;
		return 0;
	}

	// OK, we have a good block.  Now does this block contain everything we need or do we need to
	//  span one or more blocks (will do that recursively)

	uint32_t	lOffset;

	// How far into this block are we currently?
	lOffset = lStart - m_Data[nPos]->getPosition();

	if(m_Data[nPos]->getLength() - lOffset >= lLength)
	{
		// We are done with this one here
		memcpy(pDestination, m_Data[nPos]->getData()+lOffset, lLength);
		return lLength;
	}
	else
	{
		// Nope, more than just this block
		uint32_t	lPartialCover;

		lPartialCover = m_Data[nPos]->getLength()-lOffset;

		// Step 1 - Copy what we do have in this block
		memcpy(pDestination, m_Data[nPos]->getData()+lOffset, lPartialCover);

		return lPartialCover + copyData(pDestination+lPartialCover, lStart+lPartialCover, lLength-lPartialCover);
	}

	cerr << "* Warning: Unable to copy data with regards to FMNC_Transmit_Buffer, unexpected issue" << endl;
	return 0;
}

int	 FMNC_Transmit_Buffer::findBufferWithPosition (uint32_t lPosition)
{
	int		j;

	uint32_t	lRemaining;

	lRemaining = lPosition;

	for(j=0; j<m_Data.size(); j++)
	{
		if(lRemaining < m_Data[j]->getLength())
		{
			return j;
		}

		lRemaining -= m_Data[j]->getLength();
	}

	// Nope, did not find it at all
	return -1;
}

bool FMNC_Transmit_Buffer::getFlag_FillGarbage ()
{
	return m_bFillGarbage;
}

void FMNC_Transmit_Buffer::setFlag_FillGarbage (bool bFlag)
{
	m_bFillGarbage = bFlag;
}

int FMNC_Transmit_Buffer::getFillMode ()
{
	return m_nFillMode;
}

void FMNC_Transmit_Buffer::setFillMode (int nMode)
{
	m_nFillMode = nMode;
}

bool FMNC_Transmit_Buffer::addBufferBlock (FMNC_Transmit_BufferBlock * pBlock)
{
	if(pBlock != NULL)
	{
		m_Data.push_back(pBlock);
		return true;
	}
	else
	{
		cerr << " Error: Did not add a null buffer block to the transmission buffer" << endl;
		cerr << "   fmnc_test_sequence.cc (addBufferBlock)" << endl;
		return false;
	}
}

void	FMNC_Transmit_Buffer::dump_TransmitBuffer (bool bDumpContent)
{
	cout << "Transmit Buffer Information" << endl;
	cout << "   Total Bytes: " << getTotalLength() << endl;
	cout << endl;
	cout << "   Total Buffers: " << getCount() << endl;
	cout << endl;

	for(int j=0; j<getCount(); j++)
	{
		cout << " * Buffer " << j << "(" << m_Data[j]->getLength() << " bytes)" <<  endl;

		if(bDumpContent)
		{
			for(int k=0; k<m_Data[j]->getLength(); k++)
			{
				printf("%c", m_Data[j]->getData()[k]);
			}

			printf("\n\n");
		}
	}
}


bool	FMNC_Transmit_Buffer::swapOrder (int nSwap1, int nSwap2)
{
	if(nSwap1 >= getCount() || nSwap2 >= getCount())
	{
		return false;
	}

	if(nSwap1 < 0 || nSwap2 < 0)
	{
		return false;
	}

	if(nSwap1 == nSwap2)
	{
		return false;
	}

	FMNC_Transmit_BufferBlock 	*	pTemp;

	pTemp = m_Data[nSwap1];
	m_Data[nSwap1] = m_Data[nSwap2];
	m_Data[nSwap2] = pTemp;

	return true;
}

uint32_t FMNC_Transmit_Buffer::getTotalLength ()
{
	int		j;
	uint32_t	lTotal;

	lTotal = 0;

	for(j=0; j<m_Data.size(); j++)
	{
		lTotal += m_Data[j]->getLength();
	}

	return lTotal;
}

///////////////////////////////////////////////////////////////////////////////////

int	FMNC_Transmit_Buffer::getCount ()
{
	return m_Data.size();
}

///////////////////////////////////////////////////////////////////////////////////

bool FMNC_Transmit_Buffer::removeBlock (int nBlock)
{
	if(nBlock >= getCount() || nBlock < 0)
	{
		return false;
	}

	FMNC_Transmit_BufferBlock 	*	pTemp;

	pTemp = m_Data[nBlock];
	m_Data.erase(m_Data.begin()+nBlock);

	delete pTemp;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////

FMNC_Transmission_TCP::FMNC_Transmission_TCP()
{
	m_pParent = NULL;
	m_lStart = 0;
	m_lLength = 0;
	m_lSeqNum = 0;
	m_bSetPush = false;
    m_timeOut.tv_sec = 0;
    m_timeOut.tv_usec = 0;

	m_nGroup = FMNC_MEASUREMENT_GROUP_NONE;
	m_lExpectedAck = 0;
}

uint16_t	FMNC_Transmission_TCP::getGroup ()
{
	return m_nGroup;
}

void		FMNC_Transmission_TCP::setGroup (uint16_t nGroup)
{
	m_nGroup = nGroup;
}

uint32_t	FMNC_Transmission_TCP::getExpectedAckNumber ()
{
	return m_lExpectedAck;
}

void		FMNC_Transmission_TCP::setExpectedAckNumber (uint32_t nAckNum)
{
	m_lExpectedAck = nAckNum;
}

///////////////////////////////////////////////////////////////////////////////////

bool	FMNC_Transmission_TCP::getFlag_PSH ()
{
	return m_bSetPush;
}

///////////////////////////////////////////////////////////////////////////////////

void	FMNC_Transmission_TCP::setFlag_PSH (bool bFlagPush)
{
	m_bSetPush = bFlagPush;
}

///////////////////////////////////////////////////////////////////////////////////

void FMNC_Transmission_TCP::setParent (FMNC_Test_Sequence * pParent)
{
	m_pParent = pParent;
}

///////////////////////////////////////////////////////////////////////////////////

FMNC_Test_Sequence * FMNC_Transmission_TCP::getParent ()
{
	return m_pParent;
}

void FMNC_Transmission_TCP::setStart (uint32_t lStart)
{
	m_lStart = lStart;
}

uint32_t FMNC_Transmission_TCP::getStart ()
{
	return m_lStart;
}

void 	FMNC_Transmission_TCP::setLength (uint32_t lLength)
{
	m_lLength = lLength;
}

uint32_t FMNC_Transmission_TCP::getLength()
{
	return m_lLength;
}

void FMNC_Transmission_TCP::setSeqNum (uint32_t lSeqNum)
{
	m_lSeqNum = lSeqNum;
}

uint32_t FMNC_Transmission_TCP::getSeqNum()
{
	return m_lSeqNum;
}

void FMNC_Transmission_TCP::adjustSeqNum (uint32_t lBase)
{
//	cout << "    Sequence number is " << m_lSeqNum;
	m_lSeqNum += lBase;

//	cout << " changing to " << m_lSeqNum << endl;
}

void FMNC_Transmission_TCP::incrementTime (timeval * pAdjTime)
{
	timeval 	adjTime;

	// TODO: Fix this better in the future to actually add two.  We are taking advantage of the
	//       fact that the base time in this is unlikely to be anything but microseconds
	adjTime = *pAdjTime;

	adjustTime(&adjTime, m_timeOut.tv_usec);

	m_timeOut = adjTime;
}

void FMNC_Transmission_TCP::setOutTime (uint32_t lSec, uint32_t lUS)
{
	m_timeOut.tv_sec = lSec;
	m_timeOut.tv_usec = lUS;
    m_spacingUS = lUS;
}

uint32_t FMNC_Transmission_TCP::getOutTime_us ()
{
	return m_spacingUS;
}




///////////////////////////////////////////////////////////////////////////////////

FMNC_Test_Sequence::FMNC_Test_Sequence ()
{
	m_pConnection = NULL;
	m_nNextToSend = 0;

	m_nSliceSize = FMNC_DEFAULT_SLICE_SIZE;
	m_nSliceSpace_Sec = FMNC_DEFAULT_SLICE_SPACE_SEC;
	m_nSliceSpace_MicroSec = FMNC_DEFAULT_SLICE_SPACE_MICROSEC;

	pthread_mutex_init(&m_MutexList, NULL);

	m_pDictionary = NULL;
	m_bFlag_CreateDictionary = true;
}

FMNC_Test_Sequence::~FMNC_Test_Sequence ()
{
	if(m_pDictionary != NULL)
	{
		delete m_pDictionary;
		m_pDictionary = NULL;
	}

	for(int j=0; j<m_Transmissions.size(); j++)
	{
		FMNC_Transmission_TCP * pTransmission;

		pTransmission = m_Transmissions[j];
		delete pTransmission;
	}

	m_Transmissions.clear();
}

void FMNC_Test_Sequence::setNextToSend (int nNext)
{
	m_nNextToSend = nNext;
}

void FMNC_Test_Sequence::incrementNextToSend ()
{
	m_nNextToSend++;
}

int FMNC_Test_Sequence::getNextToSend ()
{
	return m_nNextToSend;
}

bool FMNC_Test_Sequence::isDone ()
{
	bool	bResult;

	pthread_mutex_lock(&m_MutexList);

	if(m_nNextToSend >= m_Transmissions.size())
	{
		bResult = true;
	}
	else
	{
		bResult = false;
	}

	pthread_mutex_unlock(&m_MutexList);

	return bResult;
}

bool FMNC_Test_Sequence::populatePacketWithNextSend (Packet * pPacket, uint32_t * pStime)
{
	pthread_mutex_lock(&m_MutexList);

//	cout << "Populating packet " << m_nNextToSend << " in the test sequence of a total of " << m_Transmissions.size()-1 << " segments" << endl;

	FMNC_Transmission_TCP *	pTransmit;

	if(m_nNextToSend >= m_Transmissions.size())
	{
		cout << "* Error: Asked to send the block but do not have anything to send" << endl;
		return false;
	}

	pTransmit = m_Transmissions[m_nNextToSend];

    *pStime=pTransmit->getOutTime_us(); //lsong
//	printf("   Test sequence at 0x%X\n", pTransmit);


	//cout << "   Slice of " << pTransmit->getLength() << " bytes with a sequence number of " << pTransmit->getSeqNum() << ":" << (pTransmit->getSeqNum()+pTransmit->getLength()) << endl;

	// TODO : Fix this to use better lengths rather than hard coding
    /* Again, hard coding sucks. I have to manually add the TCP timestamp option. 12 is the tcp timestamp option */
    uint8_t n_tcp_timestamp = 0;
	pPacket->setLength(14+20+20+n_tcp_timestamp+pTransmit->getLength());

//	cout << "      Total Packet Length: " << pPacket->getLength() << endl;

	// TODO: Fix the absolute length (No L2 for IP length FYI)
	setPacketLength(pPacket->getData(), pTransmit->getLength() + L3_IPV4_OFFSET + L4_TCP_LENGTH_NO_OPTIONS + n_tcp_timestamp );

	// Copy in our data to the payload
	char * pPayload;

	// TODO : Fix this to use better lengths rather than hard coding
	pPayload = pPacket->getData() + L2_OFFSET + L3_IPV4_OFFSET + L4_TCP_LENGTH_NO_OPTIONS + n_tcp_timestamp;

	// Where does this particular packet need to get its data from?
	uint32_t		lCopyBytes;

	lCopyBytes = m_TransmitBuffer.copyData(pPayload, pTransmit->getStart(), pTransmit->getLength());

	if(lCopyBytes < pTransmit->getLength())
	{
		cerr << "* Error: Unable to copy the full payload over, only " << lCopyBytes << " copied." << endl;
	}

	// Clear any flags (we don't ACK while slicing)
	//clearTCP_Flags(pPacket->getData());

	if(pTransmit->getFlag_PSH())
	{
//		cout << "**** SETTING TCP PSH FLAG *****" << endl;
		setTCPFlag_PSH(pPacket->getData(), pPacket->getLength(), 1);
	}
	else
	{
		setTCPFlag_PSH(pPacket->getData(), pPacket->getLength(), 0);
	}

	// Set the sequence number
	writeTCP_SeqNumber(pTransmit->getSeqNum(), pPacket->getData(), pPacket->getLength());


	updateIPv4_Checksum(pPacket->getData(), pPacket->getLength());
	updateTCP_Checksum(pPacket->getData(), pPacket->getLength());
	updateIPv4_Checksum(pPacket->getData(), pPacket->getLength());

	// Move to the next item to send
	incrementNextToSend();

	pthread_mutex_unlock(&m_MutexList);



	return true;
}


void FMNC_Test_Sequence::addTransmit (FMNC_Transmission_TCP * pTransmit)
{
	m_Transmissions.push_back(pTransmit);
}

FMNC_Connection_TCP_Slice *	FMNC_Test_Sequence::getConnection ()
{
	return m_pConnection;
}


void FMNC_Test_Sequence::setConnection (FMNC_Connection_TCP_Slice * pConnection)
{
	m_pConnection = pConnection;
}

void FMNC_Test_Sequence::adjustSeqNumber (uint32_t lSeqNum)
{
	int		j;

	pthread_mutex_lock(&m_MutexList);

	cout << "* Adjust the sequence number from a base of " << lSeqNum << endl;

	for(j=0; j<m_Transmissions.size(); j++)
	{
		m_Transmissions[j]->adjustSeqNum (lSeqNum);
	}

	pthread_mutex_unlock(&m_MutexList);
}


void FMNC_Test_Sequence::adjustDispatchTime (timeval * pAdjTime)
{
	int		j;

	pthread_mutex_lock(&m_MutexList);

	for(j=0; j<m_Transmissions.size(); j++)
	{
		m_Transmissions[j]->incrementTime(pAdjTime);
	}

	pthread_mutex_unlock(&m_MutexList);
}

uint32_t	FMNC_Test_Sequence::loadFileIntoBuffers (string sFile)
{
	string		sRead;
	string		sTotal;
	ifstream	theFile;
	streampos 	size;
	FMNC_Transmit_BufferBlock	*	pBlock;

	theFile.open(sFile.c_str(), ios::in|ios::binary|ios::ate);

	if (theFile.is_open())
	{
	    size = theFile.tellg();

		pBlock = new FMNC_Transmit_BufferBlock();
		pBlock->allocateBlock(size);
		pBlock->setLength(size);
		theFile.seekg (0, ios::beg);

		theFile.read(pBlock->getData(), size);

	    if (!theFile)
	    {
	      cerr << "Read only " << theFile.gcount() << " could be read" << endl;
	      return theFile.gcount();
	    }


		theFile.close();
	}
	else
	{
		cerr << "* Error: Failed to open the file of name " << sFile << endl;
		return 0;
	}

	m_TransmitBuffer.addBufferBlock(pBlock);
	return size;
}

string  FMNC_Test_Sequence::generate_iframe (uint32_t lID)
{
	string	siFrame;
	char 	szTemp[40];

	/* siFrame = "<iframe src=\"http://"+m_pConnection->getAddress_Dest()->toString()+":40080/tr/"; */
    /* Connect the Z440 Django server */
	siFrame = "http://fmnc.cse.nd.edu:8000/result/";
	/* siFrame = "<script type='text/javascript'>$(\"#result\").load(\"http://google.com"; */

	sprintf(szTemp, "%d-%ld.xml", lID,m_pConnection->getTime_Creation()->tv_sec);
	siFrame += szTemp;

	/* siFrame += " width=\"600\" height=\"400\"></iframe>"; */
	siFrame += "\" class=\"button alt scrolly big\">Check Out Your Test Result</a></li> </ul> </div> </section> ";

	cout << "iframe info: " << siFrame << endl;

	return siFrame;
}

bool FMNC_Test_Sequence::wrapFile_PNG (uint16_t nFixed, uint16_t nStepSeq, uint16_t nStepMult)
{
	// Front Matter

	// HTTP/1.1 200 OK
	// Date: Fri, 03 Feb 2006 03:34:33 GMT
	// Server: Apache/1.3.33 (Unix) PHP/4.3.10
	// Content-Length: 14582
	// Content-Type: text/html; charset=utf-8

	char 	szTemp[100];
	string	sTemp;
	time_t  rawtime;
	int	   	nFileLength;
	FMNC_Transmit_BufferBlock	*	pBlock;

	pBlock = new FMNC_Transmit_BufferBlock();
	string 	sFrontHeader;

	sFrontHeader = "HTTP/1.1 200 OK\n";

	// Get the time
	time (&rawtime);
	sprintf(szTemp, "%s", ctime(&rawtime));
	sTemp = szTemp;

	sFrontHeader += "Date: " + sTemp;

	// Server type
	sFrontHeader += "Server: ScaleBox:1.0.0 (Unix)\n";

	// Content-Length
	nFileLength = 0;

	ifstream	theFile;
	string		sFile;

	sFile = "fmnc/support/web/test.png";

	streampos 	imageFileSize;

	imageFileSize = 0;
	theFile.open(sFile.c_str(), ios::in|ios::binary|ios::ate);

	if (theFile.is_open())
	{
		imageFileSize = theFile.tellg();
	}
	else
	{
		cerr << "Error: Unable to parse file for image named " << sFile << " as it does not exist" << endl;
		delete pBlock;
		pBlock = NULL;
		return false;
	}

	sprintf(szTemp, "%d\n", (int) imageFileSize);
	sTemp = szTemp;
	sFrontHeader += sTemp;

	sFrontHeader += "Content-Type: image/png\n";

	sFrontHeader += "\n";

	pBlock->allocateBlock(sFrontHeader.size());
	pBlock->setLength(sFrontHeader.size());
	memcpy(pBlock->getData(), sFrontHeader.c_str(), sFrontHeader.size());
	m_TransmitBuffer.addBufferBlock(pBlock);

	// Load the last half of the web demo
	loadFileIntoBuffers("fmnc/support/web/test.png");

	uint32_t	lTestFixed;
	uint32_t	lTestNeed;
	uint32_t	lTestStep;
	int			j;

	FMNC_Transmission_TCP	*	pSlice;
	uint32_t		thePos;
	uint32_t		theCount;

	thePos = 0;
	theCount = 0;
	int		k;


	// Fixed size measurements
	for(j=0; j<nFixed; j++)
	{
		pSlice = new FMNC_Transmission_TCP();

		if(pSlice == NULL)
		{
			cerr << "** ERROR: Unable to allocate a new TCP Transmission Sequence" << endl;
			return false;
		}

		pSlice->setStart(thePos);
		pSlice->setSeqNum(pSlice->getStart());
		pSlice->setLength(m_nSliceSize);
		pSlice->setGroup(1);
		pSlice->setExpectedAckNumber(0);

		// 1000 microseconds
		pSlice->setOutTime(getSliceSpacing_Sec(), getSliceSpacing_MicroSec()*(theCount+1));
		pSlice->setFlag_PSH(false);
		m_Transmissions.push_back(pSlice);

		thePos += pSlice->getLength();
		theCount++;
	}

	// Stepped size measurements
	for(k=0; k<nStepSeq; k++)
	{
		for(j=0; j<nStepMult; j++)
		{
			pSlice = new FMNC_Transmission_TCP();
			pSlice->setGroup(2);
			pSlice->setExpectedAckNumber(0);

			if(pSlice == NULL)
			{
				cerr << "** ERROR: Unable to allocate a new TCP Transmission Sequence" << endl;
				return false;
			}

			pSlice->setStart(thePos);
			pSlice->setSeqNum(pSlice->getStart());
			pSlice->setLength(m_nSliceSize*(j+1));

			// 1000 microseconds
			pSlice->setOutTime(getSliceSpacing_Sec(), getSliceSpacing_MicroSec()*(theCount+1));
			pSlice->setFlag_PSH(false);
			m_Transmissions.push_back(pSlice);

			thePos += pSlice->getLength();
			theCount++;
		}
	}

	// Let's figure out how much is remaining
	imageFileSize-= thePos;

	uint16_t	nBonusSlices;

	nBonusSlices = 0;

	while(imageFileSize > 0)
	{
		pSlice = new FMNC_Transmission_TCP();

		if(pSlice == NULL)
		{
			cerr << "** ERROR: Unable to allocate a new TCP Transmission Sequence" << endl;
			return false;
		}

		pSlice->setStart(thePos);
		pSlice->setSeqNum(pSlice->getStart());

		if(imageFileSize >= m_nSliceSize)
		{
			pSlice->setLength(m_nSliceSize);
			imageFileSize -= m_nSliceSize;
		}
		else
		{
			pSlice->setLength(imageFileSize);
			imageFileSize = 0;
		}

		pSlice->setGroup(3);
		pSlice->setExpectedAckNumber(0);

		// 1000 microseconds
		pSlice->setOutTime(getSliceSpacing_Sec(), getSliceSpacing_MicroSec()*(theCount+1));
		pSlice->setFlag_PSH(false);
		m_Transmissions.push_back(pSlice);

		thePos += pSlice->getLength();
		theCount++;
		nBonusSlices++;
	}

	// Do our rearrangement pushing the first packet to the end of the outbound packets
	pSlice = m_Transmissions[0];
	m_Transmissions.erase(m_Transmissions.begin());
	m_Transmissions.push_back(pSlice);
	pSlice->setExpectedAckNumber(thePos);
	pSlice->setGroup(3);
	// Done, all set to go

	cout << "  Image slicing complete with " << nBonusSlices << " bonus slices of the fixed variant added at the end" << endl;

	if(getFlag_CreateDictionary())
	{
		char szTemp[60];
		string sData;

		if(!allocateDictionary())
		{
			return false;
		}

		// OK, we now have a valid dictionary

		// Test type
		m_pDictionary->addEntry("TestType", "Web-iFrame Result");

		// The total size of the TCP payloads
		sprintf(szTemp, "%d", m_TransmitBuffer.getTotalLength());
		sData = szTemp;
		m_pDictionary->addEntry("TestSize", sData);

		sprintf(szTemp, "%d", nFixed);
		sData = szTemp;
		m_pDictionary->addEntry("FixedPkts", sData);

		sprintf(szTemp, "%d", m_nSliceSize);
		sData = szTemp;
		m_pDictionary->addEntry("FixedSize", sData);

		sprintf(szTemp, "%d", m_nSliceSize);
		sData = szTemp;
		m_pDictionary->addEntry("VariablePktMin", sData);

		sprintf(szTemp, "%d", (nStepMult+1)*m_nSliceSize);
		sData = szTemp;
		m_pDictionary->addEntry("VariablePktMax", sData);

		sprintf(szTemp, "%d", m_nSliceSize);
		sData = szTemp;
		m_pDictionary->addEntry("VariablePktStep", sData);

		sprintf(szTemp, "%d", nStepSeq);
		sData = szTemp;
		m_pDictionary->addEntry("VariableSequences", sData);

		sprintf(szTemp, "%d", nStepMult);
		sData = szTemp;
		m_pDictionary->addEntry("VariableSeqPkts", sData);
	}

	return true;
}
void	FMNC_Test_Sequence::createTest_TrainSizeWebDemo (uint8_t type,uint16_t lpacketSize,uint16_t lpacketGap, uint16_t npacket, uint16_t Rmin, uint16_t Rmax, uint8_t lstep, uint32_t lID)
{
	FMNC_Transmit_BufferBlock	*	pBlock;
	uint32_t	lTestFixed;
	uint32_t	lTestNeed = 0;
	int			j;

	// How big of a payload do we need in total?
	// 	 Number of fixed samples * slice size
	//		Ex. 50 slices with 100 byte payloads --> 5k
	//   Stair step values
	//	    Step multiplier of 3, ex. 100, 200, 300

    //uint32_t lpacketSize[] = {224,224};

    //setSliceSize (lpacketSize);

    lTestFixed = npacket;
    uint8_t step=npacket/lstep;
    float t_rate = Rmin;
    for (int i=0;i<lTestFixed;i++){
            uint32_t t_size = lpacketSize;
            if (i % step == 0 && i > 0 ){
                    t_rate=t_rate+(Rmax-Rmin)*step/float(lTestFixed);
            }
            if (((lpacketSize)*8/float(t_rate) -5) > lpacketGap)
            {
                    t_size = t_rate*(lpacketGap+5)/8;
            }
            lTestNeed += t_size;
            //lTestNeed += 40*24;
            //cout<<"Calculating "<<i<<"rate "<<t_rate<<" sum "<<lTestNeed<<" now "<<t_size<<endl;
    }
    lTestNeed += 40*1;

    //uint32_t spacings[] = {80,100,140,206,417,2017};
    //uint32_t spacings[] = {2017,417,206,140,100,80};
    //uint32_t spacings[] = {450,571,763,1147,2299,11515};
    //uint32_t spacings[] = {2299,763,450};
    //uint32_t spacings[] = {1179,394,231};
    //uint32_t spacings[] = {231};



	cout << "Buffer needs are " << lTestNeed << " bytes " << endl;

	// Load the first part of the web demo
	loadFileIntoBuffers("fmnc/support/web/webDemo-Start.html");

	// Generate the iframe information here to request back to our custom server (yes, it is another request) that will have the
	//  d3js script / etc. to be rendered

	pBlock = new FMNC_Transmit_BufferBlock();
	string 	siFrame;

	siFrame = generate_iframe(lID);
	pBlock->allocateBlock(siFrame.size());
	pBlock->setLength(siFrame.size());
	memcpy(pBlock->getData(), siFrame.c_str(), siFrame.size());
	m_TransmitBuffer.addBufferBlock(pBlock);

	// Load the last half of the web demo
	loadFileIntoBuffers("fmnc/support/web/webDemo-End.html");

	// We can get away with pushing this until the end since nothing makes it up the TCP
	// stack due the rearrangement anyway

	if(m_TransmitBuffer.getTotalLength() < lTestNeed)
	{
		cout << "** Need additional bytes, generating comment-based content" << endl;
		cout << "   Have " << m_TransmitBuffer.getTotalLength() << " bytes, need " << (lTestNeed - m_TransmitBuffer.getTotalLength()) << " bytes more" << endl;

		pBlock = new FMNC_Transmit_BufferBlock();

		if((lTestNeed - m_TransmitBuffer.getTotalLength()) < 20)
		{
			pBlock->allocateBlock(lTestNeed - m_TransmitBuffer.getTotalLength());
			pBlock->setLength(lTestNeed - m_TransmitBuffer.getTotalLength());
		}
		else
		{
			pBlock->allocateBlock(lTestNeed - m_TransmitBuffer.getTotalLength());
			pBlock->setLength(lTestNeed - m_TransmitBuffer.getTotalLength());
		}

		pBlock->populateSimple(0);
		m_TransmitBuffer.addBufferBlock(pBlock);

		// Swap this added one to be just before the end

		m_TransmitBuffer.swapOrder(m_TransmitBuffer.getCount()-1,m_TransmitBuffer.getCount()-2);
	}
	else
	{
		cout << "** No need for content, we are good to go" << endl;
	}

	FMNC_Transmission_TCP	*	pSlice;
	uint32_t		thePos;
	uint32_t		theCount;

	thePos = 0;
	theCount = 0;
	int		k;

	// Fixed size measurements
    int swap=step,swap_count=0;//lsong
    uint32_t jj=0;
    float rate = Rmin;
    uint32_t blocksize=22000/(lpacketSize);
	for(j=0; j<lTestFixed + 40; j++) //Turn on/off the tail by adding/deleting the +40 part
	{
		pSlice = new FMNC_Transmission_TCP();

		if(pSlice == NULL)
		{
			cerr << "** ERROR: Unable to allocate a new TCP Transmission Sequence" << endl;
			return;
		}

        uint32_t t_size = lpacketSize;

		// 1000 microseconds
        //if(j % 10 == 0 && j > 0)
        //        rate = rate + 2;
        //uint32_t first_stage=40;
        //if ( j < first_stage)
        //        rate=rate+15/float(first_stage);
        //else
        //        rate=rate+20/float(lTestFixed - first_stage);

        uint32_t gap = 0;
        type = 0;
        if (type == 1){
                if (j>0 && j % blocksize != 0){
                        rate=rate+(Rmax-Rmin)/float(lTestFixed);
                        //pSlice->setOutTime(0, 100000);
                        gap = (lpacketSize)*8/float(rate) -5;
                }
                else if (j % blocksize == 0 && j > 0)
                        gap = 200000;
                else
                        gap=(lpacketSize)*8/float(rate) -5;
        }
        else if (type == 0){
                if (j % step == 0 && j > 0 ){
                        rate=rate+(Rmax-Rmin)*step/float(lTestFixed);
                }
                gap = (lpacketSize)*8/float(rate) -5;
        }
        if (gap > lpacketGap)
        {
                gap = lpacketGap;
                t_size = rate*(gap+5)/8;
        }
        //This is used to vary the packet gap for searching the optimal packet gap setting
        //t_size=1400;
        //gap=512;
        //rate = (t_size*8 + 40)/(5+gap);
        //////////////////////////////////////////////////////////////////////////
        if (j > lTestFixed - 1){
                gap = 10000;
                t_size = 1;
        }
        // Adjust the gap according to packet sequence
        //gap = gap - 2*(100 - 20*int(j/20));
        //t_size = rate*(gap+5)/8;

		pSlice->setStart(thePos);
		pSlice->setSeqNum(pSlice->getStart());

        pSlice->setLength(t_size);
        pSlice->setOutTime(0,gap);
        //pSlice->setLength(t_size);
		pSlice->setGroup(1);
		pSlice->setExpectedAckNumber(0);

        //cout << "fmnc_test_sequence: "<<j << "rate "<<rate<<"gap "<<gap<<" size "<<t_size<<endl;
       // if(j<npacket[0]){
       //         pSlice->setOutTime(0, (lpacketSize+40)*8/float(rate) -5);
       //         //cout<<" spacing at test sequence "<<spacings[jj]<<" at "<<j<<endl;
       // }
       // else if (j == npacket[1]){
       //         pSlice->setOutTime(0, 50000);
       // }
       // else{
       //         if(j % 10 == 0 && j > 0)
       //                 rate = rate + 2;
       //         //cout << "fmnc_test_sequence: "<<j << " "<<rate<<" "<<(lpacketSize+40)*8/float(rate) -5<<endl;
       //         pSlice->setOutTime(0, (lpacketSize+40)*8/float(rate) -5);
       //         //pSlice->setOutTime(0, 5000);
       //         //jj++;
       // }

        //pSlice->setOutTime(0, spacings[j/npacket]);
        //if(j%npacket == 0){
        //        pSlice->setOutTime(0, 500000);
        //        //cout<<" spacing at test sequence "<<spacings[j/50]<<" at "<<j<<endl;
        //}
        //else{
        //        pSlice->setOutTime(0, 1);
        //        //cout<<" spacing at test sequence "<<3000000<<" at "<<j<<endl;
        //}
		pSlice->setFlag_PSH(false);
		m_Transmissions.push_back(pSlice);

		thePos += pSlice->getLength();
		theCount++;

        swap_count++;
        if(swap_count == swap)
        {
                pSlice = m_Transmissions[m_Transmissions.size()-swap];
                m_Transmissions.erase(m_Transmissions.end()-swap);
                m_Transmissions.push_back(pSlice);
                pSlice->setExpectedAckNumber(thePos);
                pSlice->setGroup(3);
                swap_count = 0;

        }
	}

	// Do our rearrangement pushing the first packet to the end of the outbound packets
//	pSlice = m_Transmissions[0];
//	m_Transmissions.erase(m_Transmissions.begin());
//	m_Transmissions.push_back(pSlice);
//	pSlice->setExpectedAckNumber(thePos);
//	pSlice->setGroup(3);
	// Done, all set to go

	if(getFlag_CreateDictionary())
	{
		char szTemp[60];
		string sData;

		if(!allocateDictionary())
		{
			return;
		}

		// OK, we now have a valid dictionary

		// Test type
		m_pDictionary->addEntry("TestType", "Web-iFrame Result");

		// The total size of the TCP payloads
		sprintf(szTemp, "%d", m_TransmitBuffer.getTotalLength());
		sData = szTemp;
		m_pDictionary->addEntry("TestSize", sData);

		sprintf(szTemp, "%d", m_Transmissions.size());
		sData = szTemp;
		m_pDictionary->addEntry("TestPkts", sData);

		sprintf(szTemp, "%d", lTestFixed);
		sData = szTemp;
		m_pDictionary->addEntry("FixedPkts", sData);

	}

}

void	FMNC_Test_Sequence::createTest_TrainGapWebDemo (uint8_t type,uint16_t nGap, uint16_t npacket, uint16_t Rmin, uint16_t Rmax, uint32_t lID)
{
	FMNC_Transmit_BufferBlock	*	pBlock;
	uint32_t	lTestFixed;
	uint32_t	lTestNeed;
	int			j;

	// How big of a payload do we need in total?
	// 	 Number of fixed samples * slice size
	//		Ex. 50 slices with 100 byte payloads --> 5k
	//   Stair step values
	//	    Step multiplier of 3, ex. 100, 200, 300

    //uint32_t lpacketSize[] = {224,224};

    //setSliceSize (lpacketSize);

    lTestFixed = npacket;

    //uint32_t spacings[] = {80,100,140,206,417,2017};
    //uint32_t spacings[] = {2017,417,206,140,100,80};
    //uint32_t spacings[] = {450,571,763,1147,2299,11515};
    //uint32_t spacings[] = {2299,763,450};
    //uint32_t spacings[] = {1179,394,231};
    //uint32_t spacings[] = {231};


    lTestNeed  = 0;
    for(uint8_t i=0;i<lTestFixed;i++)
    {
            float rate=(Rmin+i*(Rmax-Rmin)/float(lTestFixed-1));
            uint16_t t_size = rate*nGap/8.0;
            if ( t_size > 1350 ){
                    t_size = 1350;
            }
            lTestNeed += t_size;
    }


	cout << "Buffer needs are " << lTestNeed << " bytes " << endl;

	// Load the first part of the web demo
	loadFileIntoBuffers("fmnc/support/web/webDemo-Start.html");

	// Generate the iframe information here to request back to our custom server (yes, it is another request) that will have the
	//  d3js script / etc. to be rendered

	pBlock = new FMNC_Transmit_BufferBlock();
	string 	siFrame;

	siFrame = generate_iframe(lID);
	pBlock->allocateBlock(siFrame.size());
	pBlock->setLength(siFrame.size());
	memcpy(pBlock->getData(), siFrame.c_str(), siFrame.size());
	m_TransmitBuffer.addBufferBlock(pBlock);

	// Load the last half of the web demo
	loadFileIntoBuffers("fmnc/support/web/webDemo-End.html");

	// We can get away with pushing this until the end since nothing makes it up the TCP
	// stack due the rearrangement anyway

	if(m_TransmitBuffer.getTotalLength() < lTestNeed)
	{
		cout << "** Need additional bytes, generating comment-based content" << endl;
		cout << "   Have " << m_TransmitBuffer.getTotalLength() << " bytes, need " << (lTestNeed - m_TransmitBuffer.getTotalLength()) << " bytes more" << endl;

		pBlock = new FMNC_Transmit_BufferBlock();

		if((lTestNeed - m_TransmitBuffer.getTotalLength()) < 20)
		{
			pBlock->allocateBlock(lTestNeed - m_TransmitBuffer.getTotalLength());
			pBlock->setLength(lTestNeed - m_TransmitBuffer.getTotalLength());
		}
		else
		{
			pBlock->allocateBlock(lTestNeed - m_TransmitBuffer.getTotalLength());
			pBlock->setLength(lTestNeed - m_TransmitBuffer.getTotalLength());
		}

		pBlock->populateSimple(0);
		m_TransmitBuffer.addBufferBlock(pBlock);

		// Swap this added one to be just before the end

		m_TransmitBuffer.swapOrder(m_TransmitBuffer.getCount()-1,m_TransmitBuffer.getCount()-2);
	}
	else
	{
		cout << "** No need for content, we are good to go" << endl;
	}

	FMNC_Transmission_TCP	*	pSlice;
	uint32_t		thePos;
	uint32_t		theCount;

	thePos = 0;
	theCount = 0;
	int		k;

	// Fixed size measurements
    int swap=2,swap_count=0;//lsong
    uint32_t jj=0;
    float rate = Rmin;
	for(j=0; j<lTestFixed; j++)
	{
		pSlice = new FMNC_Transmission_TCP();

		if(pSlice == NULL)
		{
			cerr << "** ERROR: Unable to allocate a new TCP Transmission Sequence" << endl;
			return;
		}


        rate=(Rmin+j*(Rmax-Rmin)/float(lTestFixed-1));
        uint32_t t_size=rate*nGap/8.0;
        uint32_t t_gap=nGap;
        if ( t_size > 1350 ){
                t_size = 1350;
                t_gap = 1350*8/rate;
        }
                
        pSlice->setStart(thePos);
        pSlice->setSeqNum(pSlice->getStart());

		pSlice->setLength(t_size);
		pSlice->setGroup(1);
		pSlice->setExpectedAckNumber(0);

		// 1000 microseconds
        //if(j % 10 == 0 && j > 0)
        //        rate = rate + 2;
        //uint32_t first_stage=40;
        //if ( j < first_stage)
        //        rate=rate+15/float(first_stage);
        //else
        //        rate=rate+20/float(lTestFixed - first_stage);


       pSlice->setOutTime(0,t_gap);

        cout << "fmnc_test_sequence: "<<j << " "<<rate<<"Gap "<<t_gap<<" Size "<<t_size<<endl;
       // if(j<npacket[0]){
       //         pSlice->setOutTime(0, (lpacketSize+40)*8/float(rate) -5);
       //         //cout<<" spacing at test sequence "<<spacings[jj]<<" at "<<j<<endl;
       // }
       // else if (j == npacket[1]){
       //         pSlice->setOutTime(0, 50000);
       // }
       // else{
       //         if(j % 10 == 0 && j > 0)
       //                 rate = rate + 2;
       //         //cout << "fmnc_test_sequence: "<<j << " "<<rate<<" "<<(lpacketSize+40)*8/float(rate) -5<<endl;
       //         pSlice->setOutTime(0, (lpacketSize+40)*8/float(rate) -5);
       //         //pSlice->setOutTime(0, 5000);
       //         //jj++;
       // }

        //pSlice->setOutTime(0, spacings[j/npacket]);
        //if(j%npacket == 0){
        //        pSlice->setOutTime(0, 500000);
        //        //cout<<" spacing at test sequence "<<spacings[j/50]<<" at "<<j<<endl;
        //}
        //else{
        //        pSlice->setOutTime(0, 1);
        //        //cout<<" spacing at test sequence "<<3000000<<" at "<<j<<endl;
        //}
		pSlice->setFlag_PSH(false);
		m_Transmissions.push_back(pSlice);

		thePos += pSlice->getLength();
		theCount++;

        swap_count++;
        if(swap_count == swap)
        {
                pSlice = m_Transmissions[m_Transmissions.size()-swap];
                m_Transmissions.erase(m_Transmissions.end()-swap);
                m_Transmissions.push_back(pSlice);
                pSlice->setExpectedAckNumber(thePos);
                pSlice->setGroup(3);
                swap_count = 0;

        }
	}

	// Do our rearrangement pushing the first packet to the end of the outbound packets
//	pSlice = m_Transmissions[0];
//	m_Transmissions.erase(m_Transmissions.begin());
//	m_Transmissions.push_back(pSlice);
//	pSlice->setExpectedAckNumber(thePos);
//	pSlice->setGroup(3);
	// Done, all set to go

	if(getFlag_CreateDictionary())
	{
		char szTemp[60];
		string sData;

		if(!allocateDictionary())
		{
			return;
		}

		// OK, we now have a valid dictionary

		// Test type
		m_pDictionary->addEntry("TestType", "Web-iFrame Result");

		// The total size of the TCP payloads
		sprintf(szTemp, "%d", m_TransmitBuffer.getTotalLength());
		sData = szTemp;
		m_pDictionary->addEntry("TestSize", sData);

		sprintf(szTemp, "%d", m_Transmissions.size());
		sData = szTemp;
		m_pDictionary->addEntry("TestPkts", sData);

		sprintf(szTemp, "%d", lTestFixed);
		sData = szTemp;
		m_pDictionary->addEntry("FixedPkts", sData);

	}

}

void	FMNC_Test_Sequence::createTest_WebDemo (uint16_t nFixed, uint16_t nStepSeq, uint16_t nStepMult, uint32_t lID)
{
	FMNC_Transmit_BufferBlock	*	pBlock;
	uint32_t	lTestFixed;
	uint32_t	lTestNeed;
	uint32_t	lTestStep;
	int			j;

	// How big of a payload do we need in total?
	// 	 Number of fixed samples * slice size
	//		Ex. 50 slices with 100 byte payloads --> 5k
	//   Stair step values
	//	    Step multiplier of 3, ex. 100, 200, 300

	lTestFixed = nFixed * m_nSliceSize;

	lTestStep = 0;
	for(j=0; j<nStepMult; j++)
	{
		lTestStep += (j+1)*m_nSliceSize; 
	}

	lTestStep *= nStepSeq;

	lTestNeed = lTestFixed + lTestStep;

	cout << "Buffer needs are " << lTestNeed << " bytes, " << lTestFixed << " fixed, " << lTestStep << " stepped" << endl;

	// Load the first part of the web demo
	loadFileIntoBuffers("fmnc/support/web/webDemo-Start.html");

	// Generate the iframe information here to request back to our custom server (yes, it is another request) that will have the
	//  d3js script / etc. to be rendered

	pBlock = new FMNC_Transmit_BufferBlock();
	string 	siFrame;

	siFrame = generate_iframe(lID);
	pBlock->allocateBlock(siFrame.size());
	pBlock->setLength(siFrame.size());
	memcpy(pBlock->getData(), siFrame.c_str(), siFrame.size());
	m_TransmitBuffer.addBufferBlock(pBlock);

	// Load the last half of the web demo
	loadFileIntoBuffers("fmnc/support/web/webDemo-End.html");

	// We can get away with pushing this until the end since nothing makes it up the TCP
	// stack due the rearrangement anyway

	if(m_TransmitBuffer.getTotalLength() < lTestNeed)
	{
		cout << "** Need additional bytes, generating comment-based content" << endl;
		cout << "   Have " << m_TransmitBuffer.getTotalLength() << " bytes, need " << (lTestNeed - m_TransmitBuffer.getTotalLength()) << " bytes more" << endl;

		pBlock = new FMNC_Transmit_BufferBlock();

		if((lTestNeed - m_TransmitBuffer.getTotalLength()) < 20)
		{
			pBlock->allocateBlock(lTestNeed - m_TransmitBuffer.getTotalLength());
			pBlock->setLength(lTestNeed - m_TransmitBuffer.getTotalLength());
		}
		else
		{
			pBlock->allocateBlock(lTestNeed - m_TransmitBuffer.getTotalLength());
			pBlock->setLength(lTestNeed - m_TransmitBuffer.getTotalLength());
		}

		pBlock->populateSimple(0);
		m_TransmitBuffer.addBufferBlock(pBlock);

		// Swap this added one to be just before the end

		m_TransmitBuffer.swapOrder(m_TransmitBuffer.getCount()-1,m_TransmitBuffer.getCount()-2);
	}
	else
	{
		cout << "** No need for content, we are good to go" << endl;
	}

	FMNC_Transmission_TCP	*	pSlice;
	uint32_t		thePos;
	uint32_t		theCount;

	thePos = 0;
	theCount = 0;
	int		k;

	// Fixed size measurements
    int swap=5,swap_count=0;//lsong
	for(j=0; j<nFixed; j++)
	{
		pSlice = new FMNC_Transmission_TCP();

		if(pSlice == NULL)
		{
			cerr << "** ERROR: Unable to allocate a new TCP Transmission Sequence" << endl;
			return;
		}

		pSlice->setStart(thePos);
		pSlice->setSeqNum(pSlice->getStart());
        pSlice->setLength(m_nSliceSize);
		pSlice->setGroup(1);
		pSlice->setExpectedAckNumber(0);
        /* if(j % 20 == 0) */
        /*         pSlice->setOutTime(getSliceSpacing_Sec(), 20000); */
        /* else */
                pSlice->setOutTime(getSliceSpacing_Sec(), getSliceSpacing_MicroSec());


        /* if(j % 3  == 2) */
        /*         pSlice->setOutTime(getSliceSpacing_Sec(), 300); */
        /* else */
		pSlice->setFlag_PSH(false);
		m_Transmissions.push_back(pSlice);

		thePos += pSlice->getLength();
		theCount++;

        swap_count++;
        if(swap_count == swap)
        {
                /* pSlice = m_Transmissions[m_Transmissions.size()-swap]; */
                /* m_Transmissions.erase(m_Transmissions.end()-swap); */
                /* m_Transmissions.push_back(pSlice); */
                /* pSlice->setExpectedAckNumber(thePos); */
                /* pSlice->setGroup(3); */
                /* swap_count = 0; */

                pSlice = m_Transmissions[m_Transmissions.size()-swap];
                uint32_t p_gap = m_Transmissions.back()->getOutTime_us();
                m_Transmissions.back()->setOutTime(getSliceSpacing_Sec(),pSlice->getOutTime_us());
                pSlice->setOutTime(getSliceSpacing_Sec(),p_gap);
                m_Transmissions[m_Transmissions.size()-swap] = m_Transmissions.back();
                m_Transmissions[m_Transmissions.size() - 1] = pSlice;
                pSlice->setExpectedAckNumber(thePos);
                pSlice->setGroup(3);
                swap_count = 0;

        }
        
       // FMNC_Transmission_TCP *	pTrans;
       // pTrans = m_Transmissions.back();
       // cout<<"DBG(lsong) in test_sequence: the spacing "<<pTrans->getOutTime_us()<<endl;
	}

	// Stepped size measurements
    swap_count = 0;
	for(k=0; k<nStepSeq; k++)
	{
		for(j=0; j<nStepMult; j++)
		{
			pSlice = new FMNC_Transmission_TCP();
			pSlice->setGroup(2);
			pSlice->setExpectedAckNumber(0);

			if(pSlice == NULL)
			{
				cerr << "** ERROR: Unable to allocate a new TCP Transmission Sequence" << endl;
				return;
			}

			pSlice->setStart(thePos);
			pSlice->setSeqNum(pSlice->getStart());
			pSlice->setLength(m_nSliceSize*(j+1));

			// 1000 microseconds
			pSlice->setOutTime(getSliceSpacing_Sec(), getSliceSpacing_MicroSec()*(theCount+1));
			pSlice->setFlag_PSH(false);
			m_Transmissions.push_back(pSlice);

			thePos += pSlice->getLength();
			theCount++;


            swap_count++;
            if(swap_count == swap)
            {
                    pSlice = m_Transmissions[m_Transmissions.size()-swap];
                    m_Transmissions.erase(m_Transmissions.end()-swap);
                    m_Transmissions.push_back(pSlice);
                    pSlice->setExpectedAckNumber(thePos);
                    pSlice->setGroup(3);
                    swap_count = 0;

            }
		}
	}

	// Do our rearrangement pushing the first packet to the end of the outbound packets
//	pSlice = m_Transmissions[0];
//	m_Transmissions.erase(m_Transmissions.begin());
//	m_Transmissions.push_back(pSlice);
//	pSlice->setExpectedAckNumber(thePos);
//	pSlice->setGroup(3);
	// Done, all set to go

	if(getFlag_CreateDictionary())
	{
		char szTemp[60];
		string sData;

		if(!allocateDictionary())
		{
			return;
		}

		// OK, we now have a valid dictionary

		// Test type
		m_pDictionary->addEntry("TestType", "Web-iFrame Result");

		// The total size of the TCP payloads
		sprintf(szTemp, "%d", m_TransmitBuffer.getTotalLength());
		sData = szTemp;
		m_pDictionary->addEntry("TestSize", sData);

		sprintf(szTemp, "%d", m_Transmissions.size());
		sData = szTemp;
		m_pDictionary->addEntry("TestPkts", sData);

		sprintf(szTemp, "%d", nFixed);
		sData = szTemp;
		m_pDictionary->addEntry("FixedPkts", sData);

		sprintf(szTemp, "%d", m_nSliceSize);
		sData = szTemp;
		m_pDictionary->addEntry("FixedSize", sData);

		sprintf(szTemp, "%d", m_nSliceSize);
		sData = szTemp;
		m_pDictionary->addEntry("VariablePktMin", sData);

		sprintf(szTemp, "%d", (nStepMult+1)*m_nSliceSize);
		sData = szTemp;
		m_pDictionary->addEntry("VariablePktMax", sData);

		sprintf(szTemp, "%d", m_nSliceSize);
		sData = szTemp;
		m_pDictionary->addEntry("VariablePktStep", sData);

		sprintf(szTemp, "%d", nStepSeq);
		sData = szTemp;
		m_pDictionary->addEntry("VariableSequences", sData);

		sprintf(szTemp, "%d", nStepMult);
		sData = szTemp;
		m_pDictionary->addEntry("VariableSeqPkts", sData);

	}

}

/* double my_rate_function(uint32_t x,double a,double b, double c) */
/* { */
/*         return a*pow(x-b,3)/3.0 + c; */
/* } */
double calc_n(double a,uint32_t n)
{
        double rate = 0.5;
        uint16_t MTU = 1350;
        for(int i=0;i<n;i++){
                double gap = MTU*8/rate;
                if(gap > 3334)
                        rate += a;
                else
                        rate += a*gap/3334.0;
        }
        return rate;
}
double find_alpha(uint32_t n, double Rmax){
        double left_a = 0.00001, right_a = 10.0;
        while(right_a - left_a > 0.005){
                if(calc_n((left_a+right_a)/2.0,n)>Rmax)
                        right_a = (left_a+right_a)/2.0;
                else
                        left_a = (left_a+right_a)/2.0;
        }
        return (left_a+right_a)/2.0;
}

void	FMNC_Test_Sequence::createTest_TrainFACellularWebDemo (uint16_t Rmin, uint16_t Rmax, double Alpha, uint32_t lID)
{
	FMNC_Transmit_BufferBlock	*	pBlock;
	uint32_t	lTestFixed; //packet number
	uint32_t	lTestNeed; //data volumn
	int			j,i;

    /* Size is in Bytes, Gap is in microsecond, rate is in Mb/s */
    uint16_t MTU = 1350;
    int swap=5,swap_count=0;//lsong

    /* if(Alpha < 1.0){ */
    /*         cerr<<"** (Error): the increasing factor Alpha is less than 1"<<endl; */
    /*         return; */
    /* } */



    lTestNeed = 0;
    lTestFixed = 0;
    
    uint16_t size_, gap_;
    double rate_;
    float max_viable_gap = 3334;

    vector<uint16_t> subtrain_size;
    vector<uint16_t> subtrain_gap;
    /* timestamp calculate pair */
    subtrain_size.push_back(MTU);
    subtrain_gap.push_back(40000);
    lTestNeed += MTU;
    lTestFixed++;
    /* I also use lTestFixed as the index of rate increment. */
    
    uint32_t sum_gap_ = 0;
    rate_ = (double) 0.5;
    double Beta = find_alpha((uint32_t) Alpha, (double)Rmax);
    while(rate_ < Rmax){
            if(MTU*8/rate_ > max_viable_gap && false){
                    gap_ = max_viable_gap;
                    size_ = max_viable_gap * rate_/8;
            }else{
                    size_ = MTU;
                    gap_ = size_ * 8/rate_;
            }
            if (gap_ > 3334){
                    rate_ += Beta;
            }
            else
                    rate_ += Beta*gap_/3334.0;

            if(lTestFixed % 20 == 0)
                    gap_ = 20000;

            subtrain_size.push_back(size_);
            subtrain_gap.push_back(gap_);
            lTestFixed++;
            lTestNeed += size_;
            /* printf(" (DEBUG) Cellular Train: %d-th pkt with size %d bytes and gap %d us (Beta %f); probe rate %5.2f Mbps\n",lTestFixed, size_,gap_,Beta,rate_); */
            if (lTestNeed > 1000000){
                    cerr<<"** (Error/Warning) The train gets too big. Let's stop the Rmax at "<<rate_<<" Mbps"<<endl;
                    break;
            }

    }

    while((lTestFixed) % swap != 0  ){
            subtrain_size.push_back(size_);
            subtrain_gap.push_back(size_ * 8/rate_);
            lTestNeed += size_;
            lTestFixed++;
    }
    /* Calculate the cost of the train */
    float sum_time = 0;
    assert(subtrain_gap.size() == subtrain_size.size());
    for(int c=0;c<subtrain_gap.size();c++)
            sum_time += subtrain_gap[c];
    cout<<"(INFO) Time cost is "<<sum_time<<" us;";
    cout<<" Pkts cost is "<<subtrain_gap.size();
    cout<<" Data cost is "<<lTestNeed/1000.0<<" KB"<<endl;

	cout << "Buffer needs are " << lTestNeed << " bytes " <<  endl;

	// Load the first part of the web demo
	loadFileIntoBuffers("fmnc/support/web/webDemo-Start.html");

	// Generate the iframe information here to request back to our custom server (yes, it is another request) that will have the
	//  d3js script / etc. to be rendered

	pBlock = new FMNC_Transmit_BufferBlock();
	string 	siFrame;

	siFrame = generate_iframe(lID);
	pBlock->allocateBlock(siFrame.size());
	pBlock->setLength(siFrame.size());
	memcpy(pBlock->getData(), siFrame.c_str(), siFrame.size());
	m_TransmitBuffer.addBufferBlock(pBlock);

	// Load the last half of the web demo
	loadFileIntoBuffers("fmnc/support/web/webDemo-End.html");

	// We can get away with pushing this until the end since nothing makes it up the TCP
	// stack due the rearrangement anyway

	if(m_TransmitBuffer.getTotalLength() < lTestNeed)
	{
		cout << "** Need additional bytes, generating comment-based content" << endl;
		cout << "   Have " << m_TransmitBuffer.getTotalLength() << " bytes, need " << (lTestNeed - m_TransmitBuffer.getTotalLength()) << " bytes more" << endl;

		pBlock = new FMNC_Transmit_BufferBlock();

		if((lTestNeed - m_TransmitBuffer.getTotalLength()) < 20)
		{
			pBlock->allocateBlock(lTestNeed - m_TransmitBuffer.getTotalLength());
			pBlock->setLength(lTestNeed - m_TransmitBuffer.getTotalLength());
		}
		else
		{
			pBlock->allocateBlock(lTestNeed - m_TransmitBuffer.getTotalLength());
			pBlock->setLength(lTestNeed - m_TransmitBuffer.getTotalLength());
		}

		pBlock->populateSimple(0);
		m_TransmitBuffer.addBufferBlock(pBlock);

		// Swap this added one to be just before the end

		m_TransmitBuffer.swapOrder(m_TransmitBuffer.getCount()-1,m_TransmitBuffer.getCount()-2);
	}
	else
	{
		cout << "** No need for content, we are good to go" << endl;
	}

	FMNC_Transmission_TCP	*	pSlice;
	uint32_t		thePos;
	uint32_t		theCount;

	thePos = 0;
	theCount = 0;

	for(j=0; j<lTestFixed;j++)
	{

		pSlice = new FMNC_Transmission_TCP();

		if(pSlice == NULL)
		{
			cerr << "** ERROR: Unable to allocate a new TCP Transmission Sequence" << endl;
			return;
		}

        /* uint16_t rate = Rmin + (j*nStepSeq/nFixed)*(Rmax - Rmin)/(nStepSeq - 1); */
        /* uint16_t n =   Gap * rate/(MTU*8) + 1 ; */
        /* cout<<" n = "<<n<<endl; */
        /* uint16_t size = ( rate * Gap/8 ) > MTU ? MTU:( rate * Gap/8 ); */
        /* uint16_t size = Gap * rate/(8 * n); */
        /* uint16_t gap = ( size * 8 )/rate ; */
        /* if( n > 1 ) */
        /*          gap = 40; */
                 /* gap = MTU*8*n/(rate*(n-1)) - Gap/(n-1); */

		pSlice->setStart(thePos);
		pSlice->setSeqNum(pSlice->getStart());
        pSlice->setLength(subtrain_size[j]);
		pSlice->setGroup(1);
		pSlice->setExpectedAckNumber(0);

        /* if( rate_index  == n ){ */
        /*         pSlice->setOutTime(getSliceSpacing_Sec(), Gap); */
        /*         rate_index = 1; */
        /* } */
        /* else{ */
        /*         pSlice->setOutTime(getSliceSpacing_Sec(), gap); */
        /*         rate_index++; */
        /* } */
        pSlice->setOutTime(getSliceSpacing_Sec(), subtrain_gap[j]);
		pSlice->setFlag_PSH(false);
		m_Transmissions.push_back(pSlice);

		thePos += pSlice->getLength();
		theCount++;

        swap_count++;
        if(swap_count == swap)
        {
                pSlice = m_Transmissions[m_Transmissions.size()-swap];
                uint32_t p_gap = m_Transmissions.back()->getOutTime_us();
                m_Transmissions.back()->setOutTime(getSliceSpacing_Sec(),pSlice->getOutTime_us());
                pSlice->setOutTime(getSliceSpacing_Sec(),p_gap);
                m_Transmissions[m_Transmissions.size()-swap] = m_Transmissions.back();
                m_Transmissions[m_Transmissions.size() - 1] = pSlice;
                /* m_Transmissions.erase(m_Transmissions.end()-swap); */
                //pSlice->setOutTime(getSliceSpacing_Sec(), getSliceSpacing_MicroSec()*2000);
                pSlice->setExpectedAckNumber(thePos);
                pSlice->setGroup(3);
                swap_count = 0;

        }
        
	}

}


void	FMNC_Test_Sequence::createTest_TrainFAWebDemo (uint16_t nFixed, uint16_t nStepSeq, uint16_t Rmin, uint16_t Rmax, uint16_t Gap,uint32_t lID)
{
	FMNC_Transmit_BufferBlock	*	pBlock;
	uint32_t	lTestFixed;
	uint32_t	lTestNeed;
	int			j;

    /* Size is in Bytes, Gap is in microsecond, rate is in Mb/s */
    uint16_t MTU = 1390;
    int swap=5,swap_count=0;//lsong
    uint16_t min_subtrain_l = 20;


    if(nStepSeq < 2){
            cerr<<"*Error: Step size must be greater than 1."<<endl;
            return;
    }

    lTestNeed = 0;
    lTestFixed = 0;
    uint16_t rate, the_n, size, gap;
    vector<uint16_t> subtrain_size;
    vector<uint16_t> subtrain_gap;
    for(int k=0; k<nStepSeq; k++){
        rate = Rmin + k*(Rmax - Rmin)/(nStepSeq - 1);
        the_n = Gap * rate/(MTU*8) + 1 ;
        /* cout<<"Rate "<<rate<<" n "<<the_n<<endl; */
        size = Gap * rate/(8 * the_n);
        gap = ( size * 8 )/rate ;
        uint16_t sub_pktN = max(nFixed,(uint16_t)( min_subtrain_l/the_n ));
        for(int kk = 0; kk < sub_pktN; kk++){
                for(int kkk = 0;kkk < the_n - 1;kkk++) {
                        subtrain_size.push_back(size);
                        subtrain_gap.push_back(40);
                        lTestNeed += size;
                        lTestFixed++;
                }
                subtrain_size.push_back(size);
                subtrain_gap.push_back(Gap);
                lTestNeed += size;
                lTestFixed++;
        }
        subtrain_size.push_back(64);
        subtrain_gap.push_back(2000);
        lTestNeed += 64;
        lTestFixed++;
    }
    while( lTestFixed % swap != 0  ){
            subtrain_size.push_back(size);
            subtrain_gap.push_back(40);
            lTestNeed += size;
            lTestFixed++;
    }
    /* Calculate the cost of the train */
    int sum = 0;
    cout<<"Data cost is "<<lTestNeed<<endl;
    sum = 0;
    for(int c=0;c<subtrain_gap.size();c++)
            sum += subtrain_gap[c];
    cout<<"Time cost is "<<sum<<" us"<<endl;
    cout<<"Pkts cost is "<<lTestFixed<<endl;

	cout << "Buffer needs are " << lTestNeed << " bytes " <<  endl;

	// Load the first part of the web demo
	loadFileIntoBuffers("fmnc/support/web/webDemo-Start.html");

	// Generate the iframe information here to request back to our custom server (yes, it is another request) that will have the
	//  d3js script / etc. to be rendered

	pBlock = new FMNC_Transmit_BufferBlock();
	string 	siFrame;

	siFrame = generate_iframe(lID);
	pBlock->allocateBlock(siFrame.size());
	pBlock->setLength(siFrame.size());
	memcpy(pBlock->getData(), siFrame.c_str(), siFrame.size());
	m_TransmitBuffer.addBufferBlock(pBlock);

	// Load the last half of the web demo
	loadFileIntoBuffers("fmnc/support/web/webDemo-End.html");

	// We can get away with pushing this until the end since nothing makes it up the TCP
	// stack due the rearrangement anyway

	if(m_TransmitBuffer.getTotalLength() < lTestNeed)
	{
		cout << "** Need additional bytes, generating comment-based content" << endl;
		cout << "   Have " << m_TransmitBuffer.getTotalLength() << " bytes, need " << (lTestNeed - m_TransmitBuffer.getTotalLength()) << " bytes more" << endl;

		pBlock = new FMNC_Transmit_BufferBlock();

		if((lTestNeed - m_TransmitBuffer.getTotalLength()) < 20)
		{
			pBlock->allocateBlock(lTestNeed - m_TransmitBuffer.getTotalLength());
			pBlock->setLength(lTestNeed - m_TransmitBuffer.getTotalLength());
		}
		else
		{
			pBlock->allocateBlock(lTestNeed - m_TransmitBuffer.getTotalLength());
			pBlock->setLength(lTestNeed - m_TransmitBuffer.getTotalLength());
		}

		pBlock->populateSimple(0);
		m_TransmitBuffer.addBufferBlock(pBlock);

		// Swap this added one to be just before the end

		m_TransmitBuffer.swapOrder(m_TransmitBuffer.getCount()-1,m_TransmitBuffer.getCount()-2);
	}
	else
	{
		cout << "** No need for content, we are good to go" << endl;
	}

	FMNC_Transmission_TCP	*	pSlice;
	uint32_t		thePos;
	uint32_t		theCount;

	thePos = 0;
	theCount = 0;

	for(j=0; j<lTestFixed;j++)
	{

		pSlice = new FMNC_Transmission_TCP();

		if(pSlice == NULL)
		{
			cerr << "** ERROR: Unable to allocate a new TCP Transmission Sequence" << endl;
			return;
		}

        /* uint16_t rate = Rmin + (j*nStepSeq/nFixed)*(Rmax - Rmin)/(nStepSeq - 1); */
        /* uint16_t n =   Gap * rate/(MTU*8) + 1 ; */
        /* cout<<" n = "<<n<<endl; */
        /* uint16_t size = ( rate * Gap/8 ) > MTU ? MTU:( rate * Gap/8 ); */
        /* uint16_t size = Gap * rate/(8 * n); */
        /* uint16_t gap = ( size * 8 )/rate ; */
        /* if( n > 1 ) */
        /*          gap = 40; */
                 /* gap = MTU*8*n/(rate*(n-1)) - Gap/(n-1); */
        /* cout<<"Debug Frame Aggregation train: j "<<j<<" size "<<subtrain_size[j]<<" gap "<<subtrain_gap[j]<<endl; */

		pSlice->setStart(thePos);
		pSlice->setSeqNum(pSlice->getStart());
        pSlice->setLength(subtrain_size[j]);
		pSlice->setGroup(1);
		pSlice->setExpectedAckNumber(0);

        /* if( rate_index  == n ){ */
        /*         pSlice->setOutTime(getSliceSpacing_Sec(), Gap); */
        /*         rate_index = 1; */
        /* } */
        /* else{ */
        /*         pSlice->setOutTime(getSliceSpacing_Sec(), gap); */
        /*         rate_index++; */
        /* } */
        pSlice->setOutTime(getSliceSpacing_Sec(), subtrain_gap[j]);
		pSlice->setFlag_PSH(false);
		m_Transmissions.push_back(pSlice);

		thePos += pSlice->getLength();
		theCount++;

        swap_count++;
        if(swap_count == swap)
        {
                pSlice = m_Transmissions[m_Transmissions.size()-swap];
                uint32_t p_gap = m_Transmissions.back()->getOutTime_us();
                m_Transmissions.back()->setOutTime(getSliceSpacing_Sec(),pSlice->getOutTime_us());
                pSlice->setOutTime(getSliceSpacing_Sec(),p_gap);
                m_Transmissions[m_Transmissions.size()-swap] = m_Transmissions.back();
                m_Transmissions[m_Transmissions.size() - 1] = pSlice;
                /* m_Transmissions.erase(m_Transmissions.end()-swap); */
                //pSlice->setOutTime(getSliceSpacing_Sec(), getSliceSpacing_MicroSec()*2000);
                pSlice->setExpectedAckNumber(thePos);
                pSlice->setGroup(3);
                swap_count = 0;

        }
        
	}

}


void FMNC_Test_Sequence::predefined_2 (bool bFlip)
{
	FMNC_Transmission_TCP *	pPredef;

	pPredef = new FMNC_Transmission_TCP();

	pPredef->setParent(this);

	// Create the data first off
	FMNC_Transmit_BufferBlock	*	pBlock;

	pBlock = new FMNC_Transmit_BufferBlock();

	// Yes, this is bad

	string		sRead;
	string		sTotal;
	ifstream	theFile;
	streampos size;

	theFile.open("fmnc/index-test.html", ios::in|ios::binary|ios::ate);

	if (theFile.is_open())
	{
	    size = theFile.tellg();

		pBlock = new FMNC_Transmit_BufferBlock();
		pBlock->allocateBlock(size);
		pBlock->setLength(size);
		theFile.seekg (0, ios::beg);

		theFile.read(pBlock->getData(), size);

	    if (!theFile)
	    {
	      cerr << "Read only " << theFile.gcount() << " could be read" << endl;
	    }


		theFile.close();
	}
	else
	{
		cerr << "* Error: Failed to open the file" << endl;
		return;
	}

	m_TransmitBuffer.addBufferBlock(pBlock);


	FMNC_Transmission_TCP	*	pSlice;
	int		j;
	uint32_t		thePos;
	uint32_t		theCount;

	j = 0;
	thePos = 0;
	theCount = 0;


	while(size > 0)
	{
		pSlice = new FMNC_Transmission_TCP();

		if(pSlice == NULL)
		{
			cerr << "** ERROR: Unable to allocate a new TCP Transmission Sequence" << endl;
			return;
		}

		if(size <= getSliceSize())
		{
			pSlice->setStart(thePos);
			pSlice->setSeqNum(pSlice->getStart());
			pSlice->setLength(size);
			// 1000 microseconds
			pSlice->setOutTime(getSliceSpacing_Sec(), getSliceSpacing_MicroSec()*(theCount+1));
			pSlice->setFlag_PSH(true);
			m_Transmissions.push_back(pSlice);

			thePos += size;
			theCount++;
			size = 0;

//			thePos += sliceSize;
//			theCount++;
//			size = 0;
//			break;
		}
		else
		{
			pSlice->setStart(thePos);
			pSlice->setSeqNum(pSlice->getStart());
			pSlice->setLength(getSliceSize());
			// 1000 microseconds
			pSlice->setOutTime(getSliceSpacing_Sec(), getSliceSpacing_MicroSec()*(theCount+1));
			m_Transmissions.push_back(pSlice);

			thePos += getSliceSize();
			theCount++;
			size -= getSliceSize();
		}
	}

	// The flip flag determines whether or not we are doing TCP Sting like
	//  interactions.
	if(bFlip)
	{
		pSlice = m_Transmissions[0];
		m_Transmissions.erase(m_Transmissions.begin());
		m_Transmissions.push_back(pSlice);
	}
}

void FMNC_Test_Sequence::constructFileWithParams (string sFile, ParamDictionary * pDictionary, bool bFlip)
{
	// TODO: Remove magic numbers and allocate appropriately (but in a static way)
	// Han: I don't know Chewie, fly casual ... but not too casual


	// TODO: This code is pretty darn ugly in that we are reusing existing code but should
	//   	 probably not be doing that but rather rolling new code

	// 20k limit (heh) + 1 for the null character
	char	szBuffer[20001];

	// Load the file into the first buffer
	loadFileIntoBuffers(sFile);

	cout << "* Loaded file (params) with total length of " << m_TransmitBuffer.getTotalLength() << endl;

	string		sData;

	// Copy the whole thing

	if(m_TransmitBuffer.getTotalLength() <= 20000)
	{
		m_TransmitBuffer.copyData (szBuffer, 0, m_TransmitBuffer.getTotalLength());
	}
	else
	{
		m_TransmitBuffer.copyData (szBuffer, 0, 20000);
		cerr << "Error: Unable to fit all of the requested file into the 20k buffer, copying only 20k" << endl;
	}

	// Fix the last character to be a null character
	szBuffer[m_TransmitBuffer.getTotalLength()] = 0x00;

	sData = szBuffer;

	string 	sResult;

	if(pDictionary != NULL)
	{
		sResult = pDictionary->replaceText(sData);
	}
	else
	{
		sResult = sData;
	}

	// Get rid of the first transmit buffer item
	m_TransmitBuffer.removeBlock(0);

	// Add the newly populated block into the buffer block for transmission
	FMNC_Transmit_BufferBlock	*	pBlock;

	pBlock = new FMNC_Transmit_BufferBlock();
	pBlock->allocateBlock(sResult.length());
	pBlock->setLength(sResult.length());

	cout << " Copying " << sResult.length() << " bytes" << endl;
	memcpy(pBlock->getData(),sResult.c_str(),sResult.length());
	m_TransmitBuffer.addBufferBlock(pBlock);

	// Debug statement
	cout << "** Data Response now prepped: " << pBlock->getLength() << " bytes" << endl;

	sliceFixed(m_nSliceSize, bFlip);

	cout << "** Slicing is finished: " << m_TransmitBuffer.getTotalLength() << " bytes" << endl;

	return;
}

void FMNC_Test_Sequence::sliceFixed (uint16_t nSlice, bool bFlip)
{
	if(nSlice == 0)
	{
		// Change it to be the MSS (roughly)
		nSlice = 1400;
	}

	uint32_t	nLengthRemaining;
	uint32_t	nPos;
	uint16_t	nSliceNum;

	nLengthRemaining = m_TransmitBuffer.getTotalLength();
	nPos = 0;
	nSliceNum = 0;

	FMNC_Transmission_TCP	*	pSlice;

    int swap=2,swap_count=0;//lsong: get swap working

	while(nLengthRemaining > 0)
	{
		cout << "Length Remaining: " << nLengthRemaining << endl;

		pSlice = new FMNC_Transmission_TCP();
		pSlice->setStart(nPos);
		pSlice->setSeqNum(pSlice->getStart());

		if(nLengthRemaining >= nSlice)
		{
			// Slice it up appropriately
			pSlice->setLength(nSlice);
		}
		else
		{
			pSlice->setLength(nLengthRemaining);
		}

		// 1000 microseconds
		pSlice->setOutTime(0, nSliceNum*1000);
		m_Transmissions.push_back(pSlice);

		nPos += pSlice->getLength();
		nLengthRemaining -= pSlice->getLength();
		nSliceNum++;

        swap_count++;
        if(swap_count == swap && bFlip)
        {
                pSlice = m_Transmissions[m_Transmissions.size()-swap];
                m_Transmissions.erase(m_Transmissions.end()-swap);
                m_Transmissions.push_back(pSlice);
                swap_count = 0;

        }
	}

	cout << " Flipping it around" << endl;

//	if(bFlip && m_Transmissions.size() > 1)
//	{
//		pSlice = m_Transmissions[0];
//		m_Transmissions.erase(m_Transmissions.begin());
//		m_Transmissions.push_back(pSlice);
//	}
}

void FMNC_Test_Sequence::populatePredefined (int nPredef)
{
	switch(nPredef)
	{
		case 2:
			setTestConfig("<Predefined Setting=\"2\" />");
			predefined_2(false);
			return;
		case 3:
			setTestConfig("<Predefined Setting=\"3\" />");
			predefined_2(true);
			return;
		case 4:
			setTestConfig("<Predefined Setting=\"4\" Fixed=\"50\" Multi=\"10\" Loops=\"2\" />");
			createTest_WebDemo (75,10,3,1);
			return;
	}

	FMNC_Transmission_TCP *	pPredef;

	pPredef = new FMNC_Transmission_TCP();

	pPredef->setParent(this);

	// Add a simple transmission sequence of 200 bytes

	// Create the data first off
	FMNC_Transmit_BufferBlock	*	pBlock;

	pBlock = new FMNC_Transmit_BufferBlock();
	pBlock->setData( new char[100]);
	pBlock->setLength(100);
	pBlock->populateSimple(0);

	m_TransmitBuffer.addBufferBlock(pBlock);

	pBlock = new FMNC_Transmit_BufferBlock();
	pBlock->setData( new char[200]);
	pBlock->setLength(200);
	pBlock->populateSimple(0);

	m_TransmitBuffer.addBufferBlock(pBlock);

	pBlock = new FMNC_Transmit_BufferBlock();
	pBlock->setData( new char[200]);
	pBlock->setLength(200);
	pBlock->populateSimple(0);

	m_TransmitBuffer.addBufferBlock(pBlock);

	pBlock = new FMNC_Transmit_BufferBlock();
	pBlock->setData( new char[200]);
	pBlock->setLength(200);
	pBlock->populateSimple(0);

	m_TransmitBuffer.addBufferBlock(pBlock);

	pBlock = new FMNC_Transmit_BufferBlock();
	pBlock->setData( new char[2000]);
	pBlock->setLength(2000);
	pBlock->populateSimple(0);

	m_TransmitBuffer.addBufferBlock(pBlock);


	FMNC_Transmission_TCP	*	pSlice;
	int		j;

	switch(nPredef)
	{
		case 0:
			// TODO: Add timing for the respective packets
			//		1 ms spacing starting from now

			for(j=1; j<6; j++)
			{
				pSlice = new FMNC_Transmission_TCP();
				pSlice->setStart(j*50);
				pSlice->setSeqNum(pSlice->getStart());
				pSlice->setLength(50);
				// 1000 microseconds
				pSlice->setOutTime(0, 1000*(j-1));
				m_Transmissions.push_back(pSlice);
			}

			pSlice = new FMNC_Transmission_TCP();
			pSlice->setStart(0);
			pSlice->setSeqNum(pSlice->getStart());
			pSlice->setLength(50);
			pSlice->setOutTime(0, 1000*5);
			m_Transmissions.push_back(pSlice);
			break;
		case 1:
			for(j=0; j<10; j++)
			{
				pSlice = new FMNC_Transmission_TCP();
				pSlice->setStart(j*50);
				pSlice->setSeqNum(pSlice->getStart());
				pSlice->setLength(50);
				// 1000 microseconds
				pSlice->setOutTime(0, 1000*(j-1));
				m_Transmissions.push_back(pSlice);
			}
			break;
	}


}

string	FMNC_Test_Sequence::getXML ()
{
	string		sXML;
	char		szTemp[25];
	string		sTemp;

	sXML = "<TestSequence ";

	sprintf(szTemp, "%d", m_Transmissions.size());
	sTemp = szTemp;
	sXML += "SeqLength=\"" + sTemp + "\"";

	sprintf(szTemp, "%d", getSliceSize());
	sTemp = szTemp;
	sXML += " SliceSize=\"" + sTemp + "\"";

	sprintf(szTemp, "%d.%d", getSliceSpacing_Sec(), getSliceSpacing_MicroSec());
	sTemp = szTemp;
	sXML += " SliceSpace=\"" + sTemp + "\"";

	sXML += ">";


	sXML += "<Config>";
	sXML += getTestConfig();
	sXML += "</Config>";

	sXML += "</TestSequence>";

	return sXML;
}

string 	FMNC_Test_Sequence::getTestConfig()
{
	return m_sTestConfig;
}

void	FMNC_Test_Sequence::setTestConfig(string sConfig)
{
	m_sTestConfig = sConfig;
}

uint32_t	FMNC_Test_Sequence::getTestLength ()
{
	int		j;
	uint32_t		lTotalData;

	lTotalData = 0;

	for(j=0; j<m_Transmissions.size(); j++)
	{
		lTotalData += m_Transmissions[j]->getLength();
	}

	return lTotalData;
}

uint16_t	FMNC_Test_Sequence::getSliceSize ()
{
	return m_nSliceSize;
}

void		FMNC_Test_Sequence::setSliceSize (uint16_t nSlice)
{
	m_nSliceSize = nSlice;
}

void		FMNC_Test_Sequence::setSliceSpacing (uint32_t lSpaceSec, uint32_t lSpaceMicroSec)
{
	m_nSliceSpace_Sec = lSpaceSec;
	m_nSliceSpace_MicroSec = lSpaceMicroSec;
}

uint32_t	FMNC_Test_Sequence::getSliceSpacing_Sec ()
{
	return m_nSliceSpace_Sec;
}

uint32_t	FMNC_Test_Sequence::getSliceSpacing_MicroSec ()
{
	return m_nSliceSpace_MicroSec;
}

FMNC_Transmit_Buffer *	FMNC_Test_Sequence::getTransmitBuffer ()
{
	return &m_TransmitBuffer;
}

void 	FMNC_Test_Sequence::dump_TransmitSequence ()
{
	cout << "Transmission Sequences (" << m_Transmissions.size() << ")" << endl;

	for(int j=0; j<m_Transmissions.size(); j++)
	{
		cout << "  Sequence " << j << ": " << m_Transmissions[j]->getSeqNum() << ":" << m_Transmissions[j]->getSeqNum() + m_Transmissions[j]->getLength() << " of length " << m_Transmissions[j]->getLength() << endl;
	}
}

void FMNC_Test_Sequence::setFlag_CreateDictionary (bool bCreateDict)
{
	m_bFlag_CreateDictionary = bCreateDict;
}

bool FMNC_Test_Sequence::getFlag_CreateDictionary ()
{
	return m_bFlag_CreateDictionary;
}

ParamDictionary * FMNC_Test_Sequence::getDictionary ()
{
	return m_pDictionary;
}

bool FMNC_Test_Sequence::allocateDictionary ()
{
	if(m_pDictionary != NULL)
	{
		return false;
	}
	else
	{
		m_pDictionary = new ParamDictionary();
		if(m_pDictionary != NULL)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}



