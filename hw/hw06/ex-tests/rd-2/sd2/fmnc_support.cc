/*
 * fmnc_support.cc
 *
 *  Created on: Apr 15, 2014
 *      Author: striegel
 */



#include "fmnc_support.h"
#include "fmnc_manager.h"

//#include "TWiCE_Gateway.h"

IOModule * 		mapModuleFromName_FMNC (string sName) {
	IOModule * pModule;

	pModule = NULL;

	if(sName == "FMNC-Server") {
		pModule = new FMNC_Manager();
	}

	return pModule;
}

