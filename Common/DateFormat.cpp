////////////////////////////////////////////////////////////////////////////////
// DateFormat implementation

#include "StdAfx.h"
#include "DateFormat.h"

using namespace ATL;

////////////////////////////////////////////////////////////////////////////////
// 
const LPCTSTR SETTINGS_SECTION_DATEFORMAT = TEXT("DateFormat");

const LPCTSTR SETTINGS_KEY_LCID    = TEXT("Locale");
const LPCTSTR SETTINGS_KEY_CALID   = TEXT("Calendar");
const LPCTSTR SETTINGS_KEY_CALTYPE = TEXT("CalendarType");
const LPCTSTR SETTINGS_KEY_FORMAT  = TEXT("DateFormat");

////////////////////////////////////////////////////////////////////////////////
// 
DateFormat::DateFormat()
{
    lcId = ::GetUserDefaultLCID();
    ::GetLocaleInfo(lcId,
        LOCALE_ICALENDARTYPE | LOCALE_RETURN_NUMBER,
        reinterpret_cast<LPTSTR>(&calId), sizeof(CALID));

    calType = CAL_SLONGDATE;
}

CString DateFormat::FormatDateString(const SYSTEMTIME& st) const
{
    const DWORD dwFlags = MakeDateFormatFlags();

    // GetDateFormat doesn't support custom format for alt calendars.
    const bool bCustomFormat =
        !(dwFlags & DATE_USE_ALT_CALENDAR) && (calType == CAL_SCUSTOMDATE);

    LPCTSTR pszFormat = NULL;
    if(bCustomFormat)
        pszFormat = dateFormat;

    const int cchDate = ::GetDateFormat(lcId, dwFlags, &st, pszFormat, NULL, 0);
    ATLASSERT(cchDate > 0);

    CString date;
    if(cchDate > 0)
    {
        ::GetDateFormat(lcId, dwFlags, &st, pszFormat,
            date.GetBuffer(cchDate), cchDate);
        date.ReleaseBuffer();
    }

    return date;
}

void DateFormat::Load(LPCTSTR pszFilename, LPCTSTR pszSectionName)
{
    if(!pszSectionName) pszSectionName = SETTINGS_SECTION_DATEFORMAT;

    lcId = ::GetPrivateProfileInt(
        pszSectionName, SETTINGS_KEY_LCID,
        lcId, pszFilename);

    calId = ::GetPrivateProfileInt(
        pszSectionName, SETTINGS_KEY_CALID,
        calId, pszFilename);

    calType = ::GetPrivateProfileInt(
        pszSectionName, SETTINGS_KEY_CALTYPE,
        calType, pszFilename);

    CString dateFormatNew;
    ::GetPrivateProfileString(
        pszSectionName, SETTINGS_KEY_FORMAT,
        dateFormat, dateFormatNew.GetBuffer(512), 512, pszFilename);

    if(dateFormat != dateFormatNew)
        dateFormat = dateFormatNew;
}

bool DateFormat::Save(LPCTSTR pszFilename, LPCTSTR pszSectionName) const
{
    if(!pszSectionName) pszSectionName = SETTINGS_SECTION_DATEFORMAT;

    TCHAR szVal[MAX_PATH] = { TEXT('\0') };
    BOOL bWrite = FALSE;

    _itot(lcId, szVal, 10);
    bWrite = ::WritePrivateProfileString(
        pszSectionName, SETTINGS_KEY_LCID,
        szVal, pszFilename);
    ATLASSERT(bWrite);

    if(!bWrite)
        return false;

    _itot(calId, szVal, 10);
    bWrite = ::WritePrivateProfileString(
        pszSectionName, SETTINGS_KEY_CALID,
        szVal, pszFilename);
    ATLASSERT(bWrite);

    if(!bWrite)
        return false;

    _itot(calType, szVal, 10);
    bWrite = ::WritePrivateProfileString(
        pszSectionName, SETTINGS_KEY_CALTYPE,
        szVal, pszFilename);
    ATLASSERT(bWrite);

    if(!bWrite)
        return false;

    bWrite = ::WritePrivateProfileString(
        pszSectionName, SETTINGS_KEY_FORMAT,
        dateFormat, pszFilename);
    ATLASSERT(bWrite);

    if(!bWrite)
        return false;

    return true;
}

DWORD DateFormat::MakeDateFormatFlags() const
{
    DWORD dwFlags = (calId == CAL_GREGORIAN ? 0 : DATE_USE_ALT_CALENDAR);
    switch(calType)
    {
    case CAL_SLONGDATE:
        dwFlags |= DATE_LONGDATE;
        break;
    case CAL_SSHORTDATE:
        dwFlags |= DATE_SHORTDATE;
        break;
    }

    return dwFlags;
}

////////////////////////////////////////////////////////////////////////////////
