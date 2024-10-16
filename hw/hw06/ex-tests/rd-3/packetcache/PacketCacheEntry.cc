#include <stdlib.h>

#include "PacketCacheEntry.h"

PacketCacheEntry::PacketCacheEntry () {

  m_nToken = 0;
  m_nHits = 0;		
  m_nCollisions = COLLISION_THRESHOLD;	
  m_nUpdates = 0;	
  

}

PacketCacheEntry::~PacketCacheEntry () {
}

