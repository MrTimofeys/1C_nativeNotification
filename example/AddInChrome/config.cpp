#include "config.h"
#include "addinlib.h"

#ifdef COM_TYPE
AddInComponentType typeComponent = eAddInCom;
#else
AddInComponentType typeComponent = eAddInNative;
#endif
const char *nameFilePrj = ADDNCHROME OS ARCH EXESUFFIX;
#ifndef COMP_EXTERNAL
const char *nameFileComponent = LIBPREFIX ADDNNATIVE OS ARCH ARCHSUFFIX "_" ADDNVER LIBSUFFIX;
#else
const char *nameFileComponent = ADDNNATIVE;
#endif
