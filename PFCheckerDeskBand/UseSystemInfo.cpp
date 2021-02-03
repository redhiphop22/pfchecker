#include "stdafx.h"
#include "UseSystemInfo.h"

#pragma comment(lib,"pdh.lib")

UseSystemInfo::UseSystemInfo()
: m_hQuery( 0 )
, m_hCounter( 0 )
{
}

UseSystemInfo::~UseSystemInfo()
{
	Destroy(); 
}

bool UseSystemInfo::Init()
{
	PDH_STATUS      status = PdhOpenQuery( 0, 0, &m_hQuery );
	if( status != ERROR_SUCCESS )			return false;

	status = PdhAddCounter( m_hQuery, L"\\Processor(_TOTAL)\\% Processor Time", 0, &m_hCounter );
	if( status != ERROR_SUCCESS )			return false;
	status = PdhCollectQueryData( m_hQuery );
	if( status != ERROR_SUCCESS )			return false;

	Sleep(1000 );

	return true;
}

void  UseSystemInfo::Destroy()
{
	if( m_hQuery )
		PdhCloseQuery( m_hQuery );
	m_hQuery = 0;
}

double UseSystemInfo::GetCpuUsage() const
{
	PDH_STATUS      status = PdhCollectQueryData( m_hQuery );
	if( status != ERROR_SUCCESS )		return 0.f;

	PDH_FMT_COUNTERVALUE    value;
	status = PdhGetFormattedCounterValue( m_hCounter, PDH_FMT_DOUBLE, 0, &value );
	if( status != ERROR_SUCCESS )		return 0.f;

	return value.doubleValue;
}

double UseSystemInfo::GetMemUsage() const
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof( MEMORYSTATUSEX );
	GlobalMemoryStatusEx( &memInfo );	
	
	DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
	DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;

	return ((static_cast<double>(memInfo.ullTotalPhys - memInfo.ullAvailPhys)) / memInfo.ullTotalPhys) * 100.f;
}
