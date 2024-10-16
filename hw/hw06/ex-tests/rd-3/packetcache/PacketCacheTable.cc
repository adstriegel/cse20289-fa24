#include <stdlib.h>
#include <iostream.h>

#include "PacketCacheTable.h"
#include "Defs.h"
#include "../pkt/PktSigMD5.h"
#include "../mem/MemoryPool.h"
#include "../util/ip-utils.h"

PacketCacheTable::PacketCacheTable () {
  int i;
  fp_t k, pb;

  m_nCount = 0;
  m_nUpdated = 0;
  for (i = 0; i < CACHETBL_LEN; i++) {
    m_CacheTable[i].m_nToken=i;
    m_CacheTable[i].m_Packet.setLength(0);
  }

  fp_free  = new fp_list_t[FP_CACHE_LEN];
  pcT = new fp_t[256];

  fp_first = fp_free;

  /* put in a "dummy" root node */
  fp_tree = &fp_free[FP_CACHE_LEN-1];
  fp_tree->smaller = NULL;
  fp_tree->bigger = NULL;
  fp_tree->fp = 2;	/* never matches a regular fp */
  fp_tree->entry = NULL;
  
	
  /* link up the list */
  fp_free[FP_CACHE_LEN-2].bigger = NULL;
  for (i = 0; i < FP_CACHE_LEN-1; ++i) {
    fp_free[i].bigger = &fp_free[i+1];
  }

  pb = 1;
  /* compute pb := p^Beta */
  for (i = 0; i < 64; ++i)
    pb = (pb * RABIN_PRIME);
  
  /* compute pcT[k] := k * pb, for 0 leq k < 256 */
  for (k = 0; k < 256; ++k)
    pcT[k] = k * pb;
  
}

PacketCacheTable::~PacketCacheTable () {
	
}

int PacketCacheTable::computeHash(Packet * pPkt) {
  char * data;
  int len;
  md5_val md5Val;
  int hashVal;

  //Rather than use the raw data starting from ethernet address, we compute the hash on real data (in TCP)
  //For simplicity, I didn't check if this packet is a IP & TCP packet
  data = pPkt->getData()+(OFFSET_IPV4_DATA); 
  len = pPkt->getLength()-(OFFSET_IPV4_DATA);
  
  //If this packet is too small, return. (Not sure if returning 0 is appropriate)
  if (len<=0) return 0;

  PktSigMD5	*		pSig;
  
  // Try to get a packet signature from the memory pool
  pSig = (PktSigMD5 *) g_MemPool.getObject(MEMPOOL_OBJ_PKTSIGMD5);
		
  if(pSig == NULL) {
    cerr << "Error: Could not allocate a new MD5 signature, aborting signature generation" << endl;
    return 0;
  }
		
  pSig->calcMD5((unsigned char *)data, len);
  md5Val = pSig->getMD5();
  //cout << "MD5 Sig: " <<  pSig << endl;

  pSig->clean();
  g_MemPool.releaseObject(pSig);

  hashVal=((md5Val.value[0] << 8) + md5Val.value[1]) & HASH_MAX;
//  cout << "Hash Val: " <<hashVal  << " for pkt " << pPkt << "   md5Val.value0: " <<md5Val.value[0] << "   md5Val.value1: " << md5Val.value[1] << "   len:" <<len<<endl;
  return hashVal;
  
}

 

fp_msg_t * PacketCacheTable::fpHashMatch(Packet * pkt, int update){
              char * data;
              u_int32_t len;

	/* MAX_PACKET/2 is grossly huge compared to the actual expected number
	 * of fingerprints from any one packet ... but let's not worry about it */
	//Xiaolong's code use lots of static variables, why?  Just to speed up?
	fp_t fp[MAX_PACKET/2];	
	fp_t fpt;
	int off[MAX_PACKET/2];	
	fp_match_t match[MAX_PACKET/2];	
	int nfp=1, nmatches=0, pos;
	PacketCacheEntry * entry, *old_entry;
	fp_list_t * newnode[MAX_PACKET/2];
	fp_msg_t packet;
	int moff, m, i, j;
	int src_off, dest_off, cp_len;

        unsigned int mbytes; /* matched bytes of this packet. */

	//Rather than use the raw data starting from ethernet address, we compute the hash on real data (in TCP)
	//For simplicity, I didn't check if this packet is a IP & TCP packet
	data = pkt->getData()+(OFFSET_IPV4_DATA); 
	len = pkt->getLength()-(OFFSET_IPV4_DATA);

	// if this packet is too small (eg its size is less than OFFSET_IPV4_DATA), ignore it. 
	if(len<=0) return NULL;
	

	fp[0] = 0;
	off[0] = 0;

	/* compute the initial hash over the first 64 bytes */
	fpt = 0;
	for (i = 0; i < 64; ++i)
		fpt = fpt * RABIN_PRIME + data[i];


	/* the first hash is always stored */
	fp[0] = fpt;

	for (pos = 64; pos < len; ++pos) {
		fpt = fpt * RABIN_PRIME - pcT[data[pos-64]] + data[pos];
		if (1 == (fpt & FINGERPRINT_SELECTION)
				&& fpt != fp[nfp-1]) {
			fp[nfp] = fpt;
			off[nfp] = pos - 64 + 1;
			++nfp;
		}
	}
	/* okay, so now fp is an array of interesting fingerprints, length nfp. */
	/* if m_EntryQueue's size is less than CACHETBL_LEN, create a new entry ann add it into the tail of m_EntryQueue; otherwise pop up the first entry from m_EntryQueue, update it, then add it to the tail of the FIFO queue, m_EntryQueue. */


	pthread_mutex_lock(&m_MutexCacheTable);
	if(m_EntryQueue.size() ==CACHETBL_LEN) {
	  old_entry = m_EntryQueue[0];
	  m_EntryQueue.pop_front();
	  /* if we're replacing another entry (the first entry in the fifo queue), let's kill it. */
	  while (old_entry->fp_first) {
	    chop_from_tree(this, old_entry->fp_first);
	    pinch_off_node(old_entry, old_entry->fp_first);
	  }
	  old_entry->m_nCollisions = 0;
	  tmp_check_tree_state(fp_tree);
	  entry=old_entry;
	} else {
	  entry=new PacketCacheEntry();
	}

	/* entry->m_nUpdates is not used. How about entry->m_nCollisions? */
	entry->m_nToken = m_nCount ++; /* do I need to do 'mod CACHETBL_LEN' here? */
	entry->m_nCollisions = 0;
	entry->m_nHits = 0;
	entry->m_Packet.setLength(len);
	entry->m_Packet.setData(len, data);
	/* we're going to push the entry into the tail of the FIFO queue */	
	m_EntryQueue.push_back(entry);

#ifdef DEBUG
	if (memcmp(entry->m_Packet.getData(), data, len) != 0)
		printf("Error: whoa, buffer is screwy");
	//	print entry???
#endif

	/* let's insert each fingerprint.  if it finds a match
	 * in the process, it'll be recorded into match[]. */
	for (i = 0; i < nfp; ++i) {
	  if (add_to_tree(this,
			  fp_tree,
			  entry,
			  &fp[i],
			  off[i],
			  &match[nmatches],
			  &newnode[i])) {
	    //printf("match.off: %d ,.len: %d\n", match[nmatches].off1, match[nmatches].len);
	    //fflush(stdout);
	    ++nmatches;
	  }

	  if (i) {
#ifdef DEBUG
	    if (newnode[i] == newnode[i-1])
	      printf("Error: something's wrong");
#endif
	    
	    newnode[i]->next_same_entry = NULL; /* prob. temporary? */
	    newnode[i-1]->next_same_entry = newnode[i];
	  } else {
	    newnode[0]->next_same_entry = NULL; /* prob. temporary? */
	    entry->fp_first = newnode[0];
            //cout << "here? " << m_EntryQueue.size() << " " << i << " " << nfp<<endl;

	  }
#ifdef DEBUG
	  tmp_check_tree_state(fp_tree);
#endif
	}
	newnode[nfp-1]->next_same_entry = NULL;
	
	pthread_mutex_unlock(&m_MutexCacheTable);
	//printf("0: %p->%p\n", newnode, newnode[0]);
#ifdef DEBUG
	if (!entry->fp_first)
	  printf("Error: \n\n\t\t\t>> fuck.<<");
#endif

	packet.num = 0;

	/* okay, now we build a new packet, using the found matches 
	 * to insert tokens */
	if (nmatches) {
		/* XXX: and this... is slow.  what's a better way to find and
		 * sort the union of several subsets of a packet? */
		moff = -1;	/* next match offset */
		m = 0;		/* the first match is always a good one */
                mbytes=0;      /* total matched bytes in match[]. This variable is used to compute redundancy. */
		/* idea: step through the packet with moff, finding the next
		 * good match from match[] */
		do {
			/* this match is unique and good, so copy it into our packet */
			packet.token[packet.num].id = match[m].entry->m_nToken + 1;
			packet.token[packet.num].off1 = match[m].off1;
			packet.token[packet.num].off2 = match[m].off2;
			packet.token[packet.num].len = match[m].len;
			++packet.num;
			moff = match[m].len + match[m].off1;
                        mbytes += match[m].len;
			match[m].off1 = MAX_PACKET; /* don't match again */

                        //printf("X: moff: %d len: %d packet.num: %d nmatches %d\n", moff, len, packet.num, nmatches);
			for (j = 0; j < nmatches; ++j) {
			  /* find the next match (that is as long as possible) */
			  if ((match[j].off1 < match[m].off1
			       || (match[j].off1 == match[m].off1
				   && match[j].len > match[m].len))
			      && match[j].off1 > moff) {
			    m = j;
			  }
			}

		} while (match[m].off1 < MAX_PACKET && packet.num <= nmatches);

                //printf("matches: %d, p.num: %d, mbytes: %d \n", nmatches, packet.num, mbytes);
                packet.mbytes=mbytes;
//for (i=0;i<packet.num;++i)
//printf("match[%d].len: %d ,", i, match[i].len);

		/* now copy in the data */
		src_off = 0;
		/* grr, the fp_msg_t structure is declared with token[1], so we gotta
		 * move ahead sizeof(token) * (ntokens-1) */
		dest_off = sizeof(msg_token_t) * (packet.num-1);
		packet.len = dest_off + sizeof(msg_token_t) 
		  + offsetof(fp_msg_t, token);
		packet.len += cp_len = packet.token[0].off1;
		//printf("%d:0:%d; ", dest_off, cp_len);
		memcpy(packet.data+dest_off, data+src_off, cp_len);
		
		/* copy in each segment that isn't matched by something else */
		for (i = 0; i < packet.num; ++i) {
		  src_off += cp_len + packet.token[i].len;
		  dest_off += cp_len;
		  cp_len = ((i < packet.num - 1) ? packet.token[i+1].off1 : len)
		    - packet.token[i].off1 - packet.token[i].len;
		  //printf("%d:%d:%d; ", dest_off, src_off, cp_len);
		  packet.len += cp_len;
		  
		  memcpy(packet.data+dest_off, data+src_off, cp_len);
		}
		//printf(")\n");
		//printf(">> packet: (%d):{%100.100s}\n", packet.len, packet.data);
	}

	
	return &packet;
}

unsigned long PacketCacheTable::getNumEntries () {
	return 0;
}
		
unsigned long	PacketCacheTable::getCurrentCacheSize () {
	return 0;	
}

unsigned long	PacketCacheTable::getNumUpdated () {
        return m_nUpdated;
}




/* return: pointer to (possible) match, NULL if no possible match */
int add_to_tree(PacketCacheTable * tbl,
				fp_list_t * root,
				PacketCacheEntry * entry,
				fp_t * fp,
				int off,
				fp_match_t * match,
				fp_list_t ** newent)
{
	int match_okay = 1;
	fp_list_t * node = tbl->fp_free;
	fp_list_t ** branch;
	//printf("NIT: %d  ", nodes_in_tree(root));

	/* okay, we have a fingerprint match */
	if (root->fp == *fp) {

		/* haha, so the bug was some (redundancy-filled) packets
		 * were matching themselves... which could actually work,
		 * I suppose, but the client would have to be redesigned a lot */
		if (root->entry == entry)
			match_okay = 0;

		expand_match(match, entry, off, root->entry, root->off);
		pinch_off_node(root->entry, root);

//printf("THIS MATCH IS %d bytes, on %p and %p\n",
		//match->len,
		//root->entry,
		//entry);
//fflush(stdout);

		root->entry = entry;
		root->off = off;
		*newent = root;
//if (match->off1 == -1)
	//printf("collision.\n");
//else
	//printf("match!\n");
	//tmp_check_tree_state(node);

		return match->off1 != -1 && match_okay;
	}

	if (root->fp < *fp)
		branch = &root->smaller;
	else
		branch = &root->bigger;

	/* okay, so here's where we add the new node */
	if (!*branch) {
#ifdef DEBUG
		if (!node->bigger)
			printf("Error: There aren't enough nodes to hold all the fingerprints!");
#endif
		tbl->fp_free = node->bigger;
		//printf("linked %p (fp %lu) as child of %p (%lu).\n", node, *fp, root, root->fp);
		*branch = node;
		node->parent = root;
		node->smaller = node->bigger = NULL;
		node->fp = *fp;
		node->entry = entry;
		node->off = off;
		*newent = node;
		//printf("NIT: %d  ", nodes_in_tree(root));
//printf("2: %p->%p\n", newent, *newent);
//printf("x");
//fflush(stdout);
	//tmp_check_tree_state(node);
		return 0;
	}
//printf(" \\> branched down...\n");
	return add_to_tree(tbl, *branch, entry, fp, off, match, newent);
}

/* remove node from entry's list of fingerprints */
void pinch_off_node(PacketCacheEntry * entry, fp_list_t * node)
{
	fp_list_t * i;

#ifdef DEBUG
	if (!entry || !entry->fp_first || !node)
		printf("Error: pointer screwup");
#endif

	/* node was the first one, so we nail it and go home. */
	if (entry->fp_first == node) {
		entry->fp_first = node->next_same_entry;
#ifdef DEBUG
		node->entry = NULL;
		if (!entry->fp_first) {
			tmp_check_entry_in_tree(node, entry);
		}
		//tmp_check_tree_state(node);
#endif
		return;
	}

	/* step through the list until we find node */
	for (i = entry->fp_first;
			i->next_same_entry != node;
			i = i->next_same_entry)
#ifdef DEBUG
		if (!i->next_same_entry)
			printf("Error: well, i'm trying to pinch off a node that isn't in the chain.");
		if (!entry->fp_first) {
			tmp_check_entry_in_tree(i, entry);
		}
		//tmp_check_tree_state(node);
#endif
		/* it's all in the loop */ ;

	/* this is the pinch. */
	i->next_same_entry = node->next_same_entry;
#ifdef DEBUG
	node->entry = NULL;
	if (!entry->fp_first) {
		tmp_check_entry_in_tree(i, entry);
	}
	//tmp_check_tree_state(node);
#endif
}

void chop_from_tree(PacketCacheTable * tbl, fp_list_t * node)
{
#ifdef DEBUG
	if (!node || !node->parent)
		printf("Error: pointers again");
	if (node->parent->smaller != node && node->parent->bigger != node)
		printf("Error: deadbeat parent disowns own child!");
#endif

	/* unlink node from tree */
	if (node->parent->smaller == node)
		node->parent->smaller = NULL;
	else
		node->parent->bigger = NULL;

	/* relink children */
	move_around_tree(node->parent, node->smaller);
	move_around_tree(node->parent, node->bigger);

	/* toss node onto the free nodes list */
	node->bigger = tbl->fp_free;
	tbl->fp_free = node;

#ifdef DEBUG
	tmp_check_tree_state(node);
#endif
}

void move_around_tree(fp_list_t * root, fp_list_t * node)
{
	fp_list_t ** branch;

#ifdef DEBUG
	if (!root)
		printf("Error: jeez, you screwed this one up");
#endif

	/* nonexistant nodes are really easy to add to a tree */
	if (!node)
		return;

	if (node->fp < root->fp)
		branch = &root->smaller;
	else
		branch = &root->bigger;

	if (!*branch) {
		*branch = node;
		node->parent = root;
		return;
	}

#ifdef DEBUG
	tmp_check_tree_state(node);
#endif
	/* let's hope for some tail-recursion */
	move_around_tree(*branch, node);
}

/* old, linear, slow-as-a-bitch version */
void expand_match(fp_match_t * match,
				PacketCacheEntry * entry1,
				int off1,
				PacketCacheEntry * entry2,
				int off2)
{
	int end1 = off1 + FP_WINDOW_SIZE-1;
	int end2 = off2 + FP_WINDOW_SIZE-1;

	if (memcmp(entry1->m_Packet.getData() + off1,
				entry2->m_Packet.getData() + off2,
				FP_WINDOW_SIZE) != 0) {
		/* hmm, maybe doesn't match after all */
		match->off1 = -1;
//printf("col (o1: %d, o2: %d)", off1, off2); fflush(stdout);
		return;
	}


//printf("going back (o1: %d, o2: %d)", off1, off2); fflush(stdout);
	/* let's push back the front */
	do {
//printf(",");
		--off1;	
		--off2;
	} while (off1 >= 0 && off2 >= 0 &&
		entry1->m_Packet.getData()[off1] == entry2->m_Packet.getData()[off2]);

//printf(" forward");
	/* and push forward the end */
	do {
//printf(".");
		++end1;	
		++end2;
	} while (end1 <= entry1->m_Packet.getLength() && end2 <= entry2->m_Packet.getLength() &&
		entry1->m_Packet.getData()[end1] == entry2->m_Packet.getData()[end2]);

	match->entry = entry2;
	match->off1 = off1 + 1;
	match->off2 = off2 + 1;
	match->len = end1-off1-2;
#ifdef DEBUG
/* okay, this is basic sanity */
	if (memcmp(entry1->m_Packet.getData() + match->off1,
			entry2->m_Packet.getData() + match->off2,
			match->len) != 0)
		printf("Error: wtf? the matching algorithm is screwed");

	if (match->off1>0 && match->off2>0 &&
			memcmp(entry1->m_Packet.getData() + match->off1 - 1,
					entry2->m_Packet.getData() + match->off2 - 1,
					match->len) == 0)
		printf("Error: Be more greedy!");

	if (match->len != end2 - off2 - 2)
		printf("Error: oh this sucks");
#endif

//printf(" - (o1: %d, o2: %d)", off1, off2); fflush(stdout);
//printf(" len: %d\n", match->len);
}


void tmp_ceit2(fp_list_t * node, PacketCacheEntry * entry)
{
	if (node->smaller)
		tmp_ceit2(node->smaller, entry);
	if (node->bigger)
		tmp_ceit2(node->bigger, entry);
	if (node->entry == entry)
		printf("Error: !!!! XXXXX entry is in tree XXXXX !!!!");
}

void tmp_check_entry_in_tree(fp_list_t * node, PacketCacheEntry * entry)
{
#ifdef EXTRA_DEBUG
	fp_list_t * root;
	root = node;
	while (root->parent)
		root = root->parent;
	tmp_ceit2(root, entry);
#endif
}

void tmp_cts2(fp_list_t * node)
{
	fp_list_t * next = node->next_same_entry;

	if (next) {
		if (node->entry != next->entry)
			printf("Error: list is broken");
	}

	if (node->smaller) {
		if (node->smaller->parent != node)
			printf("Error: tree is broken");
		tmp_cts2(node->smaller);
	}
	if (node->bigger) {
		if (node->bigger->parent != node)
			printf("Error: tree is broken");
		tmp_cts2(node->bigger);
	}
}
void tmp_check_tree_state(fp_list_t * node)
{
#ifdef EXTRA_DEBUG
	fp_list_t * root;
	root = node;
	while (root->parent)
		root = root->parent;
	tmp_cts2(root);
#endif
}

/* not actually ever used, but handy for debuggin'... */
int nodes_in_tree(fp_list_t * root)
{
	return 1 + (root->smaller ? nodes_in_tree(root->smaller) : 0) +
		(root->bigger ? nodes_in_tree(root->bigger) : 0);
}
