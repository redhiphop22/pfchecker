#pragma once

using namespace ATL;

#include "UseSystemInfo.h"
#include "DrawGraph.h"
////////////////////////////////////////////////////////////////////////////////
// CCalendarWindow 
class DateFormat;
class CVisualStyle;

enum DRAW_GRAPH
{
	DRAW_GRAPH_CPU,
	DRAW_GRAPH_MEM,

	DRAW_GRAPH_MAX
};

class CCalendarWindow :
    public CWindowImpl<CCalendarWindow>
{
public:
    CCalendarWindow();
    ~CCalendarWindow();

    BOOL Create(
        HWND hwndParent,
        LPUNKNOWN pDeskBand,
        LPUNKNOWN pInputObjectSite,
        const DateFormat* pDateFormat);

    POINTL CalcMinimalSize() const;
    POINTL CalcIdealSize() const;
    BOOL HasFocus() const;
    void UpdateCaption();

BEGIN_MSG_MAP(CCalendarWindow)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
    MESSAGE_HANDLER(WM_KILLFOCUS, OnFocus)
    MESSAGE_HANDLER(WM_POWERBROADCAST, OnPowerBroadcast)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    MESSAGE_HANDLER(WM_TIMECHANGE, OnTimeChange)
    MESSAGE_HANDLER(WM_THEMECHANGED, OnThemeChanged)
	MESSAGE_HANDLER(WM_SIZE, OnSize)	
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

private:
    LRESULT OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPowerBroadcast(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTimeChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnThemeChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    void Paint(HDC hdc, const RECT& rcPaint) const;
    UINT_PTR SetUpdateTimer();
	void	SetDrawSize();

private:
    IUnknown*					m_pDeskBand;
    BOOL						m_fHasFocus;
    CComQIPtr<IInputObjectSite> m_spInputObjectSite;

    CAutoPtr<CVisualStyle>		m_ptrVisualStyle;

    const DateFormat*			m_pDateFormat;

	mutable CString				m_strDrawString[ DRAW_GRAPH_MAX ];
	DrawGraph					m_DrawGraph[ DRAW_GRAPH_MAX ];
	COLORREF					m_TextColor						= RGB( 255, 255, 255 );
	UseSystemInfo				m_UseSystemInfo;

	int							m_CPUUse						= 0;
};

////////////////////////////////////////////////////////////////////////////////