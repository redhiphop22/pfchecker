// Calendar.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "Calendar.h"

/////////////////////////////////////////////////////////////////////////////
//
CCalendarModule _AtlModule;

/////////////////////////////////////////////////////////////////////////////
//
extern "C" int WINAPI _tWinMain(
    HINSTANCE /*hInstance*/,
    HINSTANCE /*hPrevInstance*/,
    LPTSTR /*lpCmdLine*/,
    int nShowCmd)
{
    return _AtlModule.WinMain(nShowCmd);
}