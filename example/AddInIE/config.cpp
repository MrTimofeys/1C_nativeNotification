#include "stdafx.h"
#include "config.h"
#include "AddInIE_i.h"

const IID LIBID_AddInWebLib = LIBID_AddInIELib;

LPCOLESTR sAppId = _T(ADDNIE) _T(ARCH);
//---------------------------------------------------------------------------//
LPCOLESTR sAddInServiceEx = _T(ADDNIE) _T(ARCH) L".AddInServiceEx";
//---------------------------------------------------------------------------//
LPCOLESTR sAddInSite = _T(ADDNIE) _T(ARCH) L".AddInSite";
//---------------------------------------------------------------------------//
LPCOLESTR sAdapterCOM = _T(ADDNIE) _T(ARCH) L".AdapterCOM";
//---------------------------------------------------------------------------//
LPCOLESTR sAdapterNative = _T(ADDNIE) _T(ARCH) L".AdapterNative";
//---------------------------------------------------------------------------//
const wchar_t *nameFilePrj = _T(ADDNIE) _T(ARCH) _T(LIBSUFFIX);
#ifndef COMP_EXTERNAL
const wchar_t *nameFileComponent = _T(ADDNNATIVE) _T(OS) _T(ARCH) L"_" _T(ADDNVER) _T(LIBSUFFIX);
#else
const wchar_t *nameFileComponent = _T(ADDNNATIVE);
#endif
#ifdef COM_TYPE
const AddInComponentType typeComponent = eAddInCom;
#else
const AddInComponentType typeComponent = eAddInNative;
#endif
