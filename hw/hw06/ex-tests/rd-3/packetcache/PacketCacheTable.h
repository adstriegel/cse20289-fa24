
#ifndef PACKETCACHETABLE_H_
#define PACKETCACHETABLE_H_

#include <deque>
using namespace std;

#include "../pkt/Packet.h"
#include "Defs.h"
#include "PacketCacheEntry.h"

#define DEBUG

/** The base class for the packet cache table that simply serves
 * up the requested data as appropriate 
 */
class PacketCacheTable {
	public:
		PacketCacheTable ();
		~PacketCacheTable ();

		int computeHash(Packet * pPkt);
		
		/** Look into the packet cache table to see if the data in the packet exists
		 * in the table. 
		 * @param 
		 * @returns    
		 */
		//token_t		hashMatch (Packet * pkt, int update);
		//bool		hashMatch (Packet * pkt, int update);
		fp_msg_t * fpHashMatch(Packet * pkt, int update);

		/** Get the number of entries currently contained within the packet
		 * cache
		 * @returns A valid unsigned long that is the raw number of entries (unique
		 *  hashes stored) in the table
		 */
		unsigned long	getNumEntries ();

		
		/** Get the total size consumed by data in the cache (excluding the data structures)
		 * @returns The number of bytes consumed by packet data and signatures
		 */
		unsigned long	getCurrentCacheSize (); 
		
                /** Get the number of cache replacement.
                */
                unsigned long	getNumUpdated ();

		fp_list_t * fp_tree;	/* the fp linked list */
		fp_list_t * fp_free;	/* empty nodes for the fp linked list */
		fp_list_t * fp_first;	/* the first in the array (list) of nodes */
		fp_t * pcT;				/* precompute Table */

	private:

		/* count of the number of tokens created */
		unsigned long m_nCount;				
                
                             /* count of cache replacement */
                             unsigned long m_nUpdated;

		/* protect the cache table */
		pthread_mutex_t m_MutexCacheTable;	

		/* The cache table itselt, an array of PacketCacheEntry.*/
		PacketCacheEntry  m_CacheTable[CACHETBL_LEN];

		deque<PacketCacheEntry *> m_EntryQueue;	   /* FIFO queue of entries */

};


/* remove node from an entry's list of own fingerprints */
void pinch_off_node(PacketCacheEntry * entry, fp_list_t * node);

/* remove node from tree of nodes */
void chop_from_tree(PacketCacheTable * tbl, fp_list_t * node);

/* move_around_tree is lightweight -- it doesn't expect duplicate fps */
void move_around_tree(fp_list_t * root, fp_list_t * node);
/* add_to_tree can handle duplicate fps, or adding new nodes */
int add_to_tree(PacketCacheTable * tbl,
				fp_list_t * root,
				PacketCacheEntry * entry,
				fp_t * fp,
				int off,
				fp_match_t * match,
				fp_list_t ** newent);

void expand_match(fp_match_t * match,
				PacketCacheEntry * entry1,
				int off1,
				PacketCacheEntry * entry2,
				int off2);


void tmp_ceit2(fp_list_t * node, PacketCacheEntry * entry);
void tmp_check_entry_in_tree(fp_list_t * node, PacketCacheEntry * entry);
void tmp_cts2(fp_list_t * node);
void tmp_check_tree_state(fp_list_t * node);

/* not actually ever used, but handy for debuggin'... */
int nodes_in_tree(fp_list_t * root);
#endif /*PACKETCACHETABLE_H_*/
