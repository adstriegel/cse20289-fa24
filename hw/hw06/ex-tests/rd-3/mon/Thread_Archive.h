
#ifndef THREAD_ARCHIVE_H_
#define THREAD_ARCHIVE_H_

#include <iostream>
using namespace std;

#include <deque>
using namespace std;

#include <fstream>
using namespace std;

#include <pthread.h>

#define MAX_ARCHIVE_STRING		10000

enum ARCHIVE_ITEM_TYPE {
	ARCHIVE_ITEM_NONE=0,
	ARCHIVE_ITEM_CONNECTION,
	ARCHIVE_ITEM_LOGSTRING
};

class Packet;

/// @brief Individual item to archive
class	ArchiveItem {
	public:
		ArchiveItem ();
		ArchiveItem (char * pString);
		
		~ArchiveItem ();
		
		void		setContent (int nType, void * pData);
		
		int			getType ();
		void * 	getData ();
				
	private:
		int			m_nType;
		void *		m_pData;
};

/// @brief The archive object 
/// The Archiver is responsible for offloading old content or logged
/// content to the disk.  
class Archiver {
	public:
		Archiver ();
		~Archiver ();
		
		/// @brief Dump the status of the archive to stdout
		/// Show how many items are pending to be archived and their respective
		/// status
		void 		dumpStatus ();
	
		/// @brief Add an item to be archived by the archive thread
		void		addItem (ArchiveItem * pItem);
	
		/// Make sure all of the appropriate directory structures are in place 
		/// for the archiver to operate
		void		initialize ();
	
		/// @brief Process one log item
		/// This function is called by the logging thread to push one item out to
		/// permanent storage
		/// @returns 0 if there was nothing in the queue, non-zero otherwise
		char 		doArchive ();
		
		void		archiveString(char * pString);
		
		void		archivePacket(Packet * pPacket);
		
		/// Write the current time directly out to the file
		void		archiveTime  ();
		
	protected:
		void		logConnection (ArchiveItem * pItem);
	
		void		logString		 (ArchiveItem * pItem);		
				
	
	private:
		// The output queue for items to be archived
		deque<ArchiveItem *>		m_Queue;
	
		// Mutex to protect the archive buffer during manipulation
		pthread_mutex_t m_ArchiveBuf;
		
		// Log file for output
		ofstream			m_LogFile;
	
};

/// @brief Thread for the archive mechanism
void * Thread_Archive (void * pArg);

// Globally accessible archiver
extern Archiver		g_theArchive;

#endif /*THREAD_ARCHIVE_H_*/

