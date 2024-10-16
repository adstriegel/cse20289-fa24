/*
 * fmnc_support.h
 *
 *  Created on: Apr 15, 2014
 *      Author: striegel
 */

#ifndef FMNC_SUPPORT_H_
#define FMNC_SUPPORT_H_

#define FMNC_MODULE_ENABLED		1

#include <string>
using namespace std;

#include "../pkt/IOModule.h"
//class IOModule;

// The support header file and source are function linkages available that
// allow for minimal linkages from the core of the ScaleBox framework
// (i.e. include one file and call specific functions)

/** Attempt to resolve an I/O module name from the specified string. The
 * resulting module will then be returned for future processing. The name
 * field refers to the unique class-level type of name (InputLog, InputPurge),
 * not the actual name that might be used to idenify one particular instance
 * of a given type (IL0, IL1, IL-WANSide).
 * @param	sName		The name of the module (class-wise name)
 * @returns A pointer to a valid IOModule object if successfully resolved,
 *          NULL otherwise
 */
IOModule * 		mapModuleFromName_FMNC (string sName);


#endif /* FMNC_SUPPORT_H_ */
