// AddInCOM.h : Declaration of the CAddInCOM

#pragma once
#include "resource.h"       // main symbols
#include <atlstr.h>
#include "AddIn_i.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

enum AddInErrors {
                  ADDIN_E_NONE = 1000,
                  ADDIN_E_ORDINARY = 1001,
                  ADDIN_E_ATTENTION = 1002,
                  ADDIN_E_IMPORTANT = 1003,
                  ADDIN_E_VERY_IMPORTANT = 1004,
                  ADDIN_E_INFO = 1005,
                  ADDIN_E_FAIL = 1006,
                  ADDIN_E_MSGBOX_ATTENTION = 1007,
                  ADDIN_E_MSGBOX_INFO = 1008,
                  ADDIN_E_MSGBOX_FAIL = 1009
                 };
///////////////////////////////////////////////////////////////////////////////
// class CAddInCOM
class ATL_NO_VTABLE CAddInCOM :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CAddInCOM, &CLSID_AddInCOM>,
    public IInitDone,
    public ILanguageExtender,
    public ILocale,
    public IUserInterfaceLanguage,
    public IAddInCOM
{
public:
    enum 
    {
        ePropIsEnabled = 0,
        ePropIsTimerPresent,
        ePropLocale,
        ePropLast      // Always last
    };

    enum 
    {
        eMethEnable = 0,
        eMethDisable,
        eMethShowInStatusLine,
        eMethStartTimer,
        eMethStopTimer,
        eMethOpenForm,
        eMethShowMsgBox,
        eMethLast      // Always last
    };
    CAddInCOM()
    {
        m_iConnect = NULL;
        m_iErrorLog = NULL;
        m_iProfile = NULL;
        m_iStatusLine = NULL;
        m_iAsyncEvent = NULL;
    }


DECLARE_REGISTRY_RESOURCEID(IDR_ADDINCOM)


BEGIN_COM_MAP(CAddInCOM)
    COM_INTERFACE_ENTRY(IAddInCOM)
    COM_INTERFACE_ENTRY(IInitDone)
    COM_INTERFACE_ENTRY(ILanguageExtender)
    COM_INTERFACE_ENTRY(ILocale)
    COM_INTERFACE_ENTRY(IUserInterfaceLanguage)
END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        m_hTimerQueue = 0;
        return S_OK;
    }

    void FinalRelease()
    {
    }

// IInitDone
public:
    STDMETHOD(Init)(IDispatch *pConnection);
    STDMETHOD(Done)();
    STDMETHOD(GetInfo)(SAFEARRAY **pInfo);

// ILanguageExtender
public:

    STDMETHOD(RegisterExtensionAs)(BSTR *bstrExtensionName);

    STDMETHOD(GetNProps)(long *plProps);
    STDMETHOD(FindProp)(BSTR bstrPropName,long *plPropNum);
    STDMETHOD(GetPropName)(long lPropNum,long lPropAlias,BSTR *pbstrPropName);
    STDMETHOD(GetPropVal)(long lPropNum,VARIANT *pvarPropVal);
    STDMETHOD(SetPropVal)(long lPropNum,VARIANT *pvarPropVal);
    STDMETHOD(IsPropReadable)(long lPropNum,BOOL *pboolPropRead);
    STDMETHOD(IsPropWritable)(long lPropNum,BOOL *pboolPropWrite);

    STDMETHOD(GetNMethods)(long *plMethods);
    STDMETHOD(FindMethod)(BSTR bstrMethodName,long *plMethodNum);
    STDMETHOD(GetMethodName)(long lMethodNum,long lMethodAlias,BSTR *pbstrMethodName);
    STDMETHOD(GetNParams)(long lMethodNum,long *plParams);
    STDMETHOD(GetParamDefValue)(long lMethodNum,long lParamNum,VARIANT *pvarParamDefValue);
    STDMETHOD(HasRetVal)(long lMethodNum,BOOL *pboolRetValue);
    STDMETHOD(CallAsProc)(long lMethodNum,SAFEARRAY **paParams);
    STDMETHOD(CallAsFunc)(long lMethodNum,VARIANT *pvarRetValue,SAFEARRAY **paParams);
// ILocale
public:
    STDMETHOD(SetLocale)(BSTR bstrLocale);
//IUserInterfaceLanguage
public:
    STDMETHOD(SetUserInterfaceLanguageCode)(BSTR bstrLangCode);
    
private:
    IDispatch* m_iConnect;
    IErrorLog *m_iErrorLog;
    IPropertyProfile *m_iProfile;
    IStatusLine *m_iStatusLine;
    IAsyncEvent *m_iAsyncEvent;
    BOOL m_boolEnabled;
    HANDLE              m_hTimer;
    HANDLE              m_hTimerQueue;

    BOOL LoadProperties();
    void SaveProperties();
    CAtlStringW TermString(UINT uiResID,long nAlias);
    VARIANT GetNParam(SAFEARRAY *pArray,long lIndex);
    CAtlStringW m_locale, m_langCode;
};

OBJECT_ENTRY_AUTO(__uuidof(AddInCOM), CAddInCOM)
