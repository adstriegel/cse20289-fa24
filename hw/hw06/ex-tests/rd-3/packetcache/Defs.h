/* defs.h
 * a few handy definitions */

#ifndef DEFS_H
#define DEFS_H

#define PARTIAL_HEAD

/* protocol-related constants */
#define MAX_PACKET		1520

/* number of items in the hash table.  must be a power of 2. */
#define CACHETBL_LEN		(1<<13)
//#define CACHETBL_LEN		(1<<14)
/* maximum value of a hash. must one less than a power of 2.
 * (also, if this is bigger than 1<<16, you'll have to change some stuff */
#define HASH_MAX		((1<<16) - 1)

#define COLLISION_THRESHOLD	5

typedef u_int32_t token_t;

#define PARTIAL_TOKEN_SIZE (IP_TOTLEN_SIZE + IP_CHECKSUM_SIZE+ TCP_SEQ_SIZE + TCP_ACK_SIZE + TCP_WIN_SIZE + TCP_CHECKSUM_SIZE + sizeof(token_t))


//For Rabin fingerprint: 
/* this is (2^gamma)-1 in the Wetherall paper.
 * they tried odd values from 3 to 11, and found 5 to work best.
 * still, you should play with this a little. */
#define FINGERPRINT_SELECTION	((1ULL<<5) - 1ULL)

/* this is Beta from the Wetherall paper.
 * they tried various sizes from 0 to 500, and selected (1<<6) = 64 
 * as delivering the best performance */
#define FP_WINDOW_SIZE		(1<<6)

/* a prime number.  from the Wetherall paper.*/
#define RABIN_PRIME			1048583


/* roughly, how many interesting fingerprints we can expect per packet */
#define FINGERPRINTS_PER_PACKET		((MAX_PACKET-FP_WINDOW_SIZE) \
									 / FINGERPRINT_SELECTION)

#define FP_CACHE_LEN					CACHETBL_LEN * (FINGERPRINTS_PER_PACKET+1)

/* how many shortcuts into the fp_list we make. Bigger = faster = more memory */
#define FP_INDEX_LEN				(1<<10)
/* FP_SHIFT * FP_INDEX_LEN = 1<<64 */
#define FP_SHIFT					(1<<54)

#endif /*DEFS_H*/
