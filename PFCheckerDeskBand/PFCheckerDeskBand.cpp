// CalendarDeskBand.cpp : Implementation of CCalendarDeskBand

#include "StdAfx.h"
#include "PFCheckerDeskBand.h"
#include "DateFormatSettings.h"
#include "Utils.h"

////////////////////////////////////////////////////////////////////////////////
// 
const UINT IDM_SEPARATOR_OFFSET = 0;
const UINT IDM_SETTINGS_OFFSET = 1;

const LPCTSTR SETTINGS_SECTION_DATEFORMAT = TEXT("DateFormatDB");

////////////////////////////////////////////////////////////////////////////////
// CCalendarDeskBand

CCalendarDeskBand::CCalendarDeskBand()
{
}

////////////////////////////////////////////////////////////////////////////////
// 
HRESULT CCalendarDeskBand::FinalConstruct()
{
    m_nBandID = 0;
    m_dwViewMode = DBIF_VIEWMODE_NORMAL;
    m_bRequiresSave = false;
    m_bCompositionEnabled = TRUE;
    ::MakeSettingsFilename(m_settingsPath);

    if(m_settingsPath.FileExists())
        m_dateFormat.Load(m_settingsPath, SETTINGS_SECTION_DATEFORMAT);

    return HandleShowRequest();
}

void CCalendarDeskBand::FinalRelease()
{
    if(m_settingsPath.FileExists())
        m_dateFormat.Save(m_settingsPath, SETTINGS_SECTION_DATEFORMAT);
}

HRESULT CCalendarDeskBand::HandleShowRequest()
{
    OLECHAR szAtom[MAX_GUID_STRING_LEN] = { 0 };
    ::StringFromGUID2(CLSID_CalendarDeskBand, szAtom, MAX_GUID_STRING_LEN);

    HRESULT hr = S_OK;
    const ATOM show = ::GlobalFindAtomW(szAtom);

    if(show)
    {
        CComPtr<IBandSite> spBandSite;
        hr = spBandSite.CoCreateInstance(CLSID_TrayBandSiteService);

        if(SUCCEEDED(hr))
        {
            LPUNKNOWN lpUnk = static_cast<IOleWindow*>(this);
            hr = spBandSite->AddBand(lpUnk);
        }

        ::GlobalDeleteAtom(show);
    }

    return hr;
}

HRESULT CCalendarDeskBand::UpdateDeskband()
{
    CComPtr<IInputObjectSite> spInputSite;
    HRESULT hr = GetSite(IID_IInputObjectSite,
        reinterpret_cast<void**>(&spInputSite));

    if(SUCCEEDED(hr))
    {
        CComQIPtr<IOleCommandTarget> spOleCmdTarget = spInputSite;

        if(spOleCmdTarget)
        {
            // m_nBandID must be `int' or bandID variant must be explicitly
            // set to VT_I4, otherwise IDeskBand::GetBandInfo won't
            // be called by the system.
            CComVariant bandID(m_nBandID);

            hr = spOleCmdTarget->Exec(&CGID_DeskBand,
                DBID_BANDINFOCHANGED, OLECMDEXECOPT_DODEFAULT, &bandID, NULL);
            ATLASSERT(SUCCEEDED(hr));

            if(SUCCEEDED(hr))
                m_wndCalendar.UpdateCaption();
        }
    }

    return hr;
}

////////////////////////////////////////////////////////////////////////////////
// IObjectWithSite

HRESULT STDMETHODCALLTYPE CCalendarDeskBand::SetSite( 
    /* [in] */ IUnknown *pUnkSite)
{
    HRESULT hr = __super::SetSite(pUnkSite);

    if(SUCCEEDED(hr) && pUnkSite) // pUnkSite is NULL when band is being destroyed
    {
        CComQIPtr<IOleWindow> spOleWindow = pUnkSite;

        if(spOleWindow)
        {
            HWND hwndParent = NULL;
            hr = spOleWindow->GetWindow(&hwndParent);

            if(SUCCEEDED(hr))
            {
                m_wndCalendar.Create(hwndParent,
                    static_cast<IDeskBand*>(this), pUnkSite, &m_dateFormat);

                if(!m_wndCalendar.IsWindow())
                    hr = E_FAIL;
            }
        }
    }

    return hr;
}

////////////////////////////////////////////////////////////////////////////////
// IInputObject

HRESULT STDMETHODCALLTYPE CCalendarDeskBand::UIActivateIO(
    /* [in] */ BOOL fActivate,
    /* [unique][in] */ PMSG /*pMsg*/)
{
    ATLTRACE(atlTraceCOM, 2, _T("IInputObject::UIActivateIO (%s)\n"),
        (fActivate ? _T("TRUE") : _T("FALSE")));

    if(fActivate)
        m_wndCalendar.SetFocus();

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CCalendarDeskBand::HasFocusIO()
{
    ATLTRACE(atlTraceCOM, 2, _T("IInputObject::HasFocusIO\n"));

    return (m_wndCalendar.HasFocus() ? S_OK : S_FALSE);
}

HRESULT STDMETHODCALLTYPE CCalendarDeskBand::TranslateAcceleratorIO(
    /* [in] */ PMSG /*pMsg*/)
{
    ATLTRACE(atlTraceCOM, 2, _T("IInputObject::TranslateAcceleratorIO\n"));

    return S_FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// IContextMenu

HRESULT STDMETHODCALLTYPE CCalendarDeskBand::QueryContextMenu(
    /* [in] */ HMENU hMenu,
    /* [in] */ UINT indexMenu,
    /* [in] */ UINT idCmdFirst,
    /* [in] */ UINT /*idCmdLast*/,
    /* [in] */ UINT uFlags)
{
    ATLTRACE(atlTraceCOM, 2, _T("IContextMenu::QueryContextMenu\n"));

    if(CMF_DEFAULTONLY & uFlags)
        return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);

    // Add a seperator
    ::InsertMenu(hMenu, 
        indexMenu,
        MF_SEPARATOR | MF_BYPOSITION,
        idCmdFirst + IDM_SEPARATOR_OFFSET, 0);

    // Add the new menu item
    CString sCaption;
    sCaption.LoadString(IDS_DESKBANDSETTINGS);

    ::InsertMenu(hMenu, 
        indexMenu, 
        MF_STRING | MF_BYPOSITION,
        idCmdFirst + IDM_SETTINGS_OFFSET,
        sCaption);

    return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, IDM_SETTINGS_OFFSET + 1);
}

HRESULT STDMETHODCALLTYPE CCalendarDeskBand::InvokeCommand( 
    /* [in] */ LPCMINVOKECOMMANDINFO pici)
{
    ATLTRACE(atlTraceCOM, 2, _T("IContextMenu::InvokeCommand\n"));

    if(!pici) return E_INVALIDARG;

    if(LOWORD(pici->lpVerb) == IDM_SETTINGS_OFFSET)
    {
        ATLASSERT(m_wndCalendar.IsWindow());
        CDateFormatSettings dlgSettings;
        const INT_PTR res = dlgSettings.DoModal(m_wndCalendar,
            reinterpret_cast<LPARAM>(&m_dateFormat));

        if(res == IDOK)
        {
            m_dateFormat = dlgSettings.m_dateFormat;
            m_bRequiresSave = true;
            
            const HRESULT hr = UpdateDeskband();
            ATLASSERT(SUCCEEDED(hr));
        }
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CCalendarDeskBand::GetCommandString( 
    /* [in] */ UINT_PTR /*idCmd*/,
    /* [in] */ UINT /*uType*/,
    /* [in] */ UINT* /*pReserved*/,
    /* [out] */ LPSTR /*pszName*/,
    /* [in] */ UINT /*cchMax*/)
{
    ATLTRACE(atlTraceCOM, 2, _T("IContextMenu::GetCommandString\n"));

    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// IDeskBand

HRESULT STDMETHODCALLTYPE CCalendarDeskBand::GetBandInfo( 
    /* [in] */ DWORD dwBandID,
    /* [in] */ DWORD dwViewMode,
    /* [out][in] */ DESKBANDINFO *pdbi)
{
    ATLTRACE(atlTraceCOM, 2, _T("IDeskBand::GetBandInfo\n"));

    if(!pdbi) return E_INVALIDARG;

    m_nBandID = dwBandID;
    m_dwViewMode = dwViewMode;

    if(pdbi->dwMask & DBIM_MODEFLAGS)
    {
	    pdbi->dwModeFlags = DBIMF_VARIABLEHEIGHT;
    }

    if(pdbi->dwMask & DBIM_MINSIZE)
    {
        pdbi->ptMinSize = m_wndCalendar.CalcIdealSize();
    }

    if(pdbi->dwMask & DBIM_MAXSIZE)
    {
        // the band object has no limit for its maximum height
        pdbi->ptMaxSize.x = -1;
        pdbi->ptMaxSize.y = -1;
    }

    if(pdbi->dwMask & DBIM_INTEGRAL)
    {
        pdbi->ptIntegral.x = 1;
        pdbi->ptIntegral.y = 1;
    }

    if(pdbi->dwMask & DBIM_ACTUAL)
    {
        pdbi->ptActual = m_wndCalendar.CalcIdealSize();
    }

    if(pdbi->dwMask & DBIM_TITLE)
    {
        if(dwViewMode == DBIF_VIEWMODE_FLOATING)
        {
            CString sDate;
            sDate.LoadString(IDS_DATE);
            lstrcpynW(pdbi->wszTitle, sDate, ARRAYSIZE(pdbi->wszTitle));
        }
        else pdbi->dwMask &= ~DBIM_TITLE; // do not show title
    }

    if(pdbi->dwMask & DBIM_BKCOLOR)
	{
		//Use the default background color by removing this flag.
		pdbi->dwMask &= ~DBIM_BKCOLOR;
	}

    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// IDeskBand2
HRESULT STDMETHODCALLTYPE CCalendarDeskBand::CanRenderComposited( 
    /* [out] */ BOOL *pfCanRenderComposited)
{
    if(pfCanRenderComposited)
        *pfCanRenderComposited = TRUE;

    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE CCalendarDeskBand::SetCompositionState( 
    /* [in] */ BOOL fCompositionEnabled)
{
    m_bCompositionEnabled = fCompositionEnabled;
    m_wndCalendar.Invalidate();

    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE CCalendarDeskBand::GetCompositionState( 
    /* [out] */ BOOL *pfCompositionEnabled)
{
    if(pfCompositionEnabled)
        *pfCompositionEnabled = m_bCompositionEnabled;

    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// IOleWindow

HRESULT STDMETHODCALLTYPE CCalendarDeskBand::GetWindow( 
    /* [out] */ HWND *phwnd)
{
    ATLTRACE(atlTraceCOM, 2, _T("IOleWindow::GetWindow\n"));

    if(phwnd) *phwnd = m_wndCalendar;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CCalendarDeskBand::ContextSensitiveHelp( 
    /* [in] */ BOOL /*fEnterMode*/)
{
    ATLTRACE(atlTraceCOM, 2, _T("IOleWindow::ContextSensitiveHelp\n"));

    ATLTRACENOTIMPL("IOleWindow::ContextSensitiveHelp");
}

////////////////////////////////////////////////////////////////////////////////
// IDockingWindow

HRESULT STDMETHODCALLTYPE CCalendarDeskBand::ShowDW( 
    /* [in] */ BOOL fShow)
{
    ATLTRACE(atlTraceCOM, 2, _T("IDockingWindow::ShowDW\n"));

    if(m_wndCalendar)
        m_wndCalendar.ShowWindow(fShow ? SW_SHOW : SW_HIDE);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CCalendarDeskBand::CloseDW( 
    /* [in] */ DWORD /*dwReserved*/)
{
    ATLTRACE(atlTraceCOM, 2, _T("IDockingWindow::CloseDW\n"));

    if(m_wndCalendar)
    {
        m_wndCalendar.ShowWindow(SW_HIDE);
        m_wndCalendar.DestroyWindow();
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CCalendarDeskBand::ResizeBorderDW( 
    /* [in] */ LPCRECT prcBorder,
    /* [in] */ IUnknown *punkToolbarSite,
    /* [in] */ BOOL /*fReserved*/)
{
    ATLTRACE(atlTraceCOM, 2, _T("IDockingWindow::ResizeBorderDW\n"));

    if(!m_wndCalendar) return S_OK;

    CComQIPtr<IDockingWindowSite> spDockingWindowSite = punkToolbarSite;

    if(spDockingWindowSite)
    {
        BORDERWIDTHS bw = { 0 };
        bw.top = bw.bottom = ::GetSystemMetrics(SM_CYBORDER);
        bw.left = bw.right = ::GetSystemMetrics(SM_CXBORDER);

        HRESULT hr = spDockingWindowSite->RequestBorderSpaceDW(
            static_cast<IDeskBand*>(this), &bw);

        if(SUCCEEDED(hr))
        {
            HRESULT hr = spDockingWindowSite->SetBorderSpaceDW(
                static_cast<IDeskBand*>(this), &bw);

            if(SUCCEEDED(hr))
            {
                m_wndCalendar.MoveWindow(prcBorder);
                return S_OK;
            }
        }
    }

    return E_FAIL;
}

////////////////////////////////////////////////////////////////////////////////
// IPersistStreamImpl

HRESULT CCalendarDeskBand::IPersistStreamInit_Load(
    LPSTREAM pStm,
    const ATL_PROPMAP_ENTRY* pMap)
{
    if(m_bstrDateFormat)
        m_bstrDateFormat.Empty();

    const HRESULT hr = __super::IPersistStreamInit_Load(pStm, pMap);

    if(SUCCEEDED(hr))
        m_dateFormat.dateFormat = m_bstrDateFormat;

    return hr;
}

HRESULT CCalendarDeskBand::IPersistStreamInit_Save(
    LPSTREAM pStm,
    BOOL fClearDirty,
    const ATL_PROPMAP_ENTRY* pMap)
{
    m_bstrDateFormat = m_dateFormat.dateFormat;

    return __super::IPersistStreamInit_Save(pStm, fClearDirty, pMap);
}

////////////////////////////////////////////////////////////////////////////////