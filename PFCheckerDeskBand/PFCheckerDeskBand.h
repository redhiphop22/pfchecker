// CalendarDeskBand.h : Declaration of the CCalendarDeskBand

#pragma once
#include "resource.h"       // main symbols

#include "CalendarGuids.h"
#include "PFCheckerWindow.h"
#include "DateFormat.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;

// CCalendarDeskBand

class CCalendarDeskBand :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CCalendarDeskBand, &CLSID_CalendarDeskBand>,
    public IObjectWithSiteImpl<CCalendarDeskBand>,
    public IPersistStreamInitImpl<CCalendarDeskBand>,
    public IInputObject,
    public IContextMenu,
    public IDeskBand2
{
    typedef IPersistStreamInitImpl<CCalendarDeskBand> IPersistStreamImpl;
public:
    CCalendarDeskBand();

DECLARE_REGISTRY_RESOURCEID(IDR_CALENDARDESKBAND)

BEGIN_COM_MAP(CCalendarDeskBand)
    COM_INTERFACE_ENTRY(IOleWindow)
    COM_INTERFACE_ENTRY(IDockingWindow)
    COM_INTERFACE_ENTRY(IDeskBand)
    COM_INTERFACE_ENTRY(IDeskBand2)
    COM_INTERFACE_ENTRY(IInputObject)
    COM_INTERFACE_ENTRY(IContextMenu)
    COM_INTERFACE_ENTRY(IObjectWithSite)
    COM_INTERFACE_ENTRY_IID(IID_IPersist, IPersistStreamImpl)
    COM_INTERFACE_ENTRY_IID(IID_IPersistStream, IPersistStreamImpl)
    COM_INTERFACE_ENTRY_IID(IID_IPersistStreamInit, IPersistStreamImpl)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CCalendarDeskBand)
    IMPLEMENTED_CATEGORY(CATID_DeskBand)
END_CATEGORY_MAP()

// IPersistStreamInitImpl requires property map.
BEGIN_PROP_MAP(CCalendarDeskBand)
    PROP_DATA_ENTRY("Locale", m_dateFormat.lcId, VT_UI4)
    PROP_DATA_ENTRY("Calendar", m_dateFormat.calId, VT_UI4)
    PROP_DATA_ENTRY("CalendarType", m_dateFormat.calType, VT_UI4)
    PROP_DATA_ENTRY("DateFormat", m_bstrDateFormat.m_str, VT_BSTR)
END_PROP_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct();
    void FinalRelease();

public:
    // IObjectWithSite
    //
    STDMETHOD(SetSite)( 
        /* [in] */ IUnknown *pUnkSite);

    // IInputObject
    //
    STDMETHOD(UIActivateIO)( 
        /* [in] */ BOOL fActivate,
        /* [unique][in] */ MSG *pMsg);
        
    STDMETHOD(HasFocusIO)();

    STDMETHOD(TranslateAcceleratorIO)( 
        /* [in] */ MSG *pMsg);

    // IContextMenu
    //
    STDMETHOD(QueryContextMenu)(
        /* [in] */ HMENU hmenu,
        /* [in] */ UINT indexMenu,
        /* [in] */ UINT idCmdFirst,
        /* [in] */ UINT idCmdLast,
        /* [in] */ UINT uFlags);

    STDMETHOD(InvokeCommand)( 
        /* [in] */ CMINVOKECOMMANDINFO *pici);

    STDMETHOD(GetCommandString)( 
        /* [in] */ UINT_PTR idCmd,
        /* [in] */ UINT uType,
        /* [in] */ UINT *pReserved,
        /* [out] */ LPSTR pszName,
        /* [in] */ UINT cchMax);

    // IDeskBand
    //
    STDMETHOD(GetBandInfo)(
        /* [in] */ DWORD dwBandID,
        /* [in] */ DWORD dwViewMode,
        /* [out][in] */ DESKBANDINFO *pdbi);

    // IDeskBand2
    //
    STDMETHOD(CanRenderComposited)( 
        /* [out] */ BOOL *pfCanRenderComposited);
        
    STDMETHOD(SetCompositionState)( 
        /* [in] */ BOOL fCompositionEnabled);
        
    STDMETHOD(GetCompositionState)( 
        /* [out] */ BOOL *pfCompositionEnabled);
    
    // IOleWindow
    //
    STDMETHOD(GetWindow)( 
        /* [out] */ HWND *phwnd);

    STDMETHOD(ContextSensitiveHelp)( 
        /* [in] */ BOOL fEnterMode);

    // IDockingWindow
    //
    STDMETHOD(ShowDW)( 
        /* [in] */ BOOL fShow);

    STDMETHOD(CloseDW)( 
        /* [in] */ DWORD dwReserved);

    STDMETHOD(ResizeBorderDW)( 
        /* [in] */ LPCRECT prcBorder,
        /* [in] */ IUnknown *punkToolbarSite,
        /* [in] */ BOOL fReserved);

    // IPersistStreamImpl
    //
    HRESULT IPersistStreamInit_Load(
        /* [in] */ LPSTREAM pStm,
        /* [in] */ const ATL_PROPMAP_ENTRY* pMap);

    HRESULT IPersistStreamInit_Save(
        /* [in] */ LPSTREAM pStm,
        /* [in] */ BOOL fClearDirty,
        /* [in] */ const ATL_PROPMAP_ENTRY* pMap);

private:
    HRESULT HandleShowRequest();
    HRESULT UpdateDeskband();

public:
    bool m_bRequiresSave; // used by IPersistStreamInitImpl

private:
    int m_nBandID;
    DWORD m_dwViewMode;
    DateFormat m_dateFormat;
    CCalendarWindow m_wndCalendar;
    CComBSTR m_bstrDateFormat;  // for persistence only [used in property map]
    BOOL m_bCompositionEnabled;
    CPath m_settingsPath;
};

OBJECT_ENTRY_AUTO(CLSID_CalendarDeskBand, CCalendarDeskBand)
