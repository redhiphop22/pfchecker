#pragma once

#include "OptionsDialog.h"
#include "../PFCheckerDeskBand/UseSystemInfo.h"
///////////////////////////////////////////////////////////////////////////
//
typedef CWinTraitsOR<0, WS_EX_LAYERED, CFrameWinTraits> CCalendarWinTraits;

#define MONTHCAL_MSG_MAP_ID  1
const UINT WM_SYSTRAYNOTIFY = WM_APP + 1;
const UINT WM_APPBARNOTIFY = WM_APP + 2;

///////////////////////////////////////////////////////////////////////////
//
class CCalendarWindow :
    public CWindowImpl<CCalendarWindow, CWindow, CCalendarWinTraits>
{
public:
    CCalendarWindow();

    DECLARE_WND_CLASS_EX(TEXT("CalendarWindow"), 0, COLOR_WINDOW)

    static LPCTSTR GetWndCaption()
    {
        static CString strCaption;
        if(strCaption.IsEmpty())
        {
            ATLVERIFY(strCaption.LoadString(IDS_CALENDAR));
        }
            
        return strCaption;
    }

    BEGIN_MSG_MAP(CCalendarWindow)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SYSTRAYNOTIFY, OnSysTrayNotify)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        MESSAGE_HANDLER(WM_TIMECHANGE, OnTimeChange)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
        MESSAGE_HANDLER(WM_ENDSESSION, OnEndSession)
        MESSAGE_HANDLER(WM_POWERBROADCAST, OnPowerBroadcast)
        MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
        MESSAGE_HANDLER(s_uTaskbarRestart, OnTaskbarRestart)
        ALT_MSG_MAP(MONTHCAL_MSG_MAP_ID)
    END_MSG_MAP()

// message handlers
    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnSysTrayNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnTimeChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnEndSession(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnPowerBroadcast(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnTaskbarRestart(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
    bool InitMenu();
    bool InitIcons();
    bool CreateChildren(LPCREATESTRUCT pcs);
    bool ApplySettings(bool bResize);
    bool ApplyTransparency();
    bool ApplyWindowPos(bool bResize);
    bool ApplyCalendarProperties();
    bool ApplySysTrayProperties();
    bool ApplyDeskbandProperties() const;
    bool DeskbandInstalled() const;
    CPath GetDeskbandModulePath() const;
    bool ExtractDeskbandBinary(const CPath& deskbandPath) const;
    bool DeleteDeskbandBinary(const CPath& deskbandPath) const;
    bool RegisterDeskband(const CPath& deskbandPath, bool bRegister) const;
    HRESULT RegisterDeskbandXP(const CPath& deskbandPath, bool bRegister) const;
    DWORD RegisterDeskbandSecure(CPath deskbandPath, bool bRegister) const;
    bool ShowDeskband() const;
    bool HideDeskband() const;
    bool ShowOptions();
    bool InitSettingsPath();
    bool UpdateSysTrayIcon(DWORD dwAction);
    HICON LoadDayIcon(const SYSTEMTIME& st);
    void ShowAboutBox();
    int ShowContextMenu();
    void HandleContextCommand(int nCmd);
    UINT_PTR SetUpdateTimer();
    void UpdateTodayDate(const SYSTEMTIME& stNow);
    void ShowAndActivate();

    const static int UPDATE_TIMER_ID = 1;

private:
    CContainedWindow m_ctrlMonthCal;
    COptionsDialog m_dlgOptions;
    CPath m_pathSettings;
    HICON m_hSysTrayIcon;
    HICON m_hWndIconSmall;
    HICON m_hWndIcon;

    static UINT s_uTaskbarRestart;

	UseSystemInfo	m_UseSystemInfo;
};

/////////////////////////////////////////////////////////////////////////////