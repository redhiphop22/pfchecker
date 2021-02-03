#pragma once

#include "CalendarWindow.h"

class CCalendarModule :
    public CAtlExeModuleT< CCalendarModule >
{
public :
    HRESULT PreMessageLoop(int nShowCmd)
    {
        HRESULT hr = CAtlExeModuleT<CCalendarModule>::PreMessageLoop(nShowCmd);

        if(FAILED(hr))
            return hr;

        INITCOMMONCONTROLSEX icc = { 0 };
        icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icc.dwICC = ICC_DATE_CLASSES | ICC_BAR_CLASSES;

        if(!::InitCommonControlsEx(&icc))
            return E_FAIL;

        m_wndCalendar.Create(NULL);
        ATLASSERT(m_wndCalendar.IsWindow());

        if(!m_wndCalendar.IsWindow())
            return E_FAIL;

        return S_OK;
    }

private:
    // Main window
    CCalendarWindow m_wndCalendar;
};

/////////////////////////////////////////////////////////////////////////////