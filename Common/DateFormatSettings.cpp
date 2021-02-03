// DateFormatSettings.cpp : Implementation of CDateFormatSettings

#include "StdAfx.h"
#include "DateFormatSettings.h"

////////////////////////////////////////////////////////////////////////////////
// 
namespace
{
    CDateFormatSettings* g_This;

    const struct
    {
        int nIDButton;
        CALTYPE calDateFormat;
    } g_dateFormat[] = {
        { IDC_LONGDATE, CAL_SLONGDATE },
        { IDC_SHORTDATE, CAL_SSHORTDATE },
        { IDC_CUSTOMDATE, CAL_SCUSTOMDATE }
    };
}

///////////////////////////////////////////////////////////////////////////////
// 
BOOL CBAddString(HWND hwndCtl, LPCTSTR lpString, LPARAM data)
{
    const int itemIdx = ComboBox_AddString(hwndCtl, lpString);
    ATLASSERT(itemIdx != CB_ERR && itemIdx != CB_ERRSPACE);

    const int ret = ComboBox_SetItemData(hwndCtl, itemIdx, data);
    ATLASSERT(ret != CB_ERR);

    return (ret != CB_ERR);
}

int CBSelectItemData(HWND hwndCtl, LPARAM data)
{
    const int itemCount = ComboBox_GetCount(hwndCtl);
    int itemToSelect = CB_ERR;

    for(int itemIdx = 0; itemIdx < itemCount; ++itemIdx)
    {
        const LPARAM itemData = ComboBox_GetItemData(hwndCtl, itemIdx);

        if(itemData == data)
        {
            itemToSelect = itemIdx;
            break;
        }
    }

    if(itemToSelect != CB_ERR)
        ComboBox_SetCurSel(hwndCtl, itemToSelect);

    return itemToSelect;
}

BOOL CBAddSelectItemString(HWND hwndCtl, LPCTSTR pszString)
{
    ATLASSERT(pszString);

    int itemIdx = ComboBox_FindStringExact(hwndCtl, -1, pszString);

    if(itemIdx == CB_ERR)
    {
        if(lstrlen(pszString) > 0)
            itemIdx = ComboBox_AddString(hwndCtl, pszString);
        else
            itemIdx = 0;
    }

    itemIdx = ComboBox_SetCurSel(hwndCtl, itemIdx);
    ATLASSERT(itemIdx != CB_ERR);

    return (itemIdx != CB_ERR);
}

LPARAM CBGetCurSelData(HWND hwndCtl)
{
    const int itemIdx = ComboBox_GetCurSel(hwndCtl);
    ATLASSERT(itemIdx != CB_ERR);

    return ComboBox_GetItemData(hwndCtl, itemIdx);
}

////////////////////////////////////////////////////////////////////////////////
// CDateFormatSettings

CDateFormatSettings::CDateFormatSettings() : m_enumCalendarInfo(0)
{
    g_This = this;
}

CDateFormatSettings::~CDateFormatSettings()
{
}

LRESULT CDateFormatSettings::OnInitDialog(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM lParam,
    BOOL& /*bHandled*/)
{
    if(lParam)
    {
        DateFormat* pDateFormat =
            reinterpret_cast<DateFormat*>(lParam);
        m_dateFormat = *pDateFormat;
    }

    InitControls();
    UpdateSampleDate();

	return 1;  // Let the system set the focus
}

LRESULT CDateFormatSettings::OnClickedOK(
    WORD /*wNotifyCode*/,
    WORD wID,
    HWND /*hWndCtl*/,
    BOOL& /*bHandled*/)
{
    const BOOL bRes = CollectCalendarSettings();
    ATLASSERT(bRes);

	EndDialog(wID);
	return 0;
}

LRESULT CDateFormatSettings::OnClickedCancel(
    WORD /*wNotifyCode*/,
    WORD wID,
    HWND /*hWndCtl*/,
    BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}

LRESULT CDateFormatSettings::OnChangeLanguage(
    WORD /*wNotifyCode*/,
    WORD /*wID*/,
    HWND /*hWndCtl*/,
    BOOL& /*bHandled*/)
{
    UpdateCurrentCalendar();
    UpdateDateFormat();
    UpdateSampleDate();

    return 0;
}

LRESULT CDateFormatSettings::OnChangeCalendarType(
    WORD /*wNotifyCode*/,
    WORD /*wID*/,
    HWND /*hWndCtl*/,
    BOOL& /*bHandled*/)
{
    UpdateDateFormat();
    UpdateSampleDate();

    return 0;
}

LRESULT CDateFormatSettings::OnChangeCustomDateFormat(
    WORD /*wNotifyCode*/,
    WORD wID,
    HWND hWndCtl,
    BOOL& /*bHandled*/)
{
    // Upon CBN_SELCHANGE notification, the combo's edit box is not updated yet.
    // So, in order to get correct format string we _post_ CBN_EDITCHANGE
    // notification. On arival of CBN_EDITCHANGE, the edit box will be
    // already updated.

    PostMessage(WM_COMMAND,
        MAKEWPARAM(wID, CBN_EDITCHANGE),
        reinterpret_cast<LPARAM>(hWndCtl));

    return 0;
}

LRESULT CDateFormatSettings::OnEditCustomDateFormat(
    WORD /*wNotifyCode*/,
    WORD /*wID*/,
    HWND /*hWndCtl*/,
    BOOL& /*bHandled*/)
{
    UpdateSampleDate();

    return 0;
}

LRESULT CDateFormatSettings::OnClickedDateFormat(
    WORD /*wNotifyCode*/,
    WORD wID,
    HWND /*hWndCtl*/,
    BOOL& /*bHandled*/)
{
    const CALTYPE calType = GetSelectedCalendarType();

    if(calType != m_dateFormat.calType)
    {
        m_dateFormat.calType = calType;

        BOOL bRes = SelectDateFormat(wID);
        ATLASSERT(bRes);

        bRes = UpdateSampleDate();
        ATLASSERT(bRes);
    }
    
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 

BOOL CDateFormatSettings::InitControls()
{
    BOOL bRes = FALSE;
    
    if(InitControlHwnds())
    {
        if(InitLanguages())
            bRes = UpdateDateFormat(m_dateFormat.calType);
    }

    return bRes;
}

BOOL CDateFormatSettings::InitControlHwnds()
{
    m_hwndLanguage = GetDlgItem(IDC_LANGUAGE);
    m_hwndCalendar = GetDlgItem(IDC_CALENDARTYPE);
    m_hwndFormat = GetDlgItem(IDC_DATEFORMAT);
    m_hwndSample = GetDlgItem(IDC_DATESAMPLE);

    return TRUE;
}

BOOL CDateFormatSettings::InitLanguages()
{
    BOOL bRes = FALSE;
    
    if(EnumKeyboardLayouts())
    {
        bRes = SelectLanguage();

        if(bRes)
            bRes = UpdateCurrentCalendar();
    }

    return bRes;
}



BOOL CDateFormatSettings::InitCalendars()
{
    ComboBox_ResetContent(m_hwndCalendar);

    const LCID lcid = GetSelectedLcid();

    return EnumCalendarNames(lcid);
}

BOOL CDateFormatSettings::InitDateFormat(CALTYPE calType)
{
    int nIDButton;

    switch(calType)
    {
    case CAL_SLONGDATE:
        nIDButton = IDC_LONGDATE;
        break;
    case CAL_SSHORTDATE:
        nIDButton = IDC_SHORTDATE;
        break;
    default:
        nIDButton = IDC_CUSTOMDATE;
    }

    CheckRadioButton(IDC_SHORTDATE, IDC_CUSTOMDATE, nIDButton);
    BOOL bRes = SelectDateFormat(nIDButton);

    return bRes;
}

BOOL CDateFormatSettings::InitCustomDateFormat()
{
    ComboBox_ResetContent(m_hwndFormat);
    BOOL bRes = EnumAvailableDateFormats();

    if(bRes)
        bRes = SelectCustomDateFormat();

    return bRes;
}

BOOL CDateFormatSettings::SelectLanguage()
{
    // first try preset LCID, then user default one, then first item's
    const LCID lcidToSelect[] = {
        m_dateFormat.lcId,
        ::GetUserDefaultLCID(),
        static_cast<LCID>(ComboBox_GetItemData(m_hwndLanguage, 0))
    };

    int itemIdx = CB_ERR;
    for(size_t i = 0; i < ARRAYSIZE(lcidToSelect); ++i)
    {
        itemIdx = ::CBSelectItemData(m_hwndLanguage, lcidToSelect[i]);

        if(itemIdx != CB_ERR)
            break;
    }

    return (itemIdx != CB_ERR);
}

BOOL CDateFormatSettings::SelectCurrentCalendar()
{
    // try preset calendar first, then Gregorian
    const CALID calendarToSelect[] = {
        m_dateFormat.calId,
        CAL_GREGORIAN
    };

    int itemIdx = CB_ERR;
    for(size_t i = 0; i < ARRAYSIZE(calendarToSelect); ++i)
    {
        itemIdx = ::CBSelectItemData(m_hwndCalendar, calendarToSelect[i]);

        if(itemIdx != CB_ERR)
            break;
    }

    return (itemIdx != CB_ERR);
}

BOOL CDateFormatSettings::SelectDateFormat(int nIDButton)
{
    const BOOL bEnable = (nIDButton == IDC_CUSTOMDATE);

    ::EnableWindow(m_hwndFormat, bEnable);

    BOOL bRes = TRUE;
    if(nIDButton == IDC_CUSTOMDATE)
        bRes = InitCustomDateFormat();

    return bRes;
}

BOOL CDateFormatSettings::SelectCustomDateFormat()
{
    ::CBAddSelectItemString(m_hwndFormat, m_dateFormat.dateFormat);

    return TRUE;
}

BOOL CDateFormatSettings::EnumKeyboardLayouts()
{
    const int layoutCount = ::GetKeyboardLayoutList(0, NULL);
    CAtlArray<HKL> layout;
    layout.SetCount(layoutCount);

    ::GetKeyboardLayoutList(layoutCount, layout.GetData());

    for(int i = 0; i < layoutCount; ++i)
    {
        const LANGID lid = LOWORD(layout[i]);
        const LCID lcid = MAKELCID(lid, SORT_DEFAULT);

        AddLocale(lcid);
    }

    return (ComboBox_GetCount(m_hwndLanguage) > 0);
}

BOOL CDateFormatSettings::EnumAvailableDateFormats()
{
    const LCID lcid = GetSelectedLcid();
    const CALID calid = GetSelectedCalendar();

    return EnumCalendarDateFormats(lcid, calid);
}

BOOL CDateFormatSettings::UpdateCurrentCalendar()
{
    BOOL bRes = FALSE;

    if(InitCalendars())
        bRes = SelectCurrentCalendar();

    return bRes;
}

BOOL CDateFormatSettings::UpdateDateFormat(CALTYPE calTypeHint)
{
    const CALID calid = GetSelectedCalendar();
    const BOOL bEnableCustomDate = (calid == CAL_GREGORIAN);

    ::EnableWindow(GetDlgItem(IDC_CUSTOMDATE), bEnableCustomDate);

    const CALTYPE calType = (calTypeHint == CAL_DEFAULT ? CAL_SLONGDATE : calTypeHint);

    return InitDateFormat(calType);
}

BOOL CDateFormatSettings::UpdateSampleDate()
{
    BOOL bRes = CollectCalendarSettings();
    ATLASSERT(bRes);

    SYSTEMTIME st = { 0 };
    ::GetSystemTime(&st);

    const CString& sampleDate = m_dateFormat.FormatDateString(st);
    ATLASSERT(!sampleDate.IsEmpty());
    
    ::Edit_SetText(m_hwndSample, sampleDate);

    return !sampleDate.IsEmpty();
}

BOOL CDateFormatSettings::AddLocale(LCID lcid)
{
    const int langLen = ::GetLocaleInfo(lcid, LOCALE_SLANGUAGE, NULL, 0);
    ATLASSERT(langLen > 0);

    CString layoutName;
    ::GetLocaleInfo(lcid, LOCALE_SLANGUAGE,
        layoutName.GetBufferSetLength(langLen), langLen);
    layoutName.ReleaseBuffer();

    return ::CBAddString(m_hwndLanguage, layoutName, lcid);
}

LCID CDateFormatSettings::GetSelectedLcid() const
{
    return static_cast<LCID>(::CBGetCurSelData(m_hwndLanguage));
}

CALID CDateFormatSettings::GetSelectedCalendar() const
{
    return static_cast<CALID>(::CBGetCurSelData(m_hwndCalendar));
}

CALTYPE CDateFormatSettings::GetSelectedCalendarType() const
{
    CALTYPE calDate = CAL_SCUSTOMDATE;
    for(size_t i = 0; i < ARRAYSIZE(g_dateFormat); ++i)
    {
        if(IsDlgButtonChecked(g_dateFormat[i].nIDButton) == BST_CHECKED)
        {
            calDate = g_dateFormat[i].calDateFormat;
            break;
        }
    }

    return calDate;
}

CString CDateFormatSettings::GetSelectedDateFormat() const
{
    CString dateFormat;

    const CALTYPE calDate = GetSelectedCalendarType();

    if(calDate == CAL_SCUSTOMDATE)
        dateFormat = GetSelectedCustomDateFormat();

    return dateFormat;
}

CString CDateFormatSettings::GetSelectedCustomDateFormat() const
{
    CString customFormat;

    const int textLen = ::ComboBox_GetTextLength(m_hwndFormat);
    ::ComboBox_GetText(m_hwndFormat, customFormat.GetBuffer(textLen), textLen);
    customFormat.ReleaseBuffer();

    return customFormat;
}

BOOL CDateFormatSettings::CollectCalendarSettings()
{
    m_dateFormat.lcId = GetSelectedLcid();
    m_dateFormat.calId = GetSelectedCalendar();
    m_dateFormat.calType = GetSelectedCalendarType();
    m_dateFormat.dateFormat = GetSelectedDateFormat();

    return TRUE;
}

// static
BOOL CALLBACK CDateFormatSettings::EnumCalendarInfoProcEx(
    LPTSTR lpCalendarInfoString,
    CALID Calendar)
{
    switch(g_This->m_enumCalendarInfo)
    {
    case CAL_SCALNAME:
        g_This->AddCalendarName(lpCalendarInfoString, Calendar);
        break;
    case CAL_SLONGDATE:
    case CAL_SSHORTDATE:
        g_This->AddCalendarDateFormat(lpCalendarInfoString, Calendar);
        break;
    }
    
    return TRUE;
}

BOOL CDateFormatSettings::EnumCalendarNames(LCID lcid)
{
    m_enumCalendarInfo = CAL_SCALNAME;
    const BOOL bRes = ::EnumCalendarInfoEx(
        &CDateFormatSettings::EnumCalendarInfoProcEx,
        lcid, ENUM_ALL_CALENDARS, CAL_SCALNAME);
    m_enumCalendarInfo = 0;

    return bRes;
}

BOOL CDateFormatSettings::AddCalendarName(LPCTSTR lpCalendarName, CALID Calendar)
{
    BOOL bContinue = TRUE;

    // Gregorian calendar may be listed twice for U.S. English (LCID: 0x0409) -
    // first time as CAL_GREGORIAN, second time as CAL_GREGORIAN_US.
    // We need only one of them = CAL_GREGORIAN.
    if(Calendar != CAL_GREGORIAN_US)
        bContinue = ::CBAddString(m_hwndCalendar, lpCalendarName, Calendar);

    return bContinue;
}

BOOL CDateFormatSettings::EnumCalendarDateFormats(LCID lcid, CALID Calendar)
{
    const CALTYPE dateFormat[] = { CAL_SLONGDATE, CAL_SSHORTDATE };

    BOOL bRes = TRUE;
    for(size_t i = 0; i < ARRAYSIZE(dateFormat); ++i)
    {
        m_enumCalendarInfo = dateFormat[i];
        bRes &= ::EnumCalendarInfoEx(
            &CDateFormatSettings::EnumCalendarInfoProcEx,
            lcid, Calendar, dateFormat[i]);
        m_enumCalendarInfo = 0;
    }

    return bRes;
}

BOOL CDateFormatSettings::AddCalendarDateFormat(LPCTSTR lpDateFormat, CALID /*Calendar*/)
{
    return ::CBAddString(m_hwndFormat, lpDateFormat, 0);
}

////////////////////////////////////////////////////////////////////////////////