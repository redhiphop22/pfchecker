#include "StdAfx.h"
#include "OptionsDialog.h"
#include "DateFormatSettings.h"

/////////////////////////////////////////////////////////////////////////////
//
const LPCTSTR SETTINGS_SECTION_APPEARANCE   = TEXT("Appearance");
const LPCTSTR SETTINGS_SECTION_CALENDAR     = TEXT("Calendar");

const LPCTSTR SETTINGS_KEY_ALPHA        = TEXT("Alpha");
const LPCTSTR SETTINGS_KEY_TOPMOST      = TEXT("Topmost");
const LPCTSTR SETTINGS_KEY_WEEKNUMBERS  = TEXT("Weeknumbers");
const LPCTSTR SETTINGS_KEY_DESKBAND     = TEXT("Deskband");

const BYTE DEFAULT_ALPHA = (255 * 85) / 100; // 15% transparent

/////////////////////////////////////////////////////////////////////////////
//
bool EnsureFolderPathExists(LPCTSTR pszPath)
{
    CPath pathFolder(pszPath);
    pathFolder.RemoveFileSpec();

    if(pathFolder.IsDirectory())
        return true;

    ATLASSERT(!pathFolder.FileExists());

    const int nRes = ::SHCreateDirectoryEx(NULL, pathFolder, NULL);
    ATLASSERT(nRes == ERROR_SUCCESS);

    return (nRes == ERROR_SUCCESS);
}

/////////////////////////////////////////////////////////////////////////////
// COptionsDialog
COptionsDialog::COptionsDialog() :
    m_byteAlpha(DEFAULT_ALPHA),
    m_bTopmost(false),
    m_bWeekNumbers(false),
    m_bDeskband(false)
{
}

COptionsDialog::~COptionsDialog()
{
}

/////////////////////////////////////////////////////////////////////////////
//
void COptionsDialog::LoadSettings(LPCTSTR pszFilename)
{
    m_byteAlpha = (BYTE)::GetPrivateProfileInt(
        SETTINGS_SECTION_APPEARANCE, SETTINGS_KEY_ALPHA,
        DEFAULT_ALPHA, pszFilename);

    m_bTopmost = (::GetPrivateProfileInt(
        SETTINGS_SECTION_APPEARANCE, SETTINGS_KEY_TOPMOST,
        FALSE, pszFilename) != 0);

    m_bWeekNumbers = (::GetPrivateProfileInt(
        SETTINGS_SECTION_CALENDAR, SETTINGS_KEY_WEEKNUMBERS,
        FALSE, pszFilename) != 0);

    m_bDeskband = (::GetPrivateProfileInt(
        SETTINGS_SECTION_APPEARANCE, SETTINGS_KEY_DESKBAND,
        FALSE, pszFilename) != 0);

    m_dateFormat.Load(pszFilename);
}

bool COptionsDialog::SaveSettings(LPCTSTR pszFilename) const
{
    if(!::EnsureFolderPathExists(pszFilename))
        return false;

    TCHAR szVal[MAX_PATH] = { TEXT('\0') };
    BOOL bWrite = FALSE;

    _itot(m_byteAlpha, szVal, 10);
    bWrite = ::WritePrivateProfileString(
        SETTINGS_SECTION_APPEARANCE, SETTINGS_KEY_ALPHA,
        szVal, pszFilename);
    ATLASSERT(bWrite);

    if(!bWrite)
        return false;

    bWrite = ::WritePrivateProfileString(
        SETTINGS_SECTION_APPEARANCE, SETTINGS_KEY_TOPMOST,
        (m_bTopmost ? TEXT("1") : TEXT("0")), pszFilename);
    ATLASSERT(bWrite);

    if(!bWrite)
        return false;

    bWrite = ::WritePrivateProfileString(
        SETTINGS_SECTION_APPEARANCE, SETTINGS_KEY_DESKBAND,
        (m_bDeskband ? TEXT("1") : TEXT("0")), pszFilename);
    ATLASSERT(bWrite);

    if(!bWrite)
        return false;

    bWrite = ::WritePrivateProfileString(
        SETTINGS_SECTION_CALENDAR, SETTINGS_KEY_WEEKNUMBERS,
        (m_bWeekNumbers ? TEXT("1") : TEXT("0")), pszFilename);
    ATLASSERT(bWrite);

    if(!bWrite)
        return false;

    return m_dateFormat.Save(pszFilename);
}

void COptionsDialog::UpdateTooltipText()
{
    SYSTEMTIME st = { 0 };
    ::GetLocalTime(&st);
    SetDlgItemText(IDC_TOOLTIPTEXT, m_dateFormat.FormatDateString(st));
}

LRESULT COptionsDialog::OnInitDialog(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    // Topmost check box
    CheckDlgButton(IDC_TOPMOST,
        (m_bTopmost ? BST_CHECKED : BST_UNCHECKED));

    // Transparency trackbar
    SendDlgItemMessage(IDC_TRANSPARENCY, TBM_SETRANGE,
        /*Redraw=*/FALSE, /*(min, max)=*/MAKELONG(0, 100));
    SendDlgItemMessage(IDC_TRANSPARENCY, TBM_SETTICFREQ,
        /*Frequency=*/10);
    SendDlgItemMessage(IDC_TRANSPARENCY, TBM_SETPOS,
        /*Redraw=*/TRUE,
        /*alpha=*/(100 * (255 - m_byteAlpha)) / 255);

    // Display week numbers check box
    CheckDlgButton(IDC_WEEKNUMBERS,
        (m_bWeekNumbers ? BST_CHECKED : BST_UNCHECKED));

    // Deskband checkbox
    BOOL fWow64 = TRUE;
    ::IsWow64Process(::GetCurrentProcess(), &fWow64);

    if(fWow64)
    {
        GetDlgItem(IDC_DESKBAND).ShowWindow(SW_HIDE);
    }
    else
    {
        CheckDlgButton(IDC_DESKBAND,
            (m_bDeskband ? BST_CHECKED : BST_UNCHECKED));
    }

    // Tooltip text
    UpdateTooltipText();

    return 1;  // Let the system set the focus
}

LRESULT COptionsDialog::OnFormatDate(
    WORD /*wNotifyCode*/,
    WORD /*wID*/,
    HWND /*hWndCtl*/,
    BOOL& /*bHandled*/)
{
    CDateFormatSettings dlgSettings;
    const INT_PTR res = dlgSettings.DoModal(*this,
        reinterpret_cast<LPARAM>(&m_dateFormat));

    if(res == IDOK)
    {
        m_dateFormat = dlgSettings.m_dateFormat;
        UpdateTooltipText();
    }

    return 0;
}

LRESULT COptionsDialog::OnClickedOK(
    WORD /*wNotifyCode*/,
    WORD wID,
    HWND /*hWndCtl*/,
    BOOL& /*bHandled*/)
{
    m_bTopmost = (IsDlgButtonChecked(IDC_TOPMOST) == BST_CHECKED);
    m_byteAlpha = (BYTE)(255 - (255 * SendDlgItemMessage(IDC_TRANSPARENCY, TBM_GETPOS)) / 100);
    m_bWeekNumbers = (IsDlgButtonChecked(IDC_WEEKNUMBERS) == BST_CHECKED);
    m_bDeskband = (IsDlgButtonChecked(IDC_DESKBAND) == BST_CHECKED);

    EndDialog(wID);

    return 0;
}

LRESULT COptionsDialog::OnClickedCancel(
    WORD /*wNotifyCode*/,
    WORD wID,
    HWND /*hWndCtl*/,
    BOOL& /*bHandled*/)
{
    EndDialog(wID);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////