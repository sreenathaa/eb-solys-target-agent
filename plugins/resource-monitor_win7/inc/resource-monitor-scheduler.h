/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef RESOURCE_MONITOR_SCHEDULER_
#define RESOURCE_MONITOR_SCHEDULER_

#include <Poco/Timer.h>
#include <Poco/Mutex.h>
#include <Poco/Logger.h>

#include "target_agent_prot_resource_monitor.pb.h"
#include "TlHelp32.h"


namespace TargetAgent {
namespace PluginInterface {
class CMessageDispatcher;
class CTimestampProvider;
}
}


namespace ResourceMonitor
{
class CSystemInfo;
class ResourceMonitorPlugin;
class CProcMetrics;
class ResourceMonitorScheduler
{
public:
    ResourceMonitorScheduler(const TargetAgent::PluginInterface::CMessageDispatcher* const mMsgSenderHDL, const TargetAgent::PluginInterface::CTimestampProvider* const  mTimestampProvider);
    virtual ~ResourceMonitorScheduler();

    /**
     * Creates a timer that starts monitoring based on the sample rate
     * @param samplingRate
     * @return false all the time,
     */
    bool startMonitoring(UINT samplingRate);

    /**
     * Stops the scheduler and deletes used resources
     * @param -
     * @return if it stopped the scheduler or not
     */
    bool stopMonitoring();

    /**
     * Collects and sends the information about the current state of the system.
     * @param timer is unused
     * @return -
     */
    void onNextUpdateCycleScheduled(Poco::Timer& timer);

private:
    Poco::Mutex _mutex;
    const TargetAgent::PluginInterface::CMessageDispatcher* const mMsgSenderHDL;
    const TargetAgent::PluginInterface::CTimestampProvider* const  mTimestampProvider;
    Poco::Timer* resourceMonitoringScheduler;
    Poco::Logger* logger;


    /**
     * -
     * @param -
     * @return An empty instance of a message
     */
    ResourceInfo* createNewResourceInfoMessage();

    /**
     * -
     * @param resourceInfo
     * @return -
     */
    void gatherSystemInfo(ResourceInfo* resourceInfo);

    /**
     * -
     * @param resourceInfo
     * @return -
     */
    void gatherProcessListInfo(ResourceInfo* resourceInfo);


    /**
     * -
     * @param resourceInfo
     * @return -
     */
    void sendAndDeleteResourceInfoMessage(ResourceInfo* resourceInfo);

    /**
     * Collects measurements of a specific process
     * @param pid process entry
     * @param output parameter metrics
     * @return - succes result
     */
    bool gatherProcessStatistics(const PROCESSENTRY32& procEntry, CProcMetrics* metrics);


    /**
     * Adds information contained in procInfo to procMessage.
     * Calls writeProcessMemoryInfo
     * @param procMessage
     * @param procInfo
     * @return -
     */
    void writeProcessStatistics(ProcessInfo* procMessage, const CProcMetrics* procInfo);

    ResourceMonitorScheduler(const ResourceMonitorScheduler& that);
    const ResourceMonitorScheduler& operator=(const ResourceMonitorScheduler& rhs);
private:
    static const unsigned int PROCESS_LENGTH = 50;
    static const unsigned int WIDE_STRING_LENGTH  = 100;
};

};

#endif /* RESOURCE_MONITOR_SCHEDULER_ */
