#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "Notification.h"

#include "stdafx.h"
#include "AddInNative.h"

#include <string>
#include <clocale>
#include <thread>
#include <chrono>

#define TIME_LEN 34
#define ePropLast 0 // !!! Количество свойств !!!
#define eMethLast 1 // !!! Количество методов !!!

#define eMethPlayNotification 0

static ma_decoder* g_decoder = NULL;
static ma_device* g_device = NULL;
static bool g_isAudioInitialized = false;

// Callback функция для miniaudio
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    if (!pDevice || !pDevice->pUserData || !pOutput) {
        return;
    }

    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

    (void)pInput;
}

static wchar_t *g_MethodNames[] = {L"PlayNotification" };
static wchar_t *g_MethodNamesRu[] = {L"ВоспроизвестиУведомление"};

static const wchar_t g_kClassNames[] = L"AddInNative";
static IAddInDefBase *pAsyncEvent = NULL;

uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len = 0);
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len = 0);
uint32_t getLenShortWcharStr(const WCHAR_T* Source);

//---------------------------------------------------------------------------//
long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface)
{
    if(!*pInterface)
    {
        *pInterface= new CAddInNative;
        return (long)*pInterface;
    }
    return 0;
}
//---------------------------------------------------------------------------//
AppCapabilities SetPlatformCapabilities(const AppCapabilities capabilities)
{
    return eAppCapabilities1;
}
//---------------------------------------------------------------------------//
AttachType GetAttachType()
{
    return eCanAttachAny;
}
//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pIntf)
{
   if(!*pIntf)
      return -1;

   delete *pIntf;
   *pIntf = 0;
   return 0;
}
//---------------------------------------------------------------------------//
const WCHAR_T* GetClassNames()
{
    static WCHAR_T* names = 0;
    if (!names)
        ::convToShortWchar(&names, g_kClassNames);
    return names;
}

// CAddInNative
//---------------------------------------------------------------------------//
CAddInNative::CAddInNative()
{
    m_iMemory = 0;
    m_iConnect = 0;
}
//---------------------------------------------------------------------------//
CAddInNative::~CAddInNative()
{
    if (g_isAudioInitialized) {
        if (g_device) {
            ma_device_stop(g_device);
            ma_device_uninit(g_device);
            delete g_device;
            g_device = NULL;
        }
        if (g_decoder) {
            ma_decoder_uninit(g_decoder);
            delete g_decoder;
            g_decoder = NULL;
        }
        g_isAudioInitialized = false;
    }
}
//---------------------------------------------------------------------------//
bool CAddInNative::Init(void* pConnection)
{ 
    m_iConnect = (IAddInDefBase*)pConnection;
    return m_iConnect != NULL;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetInfo()
{ 
    // Component should put supported component technology version 
    // This component supports 2.0 version
    return 2000; 
}
//---------------------------------------------------------------------------//
void CAddInNative::Done()
{
}
/////////////////////////////////////////////////////////////////////////////
// ILanguageExtenderBase
//---------------------------------------------------------------------------//
bool CAddInNative::RegisterExtensionAs(WCHAR_T** wsExtensionName)
{ 
    wchar_t *wsExtension = L"AddNotification";
    int iActualSize = ::wcslen(wsExtension) + 1;
    WCHAR_T* dest = 0;

    if (m_iMemory)
    {
        if(m_iMemory->AllocMemory((void**)wsExtensionName, iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(wsExtensionName, wsExtension, iActualSize);
        return true;
    }

    return false; 
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNProps()
{ 
    // You may delete next lines and add your own implementation code here
    return ePropLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindProp(const WCHAR_T* wsPropName)
{ 
    long plPropNum = -1;
    return plPropNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetPropName(long lPropNum, long lPropAlias)
{ 
    return NULL;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{ 
    return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::SetPropVal(const long lPropNum, tVariant *varPropVal)
{ 
    return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropReadable(const long lPropNum)
{ 
    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropWritable(const long lPropNum)
{
    return false;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNMethods()
{ 
    return eMethLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindMethod(const WCHAR_T* wsMethodName)
{ 
    long plMethodNum = -1;
    wchar_t* name = 0;

    ::convFromShortWchar(&name, wsMethodName);

    plMethodNum = findName(g_MethodNames, name, eMethLast);

    if (plMethodNum == -1)
        plMethodNum = findName(g_MethodNamesRu, name, eMethLast);

    return plMethodNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetMethodName(const long lMethodNum, const long lMethodAlias)
{ 
    if (lMethodNum >= eMethLast)
        return NULL;

    wchar_t *wsCurrentName = NULL;
    WCHAR_T *wsMethodName = NULL;
    int iActualSize = 0;

    switch(lMethodAlias)
    {
    case 0: // First language
        wsCurrentName = g_MethodNames[lMethodNum];
        break;
    case 1: // Second language
        wsCurrentName = g_MethodNamesRu[lMethodNum];
        break;
    default: 
        return 0;
    }

    iActualSize = wcslen(wsCurrentName)+1;

    if (m_iMemory && wsCurrentName)
    {
        if(m_iMemory->AllocMemory((void**)&wsMethodName, iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(&wsMethodName, wsCurrentName, iActualSize);
    }

    return wsMethodName;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNParams(const long lMethodNum)
{ 
    switch(lMethodNum)
    { 
    case eMethPlayNotification:
        return 0;
	default:
        return 0;
    }
    
    return 0;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetParamDefValue(const long lMethodNum, const long lParamNum,
                          tVariant *pvarParamDefValue)
{ 
    TV_VT(pvarParamDefValue)= VTYPE_EMPTY;

    switch(lMethodNum)
    {
    case eMethPlayNotification:
        return false;
	default:
        return false;
    }

    return false;
} 
//---------------------------------------------------------------------------//
bool CAddInNative::HasRetVal(const long lMethodNum)
{ 
    switch(lMethodNum)
    { 
    case eMethPlayNotification:
        return false;
	default:
        return false;
    }
}
//---------------------------------------------------------------------------//
void ADDIN_API CAddInNative::SetLocale(const WCHAR_T* loc)
{
#ifndef __linux__
    _wsetlocale(LC_ALL, (const wchar_t*)loc);
#else
    //We convert in char* char_locale
    //also we establish locale
    //setlocale(LC_ALL, char_locale);
#endif
}

void ADDIN_API CAddInNative::SetUserInterfaceLanguageCode(const WCHAR_T* lang)
{
    lang = u"ru";
}

//---------------------------------------------------------------------------//
bool CAddInNative::CallAsProc(const long lMethodNum, tVariant* paParams, const long lSizeArray)
{
    switch (lMethodNum) {
    case eMethPlayNotification:
    {
        // 1. Проверка MP3
        if (!notification_mp3 || notification_mp3_len == 0) {
            MessageBoxA(GetDesktopWindow(), "MP3 данные отсутствуют!", "ERROR", MB_OK);
            return false;
        }

        // 2. УПРОЩЕННАЯ miniaudio БЕЗ new/delete/memset
        ma_decoder decoder;
        ma_device device;

        // Инициализация декодера
        ma_result result = ma_decoder_init_memory(notification_mp3, notification_mp3_len, NULL, &decoder);
        if (result != MA_SUCCESS) {
            MessageBoxA(GetDesktopWindow(), "Decoder FAIL", "MINIAUDIO", MB_OK);
            return false;
        }

        // Настройка устройства
        ma_device_config config = ma_device_config_init(ma_device_type_playback);
        config.playback.format = decoder.outputFormat;
        config.playback.channels = decoder.outputChannels;
        config.sampleRate = decoder.outputSampleRate;
        config.dataCallback = data_callback;
        config.pUserData = &decoder;

        // Инициализация устройства
        result = ma_device_init(NULL, &config, &device);
        if (result != MA_SUCCESS) {
            ma_decoder_uninit(&decoder);
            MessageBoxA(GetDesktopWindow(), "Device init FAIL", "MINIAUDIO", MB_OK);
            return false;
        }

        // Запуск
        result = ma_device_start(&device);
        if (result != MA_SUCCESS) {
            ma_device_uninit(&device);
            ma_decoder_uninit(&decoder);
            MessageBoxA(GetDesktopWindow(), "Device start FAIL", "MINIAUDIO", MB_OK);
            return false;
        }

        // Ждем 4 секунды (должно хватить)
        Sleep(4000);

        // Очистка
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);

        MessageBoxA(GetDesktopWindow(), "Звук завершен!", "SUCCESS", MB_OK);
        return true;
    }
    default:
        return false;
    }
}


//---------------------------------------------------------------------------//
bool CAddInNative::CallAsFunc(const long lMethodNum,
                tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{ 
    bool ret = false;
    FILE *file = 0;
    char *name = 0;
    int size = 0;
    char *mbstr = 0;
    wchar_t* wsTmp = 0;
	switch(lMethodNum)
	{ 
	default:
		return false;
	}
    return ret; 
}

/////////////////////////////////////////////////////////////////////////////
// LocaleBase
//---------------------------------------------------------------------------//
bool CAddInNative::setMemManager(void* mem)
{
    m_iMemory = (IMemoryManager*)mem;
    return m_iMemory != 0;
}
//---------------------------------------------------------------------------//
void CAddInNative::addError(uint32_t wcode, const wchar_t* source, 
                        const wchar_t* descriptor, long code)
{
    if (m_iConnect)
    {
        WCHAR_T *err = 0;
        WCHAR_T *descr = 0;
        
        ::convToShortWchar(&err, source);
        ::convToShortWchar(&descr, descriptor);

        m_iConnect->AddError(wcode, err, descr, code);
        delete[] err;
        delete[] descr;
    }
}
//---------------------------------------------------------------------------//
long CAddInNative::findName(wchar_t* names[], const wchar_t* name, 
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
uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len)
{
    if (!len)
        len = ::wcslen(Source)+1;

    if (!*Dest)
        *Dest = new WCHAR_T[len];

    WCHAR_T* tmpShort = *Dest;
    wchar_t* tmpWChar = (wchar_t*) Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len*sizeof(WCHAR_T));
    do
    {
        *tmpShort++ = (WCHAR_T)*tmpWChar++;
        ++res;
    }
    while (len-- && *tmpWChar);

    return res;
}
//---------------------------------------------------------------------------//
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len)
{
    if (!len)
        len = getLenShortWcharStr(Source)+1;

    if (!*Dest)
        *Dest = new wchar_t[len];

    wchar_t* tmpWChar = *Dest;
    WCHAR_T* tmpShort = (WCHAR_T*)Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len*sizeof(wchar_t));
    do
    {
        *tmpWChar++ = (wchar_t)*tmpShort++;
        ++res;
    }
    while (len-- && *tmpShort);

    return res;
}
//---------------------------------------------------------------------------//
uint32_t getLenShortWcharStr(const WCHAR_T* Source)
{
    uint32_t res = 0;
    WCHAR_T *tmpShort = (WCHAR_T*)Source;

    while (*tmpShort++)
        ++res;

    return res;
}
//---------------------------------------------------------------------------//
