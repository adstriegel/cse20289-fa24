
#ifndef PACKETCACHEENTRY_H_
#define PACKETCACHEENTRY_H_

#include "../pkt/Packet.h"
#include "Defs.h"

//The code is updated according to Xiaolong's PacketCache/pc_hash.h

typedef u_int64_t fp_t;
struct FP_LIST_T;

/** An entry in the packet cache table that pertains to a specific
 * signature
 */
class PacketCacheEntry {
	public:
		PacketCacheEntry ();
		~PacketCacheEntry ();

		// the token is 32 bits
		token_t m_nToken;
		/** number of times this entry was used */
		unsigned int m_nHits;		
		/** number of collisions on this entry */
		unsigned int m_nCollisions;	
		/** number of times this entry was updated */
		unsigned int m_nUpdates;	
		
		FP_LIST_T * fp_first;
		
		Packet m_Packet; 
		
             private:
		
		
};

/* node in a (triplely)-linked list of Rabin fingerprints */
typedef struct FP_LIST_T {
	fp_t fp;
	int off;
	PacketCacheEntry * entry;
	struct FP_LIST_T * smaller;
	struct FP_LIST_T * bigger;
	struct FP_LIST_T * parent;
	struct FP_LIST_T * next_same_entry;
} fp_list_t;

typedef struct FP_MATCH_T {
	int off1;
	int off2;
	int len;
	PacketCacheEntry  * entry;
} fp_match_t;

typedef struct MSG_TOKEN_T {
  u_int16_t id;		/* matching packet id */
  u_int16_t off1;		/* offset into this packet */
  u_int16_t off2;		/* offset into the cached packet */
  u_int16_t len;		/* length of match */
}  msg_token_t;

typedef struct FP_MSG_T {
	//u_int32_t code;			/* TEMPORARY! */
	u_int16_t len;			/* entire length of this packet */
	u_int16_t num;			/* number of tokens below */
        unsigned int mbytes;            /* bytes matched in the rabin fingerprint table */
        msg_token_t token[1];
        unsigned char data[MAX_PACKET];	/* the rest of the packet */
} fp_msg_t;


#endif /*PACKETCACHEENTRY_H_*/
