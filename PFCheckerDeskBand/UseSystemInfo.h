#pragma once

#include <windows.h>
#include <pdh.h>

class UseSystemInfo
{
private:
	PDH_HQUERY				m_hQuery;
	PDH_HCOUNTER			m_hCounter;
	SYSTEM_INFO				m_SysInfo;

public:
	UseSystemInfo();
	~UseSystemInfo();

	bool					Init();
	void					Destroy();
	double					GetCpuUsage() const;
	double					GetMemUsage() const;

};