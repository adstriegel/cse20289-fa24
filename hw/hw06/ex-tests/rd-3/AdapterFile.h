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
 
#ifndef __ADAPTERFILE_H_
#define __ADAPTERFILE_H_
 
#include <vector>
#include <string>
#include <fstream>
using namespace std; 

 
#include "Adapter.h"
#include "../core/Timestamp.h"

class Packet;

// Code adapted from
//  http://www.netrino.com/Embedded-Systems/How-To/Big-Endian-Little-Endian

#define endianfixs(A) ((((uint16_t)(A) & 0xff00) >> 8) | \
                  (((uint16_t)(A) & 0x00ff) << 8))

#define endianfixl(A) ((((uint32_t)(A) & 0xff000000) >> 24) | \
                  (((uint32_t)(A) & 0x00ff0000) >> 8)  | \
                  (((uint32_t)(A) & 0x0000ff00) << 8)  | \
                  (((uint32_t)(A) & 0x000000ff) << 24))


// Statistics support

#include "../stat/Stats.h"

/** Enumerated values for the InputPassthrough module statistics
 */
enum eStatsAdapterFile {
    STAT_ADPFILE_PKTCOUNT,	
    STAT_ADPFILE_PKTDATA,
    STAT_ADPFILE_FILESPARSED,
    STAT_ADPFILE_LOOPSFINISHED,
    STAT_ADPFILE_LOOPSLEFT,
    STAT_ADPFILE_BADCAPLEN,
    STAT_ADPFILE_TOOLARGE,
    STAT_ADPFILE_EXCEEDBACKLOG,
    STAT_ADPFILE_EXCEEDBACKLOG_DURATION,
    STAT_ADPFILE_END	
};

/** Statistics for the AdapterFile  
 */
class StatsAdapterFile : public Stats
{
    public:
	StatsAdapterFile ();
	
	/** Retrieve the title for a given field
	 * @param nStat	    The index to the stat field
	 * @param szTitle   The char field to populate with the title
	 */
	virtual void		Get_Title (int nStat, char * szTitle);		
		
    private:
}; 


/** A class derived from the base Adapter class to encapsulate
 * read operations from a packet file (tcpdump).  The adapter itself
 * is read-only with the option to have write functionality
 * as necessary.  
 */
class AdapterFile : public Adapter {
    public:
	    AdapterFile ();
		~AdapterFile ();

		/** Dump the basic information to the console 
		 */		
		virtual void               dumpBasicInfo  ();
		
		/** Parse the DOM to populate the file where the DOM
		 * is generated from an XML input file. The DOM should contain
		 * the type of adapter and the underlying input and output
		 * modules. The extractExtended function takes in the 
		 * extended information specific to the child class. 
		 * @param pNode The base node to process for information
		 * @returns True if successful, false otherwise
		 */		
		bool                       extractExtendedDOM (NodeDOM * pNode);

		/** Read a packet and give it to the input chain
		 */
		void			           readPacket ();		
					
		/** Start up the device as called from the parent class.  The function
		 * simply calls openDevice to start up the threads for the class
		 */
		virtual void		       startDevice ();
		
		/** Write this specific packet to the adapter itself for
		 * direct output onto the network medium. For now, this simply goes
		 * into the ether as a file destination mechanism has not been
		 * set up.  
		 * @param pPacket The packet to add to the queue for output
		 */
		virtual void		       writePacket (Packet * pPacket);

		/** Set the file path on which to look for the files with the
		 * appropriate pattern
		 * @param sPath The path starting point to look for files
		 */
		void		               setFilePath (string sPath);
		
		/** Retrieve the file path for which to look for the files with
		 * the appropriate pattern
		 */
		string		               getFilePath ();
		
		/** Add an individual file for processing by the adapter. Note that this
		 * function is not thread-safe and is ideally only called once during the
		 * startup of the code.  The addFile will act as a relative path to the
		 * path specified via setFilePath. 
		 * Note that time does not carry between files.
		 * @param sFile The name of the file (including path if necessary)
		 * @returns True if the file was successfully located, false otherwise
		 */
		bool		               addFile (string sFile);
		
		/** Set the debug flag for this particular adapter
		 * @param sInfo Parameter setting (yes or all will enable all debugging)
		 * @returns True if valid parameter, false otherwise
		 */
		bool		               setDebugSetting (string sInfo);
			
		/** Add a set of files for processing relative to the path. This will cause
		 * the files matching the pattern to be added to the list for processing
		 * at the time of the call.  Pathing is relative to the getFilePath setting
		 * for the adapter. Path modifiers should not be included in the pattern but
		 * should rather be set using setFilePath.  
		 * Note that time does not carry between files.
		 * @param sPattern The pattern for file names to include
		 * @returns Positive count denoting the number of files added, negative if there
		 *           was an error
		 */
		int			               addFilesWithPattern (string sPattern);
		
		/** Retrieve the number of files noted by this adapter for processing
		 * @returns The number of files in the list
		 */
		int			               getFileCount ();
		
		/** Retrieve a specific file name from the list of files for playback
		 * @param nFile The index into the file name list to retrieve
		 * @returns The name of the file at the requested position if present,
		 *  otherwise an empty string 
		 */
		string		               getFile (int nFile);
		
		/** Set the flag denoting if the listing of files should be looped
		 * @param bLoop Boolean value denoting if the files should be looped (true) or not (false) 
		 */
		void		               setFlag_Loop (bool bLoop);
		
		/** Retrieve the flag denoting if the adapter should loop the file playback 
		 * in sequential order
		 * @returns True if the files should be looped, false otherwise 
		 */
		bool		               getFlag_Loop ();
		
		/** Set the flag denoting if timing information from the file should be ignored
		 * @param bNoPause Boolean value denoting if pauses should be ignored subject to a backlog limit
		 */
		void		               setFlag_NoPause (bool bNoPause);
		
		/** Retrieve the flag denoting if the timing information from the tcpdump file
		 * should be ignored when parsing packets
		 * @returns True if the values should be ignored, false otherwise 
		 */
		bool		               getFlag_NoPause ();		

		/** Set the value of the maximum number of packets that can exist in the input
		 * queue backlog.  This setting is only valid if the NoPause flag is set. The
		 * backlog refers to the number of packets. When the backlog is exceeded, the
		 * read thread will suspend until the adapter input queue begins to drop from
		 * packets being processed.    
		 * @param nMaxBacklog The new backlog level (in packets) 
		 * @returns True if successful, false otherwise
		 */ 
		bool		               setValue_MaxBacklog (int nMaxBacklog);

		/** Get the value of the maximum number of packets that can exist in the input
		 * queue backlog.  This setting is only valid if the NoPause flag is set. The
		 * backlog refers to the number of packets.  
		 * @returns The maximum size of the backlog that is possible
		 */ 
		int			               getValue_MaxBacklog ();
		
		/** Set the flag denoting if timing information should be biased from when the file was opened
		 * @param bNoPause Boolean value denoting if time should be adjusted relative to file open time
		 */
		void		               setFlag_AdjustRelative (bool bAdjustTime);
		
		/** Retrieve the flag denoting if the timing information should be adjusted relative to when
		 * the file was opened
		 * @returns True if the time should be adjusted, false otherwise 
		 */
		bool		               getFlag_AdjustRelative ();		
	
		/** Set the loop count for the files to be played back. A value of 0 denotes that
		 * the loop should continue infinitely
		 * @param nLoopCount Number of times to loop (0=infinit)
		 */
		void		               setValue_LoopCount (unsigned int nLoopCount);
		
		/** Retrieve the primary setting regarding how many times the file listing was
		 * set to be looped
		 * @returns The number of times to loop, initial setting (0=infinity)
		 */
		unsigned int	           getValue_LoopCount ();
		
	    /** Retrieve the number of loops remaining to be executed
		 * @returns The number of loops remaining to execute (-1 if infinite)
		 */
		int	                       getRemainingLoopCount ();
		
		/** Set the multiplier for the playback rate.  By default, the rate will
		 * be as close to real-time as possible subject to the file read rate. By
		 * default, the rate will be set at 1.
		 * @param fVal The muliplier to the playback rate where a number less than
		 *   one accelerates, a number greater than one slows playback 
		 * @returns True if successful, false otherwise
		 */
		bool		               setValue_PlaybackRate (float fVal);
		
		/** Retrieve the multiplier for the playback rate
		 * @returns A positive multiplier where > 1 slows and < 1 accelerates
		 */
		float		               getValue_PlaybackRate ();
		
		/** Process a command from the console or external control
		 * mechanism
		 * @param theCommands The vector of commands to process
		 * @param nOffset The offset at which to begin processing. The offset allows
		 *   us to pass the vector itself with modifications to the underlying
		 *   modules
		 * @returns Enumerated value representing result of command processing
		 */ 
		CommandResult	           processExtCommand (const vector<string> & theCommands, int nOffset);		
		
		/** Gather extended statistics (typically adapter-centric stats)
		 * @param pParent The DOM node under which to place stats for this adapter
		 * @returns True if successful, false otherwise
		 */
		virtual bool               gatherStatsExt (NodeDOM * pParent);		
		

		void                       setFlag_AlignTimes (bool bShouldAlign);
		bool                       getFlag_AlignTimes ();

    protected:
		
	    /** Open the file in the list requested and parse the initial header
	     * information
	     */
	    bool		               openFile (int nFile);
		
	    /** Validate all files on the list for processing
	     * @returns True if successful, false otherwise
	     */
	    bool		               validateFiles ();
		
    private:
	
	    // Do we include partials where we do not have all of the packet?
	    bool					   m_bAllowPartial;

	    unsigned int               m_nLoopCount;
	    int		                   m_nRemainingLoops;
	    bool		               m_bShouldLoop;
	    bool		               m_bPlaybackDone;				// Is the playback done overall?
		
	    // Debug level for the particular module
	    int		                   m_nDebugLevel;
		
	    // Playback settings
	    //
	    // The boolean value offers a lazy evaluation out to avoid floating point computations when the
	    // playback rate modifier is not being used. The playback rate contains a positive, non-zero number
	    // where a value less than one accelerates playback (less gap between packets) and a value greater
	    // than one slows 
	    bool		               m_bAccelPlayback;
	    float                      m_fPlaybackRate;
	
	    string                     m_sFilePath;
			
	    // Don't pause, read the file as fast as possible
	    bool		               m_bNoPause;
	    int		                   m_nMaxPacketBacklog;
		
	    // Set time based on current upon file open time (i.e. adjust with initial relative offset)
	    bool		               m_bAdjustTime;
	    Timestamp	               m_AdjustTime;		


	    // Flag to denote if we should align the time of the files as we read them to stay contiguous,
	    // i.e. file zero gets left as is, files 1 .. N start at the time from when the last set of
	    // data was read.
	    //
	    // XML Configuration:
	    // 		<aligntimes>No</aligntimes>
	    //		<!-- Yes or No -->

	    // This is basically a two-stage process for every packet in the file
	    //		Remove the time bias / start of the file
	    //		Add in based on our accumulated time

	    bool			           m_bAlignTime_Files;			// Default is false
	    Timestamp	 	           m_Time_Align_Accumulated;	// Total accumulated time, adjust at the end of a file
	    Timestamp		           m_Time_CurrentFileAdjust;	// How much do we adjust the file currently open?
												// We remove that from every entry and add accum time

	    // Listing of file names relative the above specified file path
	    vector<string> 	           m_FileNames;
		
	    // Currently open file object
	    ifstream	               m_CurrentFile;				// Stream into the currently open file
	    int		                   m_nCurrentFileIndex;			// Which file are we on?
	    bool		               m_bFlipEndian;			    // Endian-ness of file is different than current arch
		
		
	    // Statistics for the adapter (beyond module stats)
	    StatsAdapterFile           m_Stats;
};

#define DEFAULT_ADAPTERFILE_LOOPCOUNT		0
#define DEFAULT_ADAPTERFILE_SHOULDLOOP		false
#define	DEFAULT_ADAPTERFILE_PLAYBACKRATE	1.0
#define DEFAULT_ADAPTERFILE_MAXBACKLOG		1000

#endif /*__ADAPTERFILE_H_*/
