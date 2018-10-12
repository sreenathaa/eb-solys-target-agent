/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/


#include <iostream>
#include <climits>
#include <windows.h>
#include <psapi.h>
#include "TlHelp32.h"

#include "Poco/Timestamp.h"
#include "Poco/ScopedLock.h"

#include "resource-monitor-scheduler.h"
#include "resource-monitor-plugin.h"
#include "resource-monitor-utils.h"



namespace ResourceMonitor
{

	ResourceMonitorScheduler::ResourceMonitorScheduler(const CMessageDispatcher* const msgSenderHDL, const CTimestampProvider* const  timestampProvider):
mMsgSenderHDL(msgSenderHDL),mTimestampProvider(timestampProvider),resourceMonitoringScheduler(NULL)
{
	logger = &(Poco::Logger::get("TargetAgent.CResourceMonitorPlugin")); // inherits configuration from Target Agent
}

ResourceMonitorScheduler::~ResourceMonitorScheduler()
{
	if(NULL != resourceMonitoringScheduler)
	{
		resourceMonitoringScheduler->stop();
		delete resourceMonitoringScheduler;
	}
}

bool ResourceMonitorScheduler::startMonitoring(UINT samplingRate)
{
	bool result = false;
	if(NULL == resourceMonitoringScheduler)
	{
		logger->debug("ResourceMonitorPlugin start monitoring");
		resourceMonitoringScheduler = new Poco::Timer(0, samplingRate);
		resourceMonitoringScheduler->start(Poco::TimerCallback<ResourceMonitorScheduler>(*this, &ResourceMonitorScheduler::onNextUpdateCycleScheduled),
			Poco::Thread::PRIO_HIGHEST);
		result = true;
	}
	return result;
}

bool ResourceMonitorScheduler::stopMonitoring()
{
	bool result = false;
	if(resourceMonitoringScheduler != NULL)
	{
		resourceMonitoringScheduler->stop();
		delete resourceMonitoringScheduler;
		result = true;
	}
	return result;
}

void ResourceMonitorScheduler::onNextUpdateCycleScheduled(Poco::Timer& timer)
{
	_mutex.lock();
	{
		ResourceInfo* resourceInfoMessage = createNewResourceInfoMessage();
		gatherSystemInfo(resourceInfoMessage);
		gatherProcessListInfo(resourceInfoMessage);
		sendAndDeleteResourceInfoMessage(resourceInfoMessage);
	}
	_mutex.unlock();
}

ResourceInfo* ResourceMonitorScheduler::createNewResourceInfoMessage()
{
	return ResourceInfo::default_instance().New();
}

void ResourceMonitorScheduler::gatherSystemInfo(ResourceInfo* resourceInfo)
{
	CSystemInfo systemInfo;
	SYSTEM_INFO sysinfo;
    GetSystemInfo( &sysinfo );
	systemInfo.update();

	SystemInfo* sysInfoMessage = resourceInfo->mutable_system();
	sysInfoMessage->set_totalvmmembytes(systemInfo.getTotalVirtual());
	sysInfoMessage->set_usedvmmembytes(systemInfo.getTotalVirtual() - systemInfo.getAvailVirtual());
	sysInfoMessage->set_totalpmmembytes(systemInfo.getTotalPhys());
	sysInfoMessage->set_usedpmmembytes(systemInfo.getTotalPhys() - systemInfo.getAvailPhys());
	sysInfoMessage->set_numberofcores(sysinfo.dwNumberOfProcessors);

}

bool ResourceMonitorScheduler::gatherProcessStatistics(const PROCESSENTRY32& procEntry, CProcMetrics* cpuMetrics)
{
	CHAR processName[PROCESS_LENGTH];
	sprintf(processName, "%ls", procEntry.szExeFile);
	FILETIME ftime, fsys, fuser;
	ULARGE_INTEGER  sys, user;
	bool retVal = false;

	HANDLE hdl = OpenProcess(PROCESS_QUERY_INFORMATION ,false,procEntry.th32ProcessID);

	if(hdl != NULL)
	{

		GetProcessTimes(hdl, &ftime, &ftime, &fsys, &fuser);
		memcpy(&sys, &fsys, sizeof(FILETIME));
		memcpy(&user, &fuser, sizeof(FILETIME));

		cpuMetrics->setSysCPU(sys.QuadPart);
		cpuMetrics->setUserCPU(user.QuadPart);
		cpuMetrics->setName(processName);
		cpuMetrics->setPid(procEntry.th32ProcessID);


		PROCESS_MEMORY_COUNTERS_EX info = {0};
		info.cb = sizeof(info);
		GetProcessMemoryInfo(hdl, (PROCESS_MEMORY_COUNTERS*)&info, sizeof(info));

		cpuMetrics->setWorkingSetSize(info.WorkingSetSize);
		cpuMetrics->setPrivateMemory(info.PrivateUsage);

		CloseHandle(hdl);

		retVal = true;
	}
	else
		retVal = false;
	return retVal;

}

void ResourceMonitorScheduler::gatherProcessListInfo(ResourceInfo* resourceInfo)
{

	HANDLE m_hProcessSnap;

	BOOL result = TRUE;
	try
	{

		m_hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS , 0 );

		if( INVALID_HANDLE_VALUE == m_hProcessSnap )
		{
			throw Poco::Exception("INVALID_HANDLE_VALUE");
		}

		PROCESSENTRY32 m_pe32;
		m_pe32.dwSize = sizeof( PROCESSENTRY32 );
		if( !Process32First( m_hProcessSnap, &m_pe32 ) )
		{
			throw Poco::Exception("Information retrieval failed.");
		}
		do
		{
			CProcMetrics cpuMetrics;

			if(gatherProcessStatistics(m_pe32,&cpuMetrics))
			{
				ProcessInfo* procInfoMessage = resourceInfo->add_process();
				writeProcessStatistics(procInfoMessage, &cpuMetrics);
			}
			else
				logger->debug(Poco::format("ResourceMonitorPlugin error gathering metrics for process %d",m_pe32.th32ProcessID));

		}
		while( Process32Next( m_hProcessSnap, &m_pe32 ) );



	}
	catch (Poco::Exception&)
	{
		result = FALSE;
	}
	catch (...)
	{
		result = FALSE;
	}
	try
	{
		CloseHandle( m_hProcessSnap );
	}
	catch (...)
	{
	}


}

void ResourceMonitorScheduler::sendAndDeleteResourceInfoMessage(ResourceInfo* resourceInfo)
{

	UCHAR* payload = new UCHAR[resourceInfo->ByteSize()];
	if (resourceInfo->SerializeToArray( payload, resourceInfo->ByteSize() ) )
		mMsgSenderHDL->sendMessage(Protocol::CommonDefinitions::MSG_TYPE_RESOURCE_MONITOR, resourceInfo->ByteSize(), payload, mTimestampProvider->createTimestamp());
	delete[] payload;



	delete resourceInfo;
}


void ResourceMonitorScheduler::writeProcessStatistics(ProcessInfo* procMessage, const CProcMetrics* procInfo)
{
	procMessage->set_vmusagebytes(procInfo->getWorkingSetSize());
	procMessage->set_pid(procInfo->getPid());
	procMessage->set_name(procInfo->getName());
	procMessage->set_timeinkernelmodems(procInfo->getSysCPU());
	procMessage->set_timeinusermodems(procInfo->getUserCPU());
}

const ResourceMonitorScheduler& ResourceMonitorScheduler::operator=(const ResourceMonitorScheduler& rhs)
{
	return *this;
}

};
