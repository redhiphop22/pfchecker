#include "StdAfx.h"
#include "Utils.h"
#include "CalendarGuids.h"

using namespace ATL;

/////////////////////////////////////////////////////////////////////////////
//
const LPCTSTR APP_DIR_NAME = TEXT("PFChecker");

////////////////////////////////////////////////////////////////////////////////
// 
bool IsVistaOrHigher()
{
    const DWORD dwVersion = ::GetVersion();
    const DWORD dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));

    return dwMajorVersion >= 6; // Vista or higher
}

BOOL MakeSettingsFilename(CPath& pathFilename, BOOL bCreateDir)
{
    TCHAR sz[MAX_PATH] = { TEXT('\0') };
    BOOL bRes = ::SHGetSpecialFolderPath(NULL, sz,
        CSIDL_APPDATA, bCreateDir);
    ATLASSERT(bRes);

    if(!bRes)
        return FALSE;

    pathFilename.Combine(sz, APP_DIR_NAME);

    CPath pathBasename(APP_DIR_NAME);
    pathBasename.RenameExtension(TEXT(".ini"));

    pathFilename.Append(pathBasename);

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
