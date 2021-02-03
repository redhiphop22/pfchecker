// dllmain.cpp : Implementation of DllMain.

#include "StdAfx.h"
#include "DllMain.h"

CCalendarDeskBandModule _AtlModule;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	hInstance;
	return _AtlModule.DllMain(dwReason, lpReserved); 
}
