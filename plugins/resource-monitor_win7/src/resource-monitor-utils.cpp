/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

#include "resource-monitor-utils.h"

namespace ResourceMonitor
{


	CSystemInfo::CSystemInfo()
		:
	dwTotalPhys(0),
		dwAvailPhys(0),
		dwTotalVirtual(0),
		dwAvailVirtual(0)
	{
	}

	CSystemInfo::~CSystemInfo()
	{
	}

	void CSystemInfo::update()
	{
		MEMORYSTATUSEX statex = {0};

		statex.dwLength = sizeof (statex);

		GlobalMemoryStatusEx (&statex);


		dwTotalPhys = statex.ullTotalPhys;
		dwAvailPhys = statex.ullAvailPhys;
		dwTotalVirtual = statex.ullTotalVirtual;
		dwAvailVirtual = statex.ullAvailVirtual;
	}

	DWORDLONG  CSystemInfo::getTotalPhys() const
	{
		return dwTotalPhys;
	}

	DWORDLONG  CSystemInfo::getAvailPhys() const
	{
		return dwAvailPhys;
	}

	DWORDLONG  CSystemInfo::getTotalVirtual() const
	{
		return dwTotalVirtual;
	}

	DWORDLONG  CSystemInfo::getAvailVirtual() const
	{
		return dwAvailVirtual;
	}

	CSystemInfo& CSystemInfo::operator=(const CSystemInfo &rhs)
	{
		return *this;
	}


	CProcMetrics::CProcMetrics()
		:
	mSysCPU(0), mUserCPU(0),mPid(0),mName(),mWss(0),mPrivateMem(0)
	{
	}

	CProcMetrics& CProcMetrics::operator=(const CProcMetrics &rhs)
	{
		return *this;
	}

}
