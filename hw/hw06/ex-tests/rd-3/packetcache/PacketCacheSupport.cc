#include "PacketCacheSupport.h"

#include "PacketCacheModule.h"

IOModule * 		mapModuleFromName_PacketCache (string sName) {
	IOModule * pModule;
	
	pModule = NULL;

	if(sName == "PacketCacheModule") {
		pModule = new PacketCacheModule();
	} 
	
	return pModule; 	
}
