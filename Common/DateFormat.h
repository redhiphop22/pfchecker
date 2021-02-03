#pragma once

////////////////////////////////////////////////////////////////////////////////
// 
const CALTYPE CAL_DEFAULT = (CALTYPE)-1;
const CALTYPE CAL_SCUSTOMDATE = 0;

class DateFormat
{
public:
    DateFormat();

    ATL::CString FormatDateString(const SYSTEMTIME& st) const;

    void Load(LPCTSTR pszFilename, LPCTSTR pszSectionName = NULL);
    bool Save(LPCTSTR pszFilename, LPCTSTR pszSectionName = NULL) const;

    LCID lcId;
    CALID calId;
    CALTYPE calType;
    ATL::CString dateFormat;

private:
    DWORD MakeDateFormatFlags() const;
};

////////////////////////////////////////////////////////////////////////////////
