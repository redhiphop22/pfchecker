#include "StdAfx.h"
#include "CalendarWindow.h"
#include "CalendarGuids.h"
#include "Utils.h"

/////////////////////////////////////////////////////////////////////////////
//
const LPCTSTR DESKBAND_BIN_NAME = TEXT("PFChecker.dll");

////////////////////////////////////////////////////////////////////////////////
// 
CString StringFromGUID2(REFGUID rguid)
{
    OLECHAR szGuid[MAX_GUID_STRING_LEN] = { 0 };
    ::StringFromGUID2(rguid, szGuid, MAX_GUID_STRING_LEN);

    return szGuid;
}

DWORD RunAsAdmin(HWND hWnd, LPCTSTR lpFile, LPCTSTR lpParameters)
{
    SHELLEXECUTEINFO sei = { 0 };

    const ULONG fMask =
        SEE_MASK_FLAG_DDEWAIT |
        SEE_MASK_FLAG_NO_UI |
        SEE_MASK_NOCLOSEPROCESS;

    sei.cbSize          = sizeof(SHELLEXECUTEINFOW);
    sei.hwnd            = hWnd;
    sei.fMask           = fMask;
    sei.lpVerb          = TEXT("runas");
    sei.lpFile          = lpFile;
    sei.lpParameters    = lpParameters;
    sei.nShow           = SW_SHOWNORMAL;

    if(::ShellExecuteEx(&sei))
    {
        CHandle handle(sei.hProcess);
        const DWORD dwWaitRes = ::WaitForSingleObject(handle, INFINITE);

        if(dwWaitRes == WAIT_OBJECT_0)
        {
            DWORD dwExitCode = ERROR_SUCCESS;
            const BOOL bRes = ::GetExitCodeProcess(handle, &dwExitCode);
            ATLASSERT(bRes);

            return dwExitCode;
        }
    }

    return ::GetLastError();
}

//////////////////////////////////////////////////////////////////////////////
// static
UINT CCalendarWindow::s_uTaskbarRestart =
    ::RegisterWindowMessage(_T("TaskbarCreated"));

//////////////////////////////////////////////////////////////////////////////
//
CCalendarWindow::CCalendarWindow() :
    m_ctrlMonthCal(MONTHCAL_CLASS, this, MONTHCAL_MSG_MAP_ID),
    m_hSysTrayIcon(NULL),
    m_hWndIconSmall(NULL),
    m_hWndIcon(NULL)
{
}

LRESULT CCalendarWindow::OnCreate(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM lParam,
    BOOL& /*bHandled*/)
{
    LPCREATESTRUCT pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);

    if(!InitIcons())
        return -1;

    if(!CreateChildren(pcs))
        return -1;

    if(!InitMenu())
        return -1;

    if(!InitSettingsPath())
        return -1;

    m_dlgOptions.LoadSettings(m_pathSettings);

    if(!ApplySettings(/*Resize=*/true))
        return -1;

    if(!UpdateSysTrayIcon(NIM_ADD))
        return -1;

    if(SetUpdateTimer() == 0)
        return -1;

	m_UseSystemInfo.Init();

    return 0;
}

LRESULT CCalendarWindow::OnSize(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM lParam,
    BOOL& /*bHandled*/)
{
    m_ctrlMonthCal.MoveWindow(0, 0, LOWORD(lParam), HIWORD(lParam), FALSE);

    return 0;
}

LRESULT CCalendarWindow::OnSysCommand(
    UINT /*uMsg*/,
    WPARAM wParam,
    LPARAM /*lParam*/,
    BOOL& bHandled)
{
    switch(wParam)
    {
    case IDC_OPTIONS:
        ShowOptions();
        break;
    case IDC_ABOUT:
        ShowAboutBox();
        break;
    default:
        bHandled = FALSE;
    }

    return 0;
}

LRESULT CCalendarWindow::OnDestroy(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    KillTimer(UPDATE_TIMER_ID);

    if(m_hSysTrayIcon)
    {
        ATLVERIFY(UpdateSysTrayIcon(NIM_DELETE));
        ::DestroyIcon(m_hSysTrayIcon);
    }

    ATLASSERT(m_hWndIconSmall != NULL);
    ::DestroyIcon(m_hWndIconSmall);

    ATLASSERT(m_hWndIcon != NULL);
    ::DestroyIcon(m_hWndIcon);

    ::PostQuitMessage(0);

    return 0;
}

LRESULT CCalendarWindow::OnSysTrayNotify(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM lParam,
    BOOL& /*bHandled*/)
{
    switch(lParam)
    {
    case WM_LBUTTONUP:
        ShowAndActivate();
        break;
    case WM_RBUTTONUP:
        HandleContextCommand(ShowContextMenu());
        break;
    }

    return 0;
}

LRESULT CCalendarWindow::OnClose(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    ShowWindow(SW_HIDE);

    return 0;
}

LRESULT CCalendarWindow::OnTimeChange(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    SYSTEMTIME stNow = { 0 };
    ::GetLocalTime(&stNow);

    UpdateTodayDate(stNow);

    return 0;
}

LRESULT CCalendarWindow::OnTimer(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    SYSTEMTIME stNow = { 0 };
    ::GetLocalTime(&stNow);

    SYSTEMTIME stCtrlToday = { 0 };
    MonthCal_GetToday(m_ctrlMonthCal, &stCtrlToday);

    if(stNow.wDay != stCtrlToday.wDay)
        UpdateTodayDate(stNow);

	double cpu = m_UseSystemInfo.GetCpuUsage();
	double ram = m_UseSystemInfo.GetMemUsage();
    return 0;
}

LRESULT CCalendarWindow::OnEndSession(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    DestroyWindow();

    return TRUE; // system can continue to end session
}

LRESULT CCalendarWindow::OnPowerBroadcast(
    UINT /*uMsg*/,
    WPARAM wParam,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    if(wParam == PBT_APMRESUMECRITICAL ||
       wParam == PBT_APMRESUMESUSPEND ||
       wParam == PBT_APMRESUMESTANDBY)
    {
        // re-enable timer
        ATLVERIFY(SetUpdateTimer() != 0);

        // refresh tray icon (sometimes it is not refreshed after resume)
        ATLVERIFY(UpdateSysTrayIcon(NIM_MODIFY));
    }

    return 0;
}

LRESULT CCalendarWindow::OnSetFocus(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    m_ctrlMonthCal.SetFocus();

    return 0;
}

LRESULT CCalendarWindow::OnTaskbarRestart(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    return (UpdateSysTrayIcon(NIM_ADD) ? 0L : -1L);
}

bool CCalendarWindow::InitMenu(void)
{
    HMENU hSysMenu = GetSystemMenu(FALSE);
    ATLVERIFY(::AppendMenu(hSysMenu, MF_SEPARATOR, 0, NULL));

    CString strMenuText;
    ATLVERIFY(strMenuText.LoadString(IDS_OPTIONS));

    BOOL bAppend = ::AppendMenu(hSysMenu, MF_STRING,
        IDC_OPTIONS, strMenuText);
    ATLASSERT(bAppend);

    ATLVERIFY(strMenuText.LoadString(IDS_ABOUT));

    bAppend = ::AppendMenu(hSysMenu, MF_STRING,
        IDC_ABOUT, strMenuText);

    return (bAppend != FALSE);
}

bool CCalendarWindow::InitIcons()
{
    m_hWndIconSmall = (HICON)::LoadImage(
        _AtlBaseModule.GetResourceInstance(),
        MAKEINTRESOURCE(IDI_CALENDAR),
        IMAGE_ICON,
        ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR);
    ATLASSERT(m_hWndIconSmall != NULL);

    SetIcon(m_hWndIconSmall, /*bBigIcon=*/FALSE);

    m_hWndIcon = (HICON)::LoadImage(
        _AtlBaseModule.GetResourceInstance(),
        MAKEINTRESOURCE(IDI_CALENDAR),
        IMAGE_ICON,
        ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON),
        LR_DEFAULTCOLOR);
    ATLASSERT(m_hWndIcon != NULL);

    SetIcon(m_hWndIcon, /*bBigIcon=*/TRUE);

    return (m_hWndIconSmall && m_hWndIcon);
}

bool CCalendarWindow::CreateChildren(LPCREATESTRUCT /*pcs*/)
{
    RECT rc = { 0 };
    GetClientRect(&rc);
    ATLVERIFY(m_ctrlMonthCal.Create(m_hWnd, rc) != NULL);
    
    return (m_ctrlMonthCal.IsWindow() != FALSE);
}

bool CCalendarWindow::InitSettingsPath()
{
    ATLASSERT(m_pathSettings.m_strPath.IsEmpty());

    // Try our folder
    TCHAR sz[MAX_PATH] = { TEXT('\0') };
    ::GetModuleFileName(NULL, sz, MAX_PATH);

    CPath pathFilename(sz);
    pathFilename.RenameExtension(TEXT(".ini"));

    if(pathFilename.FileExists())
        m_pathSettings = pathFilename;
    else
        ATLVERIFY(::MakeSettingsFilename(m_pathSettings));

    return (!m_pathSettings.m_strPath.IsEmpty());
}

bool CCalendarWindow::ApplySettings(bool bResize)
{
    // Transparency
    if(!ApplyTransparency()) return false;

    // Window size and pos (default - one month size)
    if(!ApplyWindowPos(bResize)) return false;

    // Update calendar control
    if(!ApplyCalendarProperties()) return false;

    // Update system tray icon properties
    if(!ApplySysTrayProperties()) return false;

    // Update deskband state
    if(!ApplyDeskbandProperties()) return false;

    return true;
}

bool CCalendarWindow::ApplyTransparency()
{
    const BOOL bRes = ::SetLayeredWindowAttributes(m_hWnd,
        0, m_dlgOptions.m_byteAlpha, LWA_ALPHA);
    ATLASSERT(bRes);

    return (bRes != FALSE);
}

bool CCalendarWindow::ApplyWindowPos(bool bResize)
{
    CRect rcWin;

    if(bResize)
    {
        GetWindowRect(&rcWin);

        CRect rcMin;
        MonthCal_GetMinReqRect(m_ctrlMonthCal, &rcMin);

        rcWin.right = rcWin.left + max(rcMin.Width(),
            (int)MonthCal_GetMaxTodayWidth(m_ctrlMonthCal));
        rcWin.bottom = rcWin.top + rcMin.Height();

        // Do some margins
        rcWin.InflateRect(0, 0,
            rcMin.Width() / 3, rcMin.Height() / 2);
    }

    // Apply changes
    HWND hWndInsertAfter = (m_dlgOptions.m_bTopmost ?
        HWND_TOPMOST : HWND_NOTOPMOST);
    UINT nFlags = SWP_DEFERERASE |
        (bResize ? 0 : (SWP_NOSIZE | SWP_NOMOVE));

    const BOOL bRes = SetWindowPos(hWndInsertAfter, &rcWin, nFlags);
    ATLASSERT(bRes);

    return (bRes != FALSE);
}

bool CCalendarWindow::ApplyCalendarProperties()
{
    BOOL bRes = TRUE;

    // Display week numbers
    const bool bWeekNumbersShown =
        ((m_ctrlMonthCal.GetStyle() & MCS_WEEKNUMBERS) != 0);

    if(bWeekNumbersShown != m_dlgOptions.m_bWeekNumbers)
    {
        DWORD dwRemove = 0;
        DWORD dwAdd = 0;

        if(m_dlgOptions.m_bWeekNumbers)
            dwAdd = MCS_WEEKNUMBERS;
        else
            dwRemove = MCS_WEEKNUMBERS;

        bRes = m_ctrlMonthCal.ModifyStyle(dwRemove, dwAdd);
        ATLASSERT(bRes);
    }

    return (bRes != FALSE);
}

bool CCalendarWindow::ApplySysTrayProperties()
{
    bool bRes = true;

    if(m_hSysTrayIcon)
    {
        bRes = UpdateSysTrayIcon(NIM_MODIFY);
        ATLASSERT(bRes);
    }

    return bRes;
}

bool CCalendarWindow::ApplyDeskbandProperties() const
{
    BOOL fWow64 = TRUE;
    ::IsWow64Process(::GetCurrentProcess(), &fWow64);

    if(fWow64) return true; // cannot install 32-bit deskband for Win64

    const CPath& deskbandPath = GetDeskbandModulePath();
    const bool bDeskbandInstalled = DeskbandInstalled();

    if(m_dlgOptions.m_bDeskband && !bDeskbandInstalled)
    {   // install and show
        if(!ExtractDeskbandBinary(deskbandPath))
            return false;

        if(!RegisterDeskband(deskbandPath, true))
            return false;

        // Show the desk band only if wasn't previously installed;
        // otherwise leave the choice to user.
        if(!ShowDeskband())
            return false;
    }
    else if(!m_dlgOptions.m_bDeskband && bDeskbandInstalled)
    {   // hide and uninstall

        // If there are leftovers of previous deskband objects that cannot
        // be uninstalled (missing files, etc), continue anyway.
        // No point to fail here.
        HideDeskband();
        RegisterDeskband(deskbandPath, false);
        DeleteDeskbandBinary(deskbandPath);
    }

    return true;
}

bool CCalendarWindow::DeskbandInstalled() const
{
    const CString& keyName = TEXT("CLSID\\") +
        ::StringFromGUID2(CLSID_CalendarDeskBand);

    CRegKey keyDeskband;
    const LONG lRes = keyDeskband.Open(HKEY_CLASSES_ROOT, keyName, KEY_READ);

    return (lRes == ERROR_SUCCESS);
}

CPath CCalendarWindow::GetDeskbandModulePath() const
{
    CPath deskbandPath = m_pathSettings;
    deskbandPath.RemoveFileSpec();
    deskbandPath.Append(DESKBAND_BIN_NAME);

    return deskbandPath;
}

bool CCalendarWindow::ExtractDeskbandBinary(const CPath& deskbandPath) const
{
    HRSRC hResInfo = reinterpret_cast<HRSRC>(
        ::FindResource(
            NULL,
            MAKEINTRESOURCE(IDR_DESKBAND_BIN),
            TEXT("DESKBAND")
        )
    );
    ATLASSERT(hResInfo);

    if(!hResInfo) return false;

    HGLOBAL hResData = ::LoadResource(NULL, hResInfo);
    ATLASSERT(hResData);

    if(!hResData) return false;

    LPVOID pDeskbandBinData = ::LockResource(hResData);
    ATLASSERT(pDeskbandBinData);

    if(!pDeskbandBinData) return false;

    const DWORD dwDeskbandBinSize = ::SizeofResource(NULL, hResInfo);
    ATLASSERT(dwDeskbandBinSize > 0);

    if(!dwDeskbandBinSize) return false;

    CAtlFile deskbandBin;
    HRESULT hr = deskbandBin.Create(deskbandPath,
        GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS);
    ATLASSERT(SUCCEEDED(hr));

    if(FAILED(hr)) return false;

    hr = deskbandBin.Write(pDeskbandBinData, dwDeskbandBinSize);
    ATLASSERT(SUCCEEDED(hr));
    if(FAILED(hr)) return false;

    return true;
}

bool CCalendarWindow::DeleteDeskbandBinary(const CPath& deskbandPath) const
{
    if(!deskbandPath.FileExists()) return true;

    CPath deskbandDir = deskbandPath;
    deskbandDir.RemoveFileSpec();

    TCHAR szTmpFile[MAX_PATH + 1] = { 0 };
    const UINT nRet = ::GetTempFileName(deskbandDir, TEXT("del"), 0, szTmpFile);
    ATLASSERT(nRet);

    if(!nRet) return false;

    BOOL bRes = ::MoveFileEx(deskbandPath, szTmpFile, MOVEFILE_REPLACE_EXISTING);
    ATLASSERT(bRes);

    if(!bRes) return false;

    // If user is an admin, then the file gets deleted on the next reboot;
    // for standard users the MOVEFILE_DELAY_UNTIL_REBOOT flag is unavailable,
    // so we delete with DeleteFile, which is likely to fail because
    // the deskband DLL will be in use by Explorer.exe. However, the file
    // will be deleted when last handle to it is closed.
    if(::IsUserAnAdmin())
    {
        bRes = ::MoveFileEx(szTmpFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    }
    else
    {
        ::DeleteFile(szTmpFile);
        bRes = TRUE;
    }
    ATLASSERT(bRes);

    return (bRes != FALSE);
}

bool CCalendarWindow::RegisterDeskband(const CPath& deskbandPath, bool bRegister) const
{
    bool bRes = false;
    if(::IsVistaOrHigher())
    {
        bRes = (RegisterDeskbandSecure(deskbandPath, bRegister) == ERROR_SUCCESS);
    }
    else    // XP
    {
        bRes = SUCCEEDED(RegisterDeskbandXP(deskbandPath, bRegister));
    }

    return bRes;
}

struct LibraryLoader
{
    LibraryLoader(LPCTSTR pszPath) : hModule(::LoadLibrary(pszPath)) {}
    ~LibraryLoader() { if(hModule) ::FreeLibrary(hModule); }

    operator HMODULE() const { return hModule; }

    HMODULE hModule;
};

HRESULT CCalendarWindow::RegisterDeskbandXP(const CPath& deskbandPath, bool bRegister) const
{
    LibraryLoader deskbandLib(deskbandPath);

    HRESULT hr = E_FAIL;
    if(deskbandLib)
    {
        typedef HRESULT (STDAPICALLTYPE *REGISTERSERVERPROC)(void);

        const LPCSTR pszProcName = (bRegister ?
            "DllRegisterServer" : "DllUnregisterServer");

        REGISTERSERVERPROC DllRegisterProc =
            reinterpret_cast<REGISTERSERVERPROC>(
                ::GetProcAddress(deskbandLib, pszProcName));

        if(DllRegisterProc)
        {
            hr = DllRegisterProc();
            ATLASSERT(SUCCEEDED(hr));

            return hr;
        }
    }

    hr = HRESULT_FROM_WIN32(::GetLastError());
    ATLASSERT(FALSE);

    return hr;
}

DWORD CCalendarWindow::RegisterDeskbandSecure(CPath deskbandPath, bool bRegister) const
{
    deskbandPath.QuoteSpaces();
    const CString& parameters = (bRegister ? TEXT("") : TEXT("/u ")) + deskbandPath;

    const DWORD dwExitCode = ::RunAsAdmin(m_hWnd, TEXT("REGSVR32.EXE"), parameters);
    ATLASSERT(dwExitCode == ERROR_SUCCESS);

    return dwExitCode;
}

bool CCalendarWindow::ShowDeskband() const
{
    CComPtr<ITrayDeskBand> spTrayDeskBand;
    HRESULT hr = spTrayDeskBand.CoCreateInstance(CLSID_TrayDeskBand);

    if(SUCCEEDED(hr))   // Vista and higher
    {
        hr = spTrayDeskBand->DeskBandRegistrationChanged();
        ATLASSERT(SUCCEEDED(hr));

        if(SUCCEEDED(hr))
        {
            hr = spTrayDeskBand->IsDeskBandShown(CLSID_CalendarDeskBand);
            ATLASSERT(SUCCEEDED(hr));

            if(SUCCEEDED(hr) && hr == S_FALSE)
                hr = spTrayDeskBand->ShowDeskBand(CLSID_CalendarDeskBand);
        }
    }
    else    // WinXP workaround
    {
        const CString& sAtom = ::StringFromGUID2(CLSID_CalendarDeskBand);

        if(!::GlobalFindAtom(sAtom))
            ::GlobalAddAtom(sAtom);

        // Beware! SHLoadInProc is not implemented under Vista and higher.
        hr = ::SHLoadInProc(CLSID_CalendarDeskBand);
        ATLASSERT(SUCCEEDED(hr));
    }

    return SUCCEEDED(hr);
}

bool CCalendarWindow::HideDeskband() const
{
    CComPtr<ITrayDeskBand> spTrayDeskBand;
    HRESULT hr = spTrayDeskBand.CoCreateInstance(CLSID_TrayDeskBand);

    if(SUCCEEDED(hr))   // Vista and higher
    {
        hr = spTrayDeskBand->IsDeskBandShown(CLSID_CalendarDeskBand);

        if(hr == S_OK)
            hr = spTrayDeskBand->HideDeskBand(CLSID_CalendarDeskBand);
    }
    else    // WinXP
    {
        CComPtr<IBandSite> spBandSite;
        hr = spBandSite.CoCreateInstance(CLSID_TrayBandSiteService);

        if(SUCCEEDED(hr))
        {
            DWORD dwBandID = 0;
            const UINT nBandCount = spBandSite->EnumBands((UINT)-1, &dwBandID);

            for(UINT i = 0; i < nBandCount; ++i)
            {
                spBandSite->EnumBands(i, &dwBandID);

                CComPtr<IPersist> spPersist;
                hr = spBandSite->GetBandObject(dwBandID, IID_IPersist, (void**)&spPersist);
                if(SUCCEEDED(hr))
                {
                    CLSID clsid = CLSID_NULL;
                    hr = spPersist->GetClassID(&clsid);

                    if(SUCCEEDED(hr) && ::IsEqualCLSID(clsid, CLSID_CalendarDeskBand))
                    {
                        hr = spBandSite->RemoveBand(dwBandID);
                        break;
                    }
                }
            }
        }
    }

    return SUCCEEDED(hr);
}

bool CCalendarWindow::ShowOptions(void)
{
    if(!IsWindowVisible() || IsIconic())
        ShowAndActivate();

    if(IDOK == m_dlgOptions.DoModal(m_hWnd))
    {
        ATLVERIFY(m_dlgOptions.SaveSettings(m_pathSettings));
        ATLVERIFY(ApplySettings(/*Resize=*/FALSE));
    }

    return true;
}

bool CCalendarWindow::UpdateSysTrayIcon(DWORD dwAction)
{
    NOTIFYICONDATA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = m_hWnd;

    HICON hIconPrev = NULL;

    switch(dwAction)
    {
    case NIM_MODIFY:
        hIconPrev = m_hSysTrayIcon;
        // fall through
    case NIM_ADD:
        {
            SYSTEMTIME stToday = { 0 };
            ::GetLocalTime(&stToday);
            const CString& strDateStr =
                m_dlgOptions.m_dateFormat.FormatDateString(stToday);

            m_hSysTrayIcon = LoadDayIcon(stToday);

            nid.uFlags |= NIF_TIP | NIF_ICON | NIF_MESSAGE;
            nid.uCallbackMessage = WM_SYSTRAYNOTIFY;
            lstrcpyn(nid.szTip, strDateStr, ARRAYSIZE(nid.szTip));
            nid.hIcon = m_hSysTrayIcon;
        }
        break;
    case NIM_DELETE:
        hIconPrev = m_hSysTrayIcon;
        break;
    }

    BOOL bRes = ::Shell_NotifyIcon(dwAction, &nid);
    ATLASSERT(bRes);

    if(hIconPrev != NULL)
        ::DestroyIcon(hIconPrev);

    return (bRes != FALSE);
}

// Note: This function relies on the fact that
//       daily icon ID's increase incrementally.
HICON CCalendarWindow::LoadDayIcon(const SYSTEMTIME& st)
{
    HICON hIcon = (HICON)::LoadImage(
        _AtlBaseModule.GetResourceInstance(),
        MAKEINTRESOURCE(IDI_DAY00 + st.wDay),
        IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);

    ATLASSERT(hIcon != NULL);

    return hIcon;
}

void CCalendarWindow::ShowAboutBox(void)
{
    MSGBOXPARAMS mbp = { 0 };
    mbp.cbSize      = sizeof(MSGBOXPARAMS);
    mbp.hwndOwner   = m_hWnd;
    mbp.hInstance   = _AtlBaseModule.GetResourceInstance();
    mbp.lpszText    = MAKEINTRESOURCE(IDS_ABOUTTEXT);
    mbp.lpszCaption = MAKEINTRESOURCE(IDS_ABOUTCAPTION);
    mbp.dwStyle     = MB_USERICON | MB_OK;
    mbp.lpszIcon    = MAKEINTRESOURCE(IDI_CALENDAR);
    mbp.dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

    ::MessageBoxIndirect(&mbp);
}

int CCalendarWindow::ShowContextMenu(void)
{
    HMENU hMenu = ::LoadMenu(
        _AtlBaseModule.GetResourceInstance(),
        MAKEINTRESOURCE(IDM_CALENDAR));
    ATLASSERT(hMenu != NULL);

    HMENU hTrackPopup = ::GetSubMenu(hMenu, 0);
    ATLASSERT(hTrackPopup != NULL);

    const UINT nCheck = MF_BYCOMMAND |
        (IsWindowVisible() ? MF_CHECKED : MF_UNCHECKED);
    ::CheckMenuItem(hTrackPopup, IDC_CALENDAR, nCheck);

    POINT pt = { 0 };
    ::GetCursorPos(&pt);

    ::SetForegroundWindow(m_hWnd);

    int nCmd = ::TrackPopupMenu(hTrackPopup, TPM_RETURNCMD,
        pt.x, pt.y, 0, m_hWnd, NULL);

    PostMessage(WM_NULL);

    return nCmd;
}

void CCalendarWindow::HandleContextCommand(int nCmd)
{
    if(nCmd == 0) return;

    switch(nCmd)
    {
    case IDC_CALENDAR:
        if(IsWindowVisible())
            ShowWindow(SW_HIDE);
        else
            ShowAndActivate();
        break;
    case IDC_OPTIONS:
        ShowOptions();
        break;
    case IDC_ABOUT:
        ShowAboutBox();
        break;
    case IDC_EXIT:
        DestroyWindow();
        break;
    default:
        ATLASSERT(FALSE);
    }
}

UINT_PTR CCalendarWindow::SetUpdateTimer()
{
    // Wake up every 10 seconds.
    UINT_PTR nTimerId = SetTimer(UPDATE_TIMER_ID, 1000);

    ATLASSERT(nTimerId != 0);

    return nTimerId;
}

void CCalendarWindow::UpdateTodayDate(const SYSTEMTIME& stNow)
{
    MonthCal_SetToday(m_ctrlMonthCal, &stNow);

    ATLVERIFY(UpdateSysTrayIcon(NIM_MODIFY));
}

void CCalendarWindow::ShowAndActivate()
{
    if(!IsWindowVisible() || IsIconic())
        ShowWindow(SW_SHOWNORMAL);

    ::SetForegroundWindow(m_hWnd);
}

/////////////////////////////////////////////////////////////////////////////