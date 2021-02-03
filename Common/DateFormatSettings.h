// DateFormatSettings.h : Declaration of the CDateFormatSettings

#pragma once

#include "DateFormatSettingsRes.h"
#include "DateFormat.h"

using namespace ATL;

////////////////////////////////////////////////////////////////////////////////
// CDateFormatSettings

class CDateFormatSettings : 
	public CDialogImpl<CDateFormatSettings>
{
public:
	CDateFormatSettings();
	~CDateFormatSettings();

	enum { IDD = IDD_DATEFORMAT };

BEGIN_MSG_MAP(CDateFormatSettings)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
    COMMAND_HANDLER(IDC_LANGUAGE, CBN_SELCHANGE, OnChangeLanguage)
    COMMAND_HANDLER(IDC_CALENDARTYPE, CBN_SELCHANGE, OnChangeCalendarType)
    COMMAND_HANDLER(IDC_DATEFORMAT, CBN_SELCHANGE, OnChangeCustomDateFormat)
    COMMAND_HANDLER(IDC_DATEFORMAT, CBN_EDITCHANGE, OnEditCustomDateFormat)
    COMMAND_RANGE_CODE_HANDLER(IDC_SHORTDATE, IDC_CUSTOMDATE, BN_CLICKED, OnClickedDateFormat)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnChangeLanguage(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnChangeCalendarType(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnChangeCustomDateFormat(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnEditCustomDateFormat(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClickedDateFormat(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

public:
    DateFormat m_dateFormat;

private:
    BOOL InitControls();
    BOOL InitControlHwnds();
    BOOL InitLanguages();
    BOOL InitCalendars();
    BOOL InitDateFormat(CALTYPE calType);
    BOOL InitCustomDateFormat();

    BOOL SelectLanguage();
    BOOL SelectCurrentCalendar();
    BOOL SelectDateFormat(int nIDButton);
    BOOL SelectCustomDateFormat();

    BOOL EnumKeyboardLayouts();
    BOOL EnumAvailableDateFormats();

    BOOL UpdateCurrentCalendar();
    BOOL UpdateDateFormat(CALTYPE calTypeHint = CAL_DEFAULT);
    BOOL UpdateSampleDate();

    BOOL AddLocale(LCID lcid);

    LCID GetSelectedLcid() const;
    CALID GetSelectedCalendar() const;
    CALTYPE GetSelectedCalendarType() const;
    CString GetSelectedDateFormat() const;
    CString GetSelectedCustomDateFormat() const;

    BOOL CollectCalendarSettings();

    // enum calendars
    static BOOL CALLBACK EnumCalendarInfoProcEx(LPTSTR lpCalendarInfoString, CALID Calendar);

    BOOL EnumCalendarNames(LCID lcid);
    BOOL AddCalendarName(LPCTSTR lpCalendarName, CALID Calendar);

    BOOL EnumCalendarDateFormats(LCID lcid, CALID Calendar);
    BOOL AddCalendarDateFormat(LPCTSTR lpDateFormat, CALID Calendar);

    CALTYPE m_enumCalendarInfo;
    //

private:
    HWND m_hwndLanguage;
    HWND m_hwndCalendar;
    HWND m_hwndFormat;
    HWND m_hwndSample;
};

////////////////////////////////////////////////////////////////////////////////