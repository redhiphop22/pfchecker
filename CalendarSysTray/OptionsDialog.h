#pragma once

#include "resource.h"       // main symbols
#include "DateFormat.h"

// COptionsDialog

class COptionsDialog : 
    public CDialogImpl<COptionsDialog>
{
public:
    COptionsDialog();
    ~COptionsDialog();

    void LoadSettings(LPCTSTR pszFilename);
    bool SaveSettings(LPCTSTR pszFilename) const;

    enum { IDD = IDD_OPTIONS };

BEGIN_MSG_MAP(COptionsDialog)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    COMMAND_HANDLER(IDC_FORMATDATE, BN_CLICKED, OnFormatDate)
    COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
    COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnFormatDate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
    bool SaveDateFormat(LPCTSTR pszFilename) const;
    void UpdateTooltipText();

public:
    BYTE m_byteAlpha; // min - 0; max - 255
    bool m_bTopmost;
    bool m_bWeekNumbers;
    bool m_bDeskband;
    DateFormat m_dateFormat;
};

/////////////////////////////////////////////////////////////////////////////