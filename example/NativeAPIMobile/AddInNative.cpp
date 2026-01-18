
#include "stdafx.h"
#include "AddInNative.h"
#include "ConversionWchar.h"
#include <stdio.h>

#if defined(__ANDROID__) || defined(__APPLE__)

#include <wchar.h>

#endif //__ANDROID__ || __APPLE__

#if defined(__APPLE__) && !defined(BUILD_DYNAMIC_LIBRARY)

namespace COM_1C_STEP_COUNTER
{

#endif //__APPLE__ && !BUILD_DYNAMIC_LIBRARY
    
static const wchar_t *g_PropNames[] =
   {L"IsEnabled"};

static const wchar_t *g_MethodNames[] =
   {L"Enable",
    L"Disable",
    L"GetStepCount",
    L"GetMovementType",
    L"GetOrientation",
    L"ShowOrientation",
    L"GetDescriptionLastError"};

static const wchar_t *g_PropNamesRu[] =
   {L"Включен"};

static const wchar_t *g_MethodNamesRu[] =
    {L"Включить",
     L"Выключить",
     L"ПолучитьКоличествоШагов",
     L"ПолучитьТипДвижения",
     L"ПолучитьУгловоеПоложениеУстройства",
     L"ПоказатьУгловоеПоложениеУстройства",
     L"ПолучитьОписаниеПоследнейОшибки"};

static const wchar_t g_ComponentNameType[] = L"com_1c_StepCounterExtension";
    
#if defined(__APPLE__) && !defined(BUILD_DYNAMIC_LIBRARY)

static const char g_ComponentName[] = "com_1c_StepCounter";

#endif //__APPLE__ && !BUILD_DYNAMIC_LIBRARY

// This component supports 2.1 version
const long g_VersionAddIn = 2100;

static AppCapabilities g_capabilities = eAppCapabilitiesInvalid;

//---------------------------------------------------------------------------//
long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface)
{
    if(!*pInterface)
    {
        *pInterface = new StepCounter;
        return (long)*pInterface;
    }
    return 0;
}
    
//---------------------------------------------------------------------------//
AppCapabilities SetPlatformCapabilities(const AppCapabilities capabilities)
{
    g_capabilities = capabilities;
    return eAppCapabilitiesLast;
}
    
//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pInterface)
{
    if(!*pInterface)
        return -1;

    delete *pInterface;
    *pInterface = 0;
    return 0;
}
    
//---------------------------------------------------------------------------//
const WCHAR_T* GetClassNames()
{
    return s_kClassNames;
}
    
//---------------------------------------------------------------------------//
StepCounter::StepCounter() : m_iConnect(0), m_iMemory(0), isEnabled(false)
{
}
    
//---------------------------------------------------------------------------//
StepCounter::~StepCounter()
{
}
    
/////////////////////////////////////////////////////////////////////////////
// IInitDoneBase
//---------------------------------------------------------------------------//
bool StepCounter::Init(void* pConnection)
{
    m_iConnect = (IAddInDefBaseEx*)pConnection;
    if (m_iConnect)
    {
        stepCounter.setIConnect(m_iConnect);
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------//
bool StepCounter::setMemManager(void* mem)
{
    m_iMemory = (IMemoryManager*)mem;
    return m_iMemory != 0;
}
    
//---------------------------------------------------------------------------//
long StepCounter::GetInfo()
{ 
    // Component should put supported component technology version
    return g_VersionAddIn;
}
    
//---------------------------------------------------------------------------//
void StepCounter::Done()
{
    m_iConnect = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// ILanguageExtenderBase
//---------------------------------------------------------------------------//
bool StepCounter::RegisterExtensionAs(WCHAR_T** wsExtensionName)
{ 
    const wchar_t *wsExtension = g_ComponentNameType;
    uint32_t iActualSize = static_cast<uint32_t>(::wcslen(wsExtension) + 1);

    if (m_iMemory)
    {
        if(m_iMemory->AllocMemory((void**)wsExtensionName, iActualSize * sizeof(WCHAR_T)))
        {
            convToShortWchar(wsExtensionName, wsExtension, iActualSize);
            return true;
        }
    }

    return false; 
}
    
//---------------------------------------------------------------------------//
long StepCounter::GetNProps()
{ 
    // You may delete next lines and add your own implementation code here
    return ePropLast;
}
    
//---------------------------------------------------------------------------//
long StepCounter::FindProp(const WCHAR_T* wsPropName)
{ 
    long plPropNum = -1;
    wchar_t* propName = 0;
    convFromShortWchar(&propName, wsPropName);
    
    plPropNum = findName(g_PropNames, propName, ePropLast);

    if (plPropNum == -1)
        plPropNum = findName(g_PropNamesRu, propName, ePropLast);

    delete[] propName;

    return plPropNum;
}
    
//---------------------------------------------------------------------------//
const WCHAR_T* StepCounter::GetPropName(long lPropNum, long lPropAlias)
{ 
    if (lPropNum >= ePropLast)
        return NULL;

    wchar_t *wsCurrentName = NULL;
    WCHAR_T *wsPropName = NULL;

    switch(lPropAlias)
    {
    case 0: // First language (english)
        wsCurrentName = (wchar_t*)g_PropNames[lPropNum];
        break;
    case 1: // Second language (local)
        wsCurrentName = (wchar_t*)g_PropNamesRu[lPropNum];
        break;
    default:
        return 0;
    }
    
    uint32_t iActualSize = static_cast<uint32_t>(wcslen(wsCurrentName) + 1);

    if (m_iMemory && wsCurrentName)
    {
        if (m_iMemory->AllocMemory((void**)&wsPropName, iActualSize * sizeof(WCHAR_T)))
            convToShortWchar(&wsPropName, wsCurrentName, iActualSize);
    }

    return wsPropName;
}
    
//---------------------------------------------------------------------------//
bool StepCounter::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{ 
    switch(lPropNum)
    {
        case ePropIsEnabled:
        {
            TV_VT(pvarPropVal) = VTYPE_BOOL;
            TV_BOOL(pvarPropVal) = isEnabled;
            break;
        }
        default:
            return false;
    }
    return true;
}
    
//---------------------------------------------------------------------------//
bool StepCounter::SetPropVal(const long lPropNum, tVariant *varPropVal)
{ 
    return true;
}
    
//---------------------------------------------------------------------------//
bool StepCounter::IsPropReadable(const long lPropNum)
{ 
    switch(lPropNum)
    { 
        case ePropIsEnabled:
            return true;
    }
    return false;
}
    
//---------------------------------------------------------------------------//
bool StepCounter::IsPropWritable(const long lPropNum)
{
    return false;
}

//---------------------------------------------------------------------------//
long StepCounter::GetNMethods()
{ 
    return eMethLast;
}
    
//---------------------------------------------------------------------------//
long StepCounter::FindMethod(const WCHAR_T* wsMethodName)
{ 
    long plMethodNum = -1;
    wchar_t* name = 0;
    convFromShortWchar(&name, wsMethodName);

    plMethodNum = findName(g_MethodNames, name, eMethLast);

    if (plMethodNum == -1)
        plMethodNum = findName(g_MethodNamesRu, name, eMethLast);

    delete[] name;

    return plMethodNum;
}
    
//---------------------------------------------------------------------------//
const WCHAR_T* StepCounter::GetMethodName(const long lMethodNum, const long lMethodAlias)
{ 
    if (lMethodNum >= eMethLast)
        return NULL;

    wchar_t *wsCurrentName = NULL;
    WCHAR_T *wsMethodName = NULL;

    switch(lMethodAlias)
    {
    case 0: // First language (english)
        wsCurrentName = (wchar_t*)g_MethodNames[lMethodNum];
        break;
    case 1: // Second language (local)
        wsCurrentName = (wchar_t*)g_MethodNamesRu[lMethodNum];
        break;
    default: 
        return 0;
    }

    uint32_t iActualSize = static_cast<uint32_t>(wcslen(wsCurrentName) + 1);

    if (m_iMemory && wsCurrentName)
    {
        if(m_iMemory->AllocMemory((void**)&wsMethodName, iActualSize * sizeof(WCHAR_T)))
            convToShortWchar(&wsMethodName, wsCurrentName, iActualSize);
    }

    return wsMethodName;
}
    
//---------------------------------------------------------------------------//
long StepCounter::GetNParams(const long lMethodNum)
{ 
    switch (lMethodNum)
    { 
        case eMethGetOrientation:
            return 1;
    }
    return 0;
}
    
//---------------------------------------------------------------------------//
bool StepCounter::GetParamDefValue(const long lMethodNum, const long lParamNum,
                        tVariant *pvarParamDefValue)
{ 
    // There are no parameter values by default
    TV_VT(pvarParamDefValue) = VTYPE_EMPTY;

    switch (lMethodNum)
    {
        case eMethEnable:
        case eMethDisable:
        case eMethGetStepCount:
        case eMethGetMovementType:
        case eMethShowOrientation:
        case eMethGetDescriptionLastError:
        {
            // No parameters
            return false;
        }
        case eMethGetOrientation:
        {
            // Only first parameter
            if (lParamNum == 0)
                return true;
        }
    }
    return false;
}
    
//---------------------------------------------------------------------------//
bool StepCounter::HasRetVal(const long lMethodNum)
{
    switch (lMethodNum)
    {
        case eMethGetStepCount:
        case eMethGetMovementType:
        case eMethGetOrientation:
        case eMethGetDescriptionLastError:
            return true;
            break;            
        default:
            return false;
    }
    return false;
}
    
//---------------------------------------------------------------------------//
bool StepCounter::CallAsProc(const long lMethodNum,
                    tVariant* paParams, const long lSizeArray)
{
    switch(lMethodNum)
    {
        case eMethEnable:
        {
            stepCounter.startStepCounterUpdatesFromDate();
            isEnabled = true;
            break;
        }
        case eMethDisable:
        {
            stepCounter.stopStepCounterUpdates();
            isEnabled = false;
            break;
        }
        case eMethShowOrientation:
        {
            if(eAppCapabilities1 <= g_capabilities)
            {
                IAddInDefBaseEx* cnn = (IAddInDefBaseEx*)m_iConnect;
                IMsgBox* imsgbox = (IMsgBox*)cnn->GetInterface(eIMsgBox);
                if (imsgbox)
                {
                    const uint16_t MAX_RESULT_STRING_LENGTH = 40;
                    wchar_t buf[MAX_RESULT_STRING_LENGTH];
                    memset(buf, 0, sizeof(wchar_t) * MAX_RESULT_STRING_LENGTH);
                    
                    swprintf(buf, MAX_RESULT_STRING_LENGTH - 1,
                             L"XOZ:%5.3f XOY:%5.3f YOZ:%5.3f",
                             stepCounter.orientation(eOrientXOZ),
                             stepCounter.orientation(eOrientXOY),
                             stepCounter.orientation(eOrientYOZ));
                
                    WCHAR_T *msg = 0;
                    uint32_t iActualSize = static_cast<uint32_t>(wcslen(buf) + 1);
                    convToShortWchar(&msg, buf, iActualSize);
                    imsgbox->Alert(msg);
                    delete [] msg;
                }
            }
            break;
        }
    default:
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------//
bool StepCounter::CallAsFunc(const long lMethodNum,
                tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    switch(lMethodNum)
    {
        case eMethGetStepCount:
        case eMethGetMovementType:
        {
            // No parameters
            if (lSizeArray || paParams)
                return false;

            TV_VT(pvarRetValue) = VTYPE_I4;
            if (lMethodNum == eMethGetStepCount)
                TV_I4(pvarRetValue) = static_cast<int32_t>(stepCounter.stepCount());
            else
                TV_I4(pvarRetValue) = static_cast<int32_t>(stepCounter.typeMotion());
            return true;
            break;
        }
        case eMethGetOrientation:
        {
            // One parameter
            if ((lSizeArray != 1) || !paParams)
                return false;
 
            switch (paParams->lVal)
            {
                case 0:  // XOY
                case 1:  // XOZ
                case 2:  // YOZ
                {
                    TV_VT(pvarRetValue) = VTYPE_R8;
                    TV_R8(pvarRetValue) = stepCounter.orientation(static_cast<Orientations>(paParams->lVal));
                    return true;
                    break;
                }
            }
            break;
        }
        case eMethGetDescriptionLastError:
        {
            // No parameters
            if (lSizeArray || paParams)
                return false;
            
            std::wstring errDescr = stepCounter.getLastErrorDescr();
            uint32_t iActualSize = static_cast<uint32_t>(errDescr.size() + 1);
            if (m_iMemory->AllocMemory((void**)&pvarRetValue->pwstrVal, iActualSize * sizeof(WCHAR_T)))
            {
                convToShortWchar(&pvarRetValue->pwstrVal, errDescr.c_str(), iActualSize);
                pvarRetValue->wstrLen = iActualSize - 1;
                TV_VT(pvarRetValue) = VTYPE_PWSTR;
                return true;
            }
            break;
        }
        default:
            return false;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////
// ILocaleBase
//---------------------------------------------------------------------------//
void StepCounter::SetLocale(const WCHAR_T* loc)
{
}
//---------------------------------------------------------------------------//
void StepCounter::SetUserInterfaceLanguageCode(const WCHAR_T * lang)
{
}

/////////////////////////////////////////////////////////////////////////////
// Other

//---------------------------------------------------------------------------//
void StepCounter::addError(uint32_t wcode, const wchar_t* source,
                        const wchar_t* descriptor, long code)
{
    if (m_iConnect)
    {
        WCHAR_T *err = 0;
        WCHAR_T *descr = 0;
        
        convToShortWchar(&err, source);
        convToShortWchar(&descr, descriptor);

        m_iConnect->AddError(wcode, err, descr, code);

        delete[] descr;
        delete[] err;
    }
}
    
//---------------------------------------------------------------------------//
bool StepCounter::sendMessageFromEvent(const wchar_t* textMsg)
{
    if (m_iConnect)
    {
        m_iConnect->ExternalEvent(s_classNameStepCounter, s_eventChangeName, s_paramEvent);
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------//
long StepCounter::findName(const wchar_t* names[], const wchar_t* name,
                        const uint32_t size) const
{
    long ret = -1;
    for (uint32_t i = 0; i < size; i++)
    {
        if (!wcscmp(names[i], name))
        {
            ret = i;
            break;
        }
    }
    return ret;
}

//---------------------------------------------------------------------------//
#if defined(__APPLE__) && !defined(BUILD_DYNAMIC_LIBRARY)

};

namespace COM_1C_STEP_COUNTER
{

    static LPCVOID addin_exports[] =
    {
        "GetClassObject", (LPCVOID)GetClassObject,
        "DestroyObject", (LPCVOID)DestroyObject,
        "GetClassNames", (LPCVOID)GetClassNames,
        "SetPlatformCapabilities", (LPCVOID)SetPlatformCapabilities,
        NULL
    };

    DECLARE_DLL((const char*)g_ComponentName, addin_exports);
}

#endif //__APPLE__ && !BUILD_DYNAMIC_LIBRARY
