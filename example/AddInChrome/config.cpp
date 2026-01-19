#include "config.h"
#include "addinlib.h"

#ifdef COM_TYPE
AddInComponentType typeComponent = eAddInCom;
#else
AddInComponentType typeComponent = eAddInNative;
#endif
const char *nameFilePrj = "AddInChrWin32.exe";
#ifndef COMP_EXTERNAL
const char *nameFileComponent = "AddInNative.dll";
#else
const char *nameFileComponent = "ADDNNATIVE";
#endif
