#include "StdAfx.h"
#include "PFCheckerWindow.h"
#include "VisualStyle.h"
#include "Utils.h"

////////////////////////////////////////////////////////////////////////////////
// 
const UINT_PTR UPDATE_TIMER_ID = 1;

////////////////////////////////////////////////////////////////////////////////
// 
class SelectGdiObject
{
public:
    SelectGdiObject(HDC hDC, HGDIOBJ hGdiObj) :
        m_hDC(hDC),
        m_hGdiObj(hGdiObj),
        m_hSaveGdiObj(::SelectObject(hDC, hGdiObj))
    {
        ATLASSERT(m_hSaveGdiObj && m_hSaveGdiObj != HGDI_ERROR);
    }

    ~SelectGdiObject()
    {
        if(m_hSaveGdiObj && m_hSaveGdiObj != HGDI_ERROR)
            ::SelectObject(m_hDC, m_hSaveGdiObj);
    }

    operator HGDIOBJ() const { return m_hGdiObj; }

private:
    HDC m_hDC;
    HGDIOBJ m_hGdiObj;
    HGDIOBJ m_hSaveGdiObj;

private:
    // not copyable
    SelectGdiObject(const SelectGdiObject&);
    SelectGdiObject& operator=(const SelectGdiObject&);
};

class DoubleBufferPaint
{
public:
    DoubleBufferPaint(HWND hWnd, DWORD dwRop = SRCCOPY) : m_hWnd(hWnd), m_dwRop(dwRop)
    {
        m_hdc = ::BeginPaint(m_hWnd, &m_ps);

        int x, y, cx, cy;
        InitDims(&x, &y, &cx, &cy);

        m_hdcMem = ::CreateCompatibleDC(m_hdc);
        m_hbmMem = ::CreateCompatibleBitmap(m_hdc, cx, cy);
        m_hbmSave = ::SelectObject(m_hdcMem, m_hbmMem);
        ::SetWindowOrgEx(m_hdcMem, x, y, NULL);
    }

    ~DoubleBufferPaint()
    {
        int x, y, cx, cy;
        InitDims(&x, &y, &cx, &cy);

        ::BitBlt(m_hdc, x, y, cx, cy, m_hdcMem, x, y, m_dwRop);

        ::SelectObject(m_hdcMem, m_hbmSave);
        ::DeleteObject(m_hbmMem);
        ::DeleteDC(m_hdcMem);

        ::EndPaint(m_hWnd, &m_ps);
    }

    HDC GetDC() const { return m_hdcMem; }
    const RECT& GetPaintRect() const { return m_ps.rcPaint; }

private:
    void InitDims(int* px, int* py, int* pcx, int* pcy) const
    {
        *px = m_ps.rcPaint.left;
        *py = m_ps.rcPaint.top;
        *pcx = m_ps.rcPaint.right - m_ps.rcPaint.left;
        *pcy = m_ps.rcPaint.bottom - m_ps.rcPaint.top;
    }

private:
    HWND m_hWnd;
    DWORD m_dwRop;

    HDC m_hdc;
    HDC m_hdcMem;
    HGDIOBJ m_hbmMem;
    HGDIOBJ m_hbmSave;
    PAINTSTRUCT m_ps;

private:
    // not copyable
    DoubleBufferPaint(const DoubleBufferPaint&);
    DoubleBufferPaint& operator=(const DoubleBufferPaint&);
};

////////////////////////////////////////////////////////////////////////////////
// CCalendarWindow
CCalendarWindow::CCalendarWindow() :
    m_pDeskBand(NULL),
    m_fHasFocus(FALSE),
    m_ptrVisualStyle(CVisualStyle::Create()),
    m_pDateFormat(NULL)
{
}

CCalendarWindow::~CCalendarWindow()
{
}

////////////////////////////////////////////////////////////////////////////////
// 

BOOL CCalendarWindow::Create(
    HWND hwndParent,
    LPUNKNOWN pDeskBand,
    LPUNKNOWN pInputObjectSite,
    const DateFormat* pDateFormat)
{
    if(!__super::Create(hwndParent))
        return FALSE;

    ATLASSERT(pDeskBand);
    m_pDeskBand = pDeskBand;

    ATLASSERT(pInputObjectSite);
    m_spInputObjectSite = pInputObjectSite;

    ATLASSERT(pDateFormat);
    m_pDateFormat = pDateFormat;

    const UINT_PTR timerId = SetUpdateTimer();
    ATLASSERT(timerId != 0);

    if(!timerId)
        return FALSE;

	if( false == m_UseSystemInfo.Init() )
	{
		return FALSE;
	}

	m_DrawGraph[ DRAW_GRAPH_CPU ].Init( RGB(0, 162, 232) );
	m_DrawGraph[ DRAW_GRAPH_MEM ].Init( RGB(255,137,19) );

	SetDrawSize();

    UpdateCaption();

    return TRUE;
}

POINTL CCalendarWindow::CalcMinimalSize() const
{
    POINTL pt = {
        ::GetSystemMetrics(SM_CXMIN),
        ::GetSystemMetrics(SM_CYMIN)
    };

    return pt;
}

POINTL CCalendarWindow::CalcIdealSize() const
{
    if(!IsWindow()) return CalcMinimalSize();

	CString strTemp;
	strTemp.Format( L"MMMMMM" );

    HDC hic = ::CreateIC(_T("DISPLAY"), NULL, NULL, NULL);

    SIZE size = { 0 };
    {
        SelectGdiObject gdiFont(hic, m_ptrVisualStyle->GetFont());
    
        const BOOL bRes = ::GetTextExtentPoint32(hic, strTemp, strTemp.GetLength(), &size);
        ATLASSERT(bRes);
    }

    ::DeleteDC(hic);

    const POINTL pt = { size.cx*2, size.cy };

    return pt;
}

BOOL CCalendarWindow::HasFocus() const
{
    return m_fHasFocus;
}

void CCalendarWindow::UpdateCaption()
{
	ATLASSERT( IsWindow() );

	int Temp = static_cast<int>(m_UseSystemInfo.GetCpuUsage() );
	m_DrawGraph[ DRAW_GRAPH_CPU ].AddValue( Temp );
	m_strDrawString[ DRAW_GRAPH_CPU ].Format( L"CPU:%3d%%", Temp );

	Temp = static_cast<int>(m_UseSystemInfo.GetMemUsage() );
	m_DrawGraph[ DRAW_GRAPH_MEM ].AddValue( Temp );
	m_strDrawString[ DRAW_GRAPH_MEM ].Format( L"MEM:%3d%%", Temp );

	Invalidate();
}

////////////////////////////////////////////////////////////////////////////////
// Message handlers

LRESULT CCalendarWindow::OnFocus(
    UINT uMsg,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    m_fHasFocus = (uMsg == WM_SETFOCUS);

    if(m_spInputObjectSite)
        m_spInputObjectSite->OnFocusChangeIS(m_pDeskBand, m_fHasFocus);

    return 0L;
}

LRESULT CCalendarWindow::OnDestroy(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    KillTimer(UPDATE_TIMER_ID);

    return 0;
}

LRESULT CCalendarWindow::OnEraseBackground(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    return 1;
}

LRESULT CCalendarWindow::OnPaint(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    DoubleBufferPaint dbuffPaint(m_hWnd);
    
    Paint(dbuffPaint.GetDC(), dbuffPaint.GetPaintRect());

    return 0;
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
    }

    return 0;
}

LRESULT CCalendarWindow::OnTimer(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    UpdateCaption();

    return 0;
}

LRESULT CCalendarWindow::OnTimeChange(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL& bHandled)
{
    return OnTimer(uMsg, wParam, lParam, bHandled);
}

LRESULT CCalendarWindow::OnThemeChanged(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
    // re-create theme style
    m_ptrVisualStyle.Free();
    m_ptrVisualStyle.Attach(CVisualStyle::Create());

    return 0;
}

LRESULT CCalendarWindow::OnSize(
    UINT /*uMsg*/,
    WPARAM /*wParam*/,
    LPARAM /*lParam*/,
    BOOL& /*bHandled*/)
{
	SetDrawSize();

    return 0;
}

void CCalendarWindow::SetDrawSize()
{
	 // re-create theme style
	RECT TempRect;
    GetClientRect( &TempRect );

	RECT rcRect;
	rcRect.top		= TempRect.top;
	rcRect.bottom	= TempRect.bottom;

	float fWith = static_cast<float>(TempRect.right-TempRect.left) / DRAW_GRAPH_MAX;
	for( int i = 0 ; i < DRAW_GRAPH_MAX ; i++ )
	{
		rcRect.left		= static_cast<LONG>(fWith * i);
		rcRect.right	= static_cast<LONG>(fWith * (i+1));
		m_DrawGraph[i].OnSize( rcRect );
	}
}

void CCalendarWindow::Paint(HDC hdc, const RECT& rcPaint) const
{
   m_ptrVisualStyle->DrawBackground(m_hWnd, hdc, rcPaint);
	
   for( int i = 0 ; i < DRAW_GRAPH_MAX ; i++ )
   {
	   m_DrawGraph[ i ].OnDraw( hdc );
   }

    SelectGdiObject gdiFont(hdc, m_ptrVisualStyle->GetFont());
    ::SetBkMode(hdc, TRANSPARENT);

    ::SetTextColor(hdc, m_TextColor );

	for( int i = 0 ; i < DRAW_GRAPH_MAX ; i++ )
	{
		::DrawText(hdc, m_strDrawString[i], m_strDrawString[i].GetLength(), const_cast<LPRECT>(&m_DrawGraph[ i ].GetRect()), DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	}
}

UINT_PTR CCalendarWindow::SetUpdateTimer()
{
    // Wake up every 10 seconds.
    UINT_PTR nTimerId = SetTimer(UPDATE_TIMER_ID, 1000);

    ATLASSERT(nTimerId != 0);

    return nTimerId;
}

////////////////////////////////////////////////////////////////////////////////
