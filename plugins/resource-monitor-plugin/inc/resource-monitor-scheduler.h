/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

/*! \file resource-monitor-scheduler.h
\brief gathers the content of proc file system periodically
and forwards the content to the plugin
 
known limitations:
   *timer configuration has to be improved(e.g. nice level)
*/

#ifndef RESOURCE_MONITOR_SCHEDULER_
#define RESOURCE_MONITOR_SCHEDULER_

#include <map>
#include <vector>
#include <memory>

#include <Poco/Logger.h>
#include <Poco/Activity.h>
#include "Poco/Timer.h"
#include "Poco/Thread.h"
#include "TargetAgentPluginInterface.h"
#include "target_agent_prot_resource_monitor.pb.h"

#include "procFsReader.h"
/**
 * forward declaration of the dependencies
 */
class ResourceInfo;

namespace ResourceMonitor {

class ResourceMonitorPlugin;
}

/**
 * Class containing the logic to trigger the data collection from the proc file system
 */
namespace ResourceMonitor {

class ResourceMonitorScheduler {
    class CPeriodicInfo {
    public:
        int timer_fd;
        unsigned long long wakeups_missed;
    };
public:
    /**
     * ResourceMonitorScheduler constructor.
     * receives a sender object to dispatch data to the plugin
     */
    ResourceMonitorScheduler(
        const TargetAgent::PluginInterface::CMessageDispatcher* senderHandle,
        const TargetAgent::PluginInterface::CTimestampProvider* ts);

    /**
     * ResourceMonitorScheduler destructor.
     */
    virtual ~ResourceMonitorScheduler();

    /**
     * trigger data gathering
     * @param observed vector containing the processes for which also the threads must be monitored
     * @param samplingRate data sampling interval
     * @see -
     * @return boolean specifying whether the function was successful
     */
    bool startMonitoring(void);

    /**
     * stop data gathering
     * @param none
     * @see -
     * @return boolean specifying whether the function was successful
     */
    bool stopMonitoring(void);

    /**
     * Store the configuration form config xml
     * @param observed vector containing the processes for which also the threads must be monitored
     * @param samplingRate data sampling interval
     * @see -
     * @return boolean specifying whether the function was successful
     */
    bool setConfiguration(std::vector<int>& observed, unsigned int samplingRate,
                          std::vector<std::string>& unresolvedNames);

    /**
     * Dispatcher function for messages
     * @param resourceInfoMessage message to be dispatches
     * @param timestamp timestamp provided from plugin for accuracy
     * @see -
     * @return boolean specifying whether the function was successful
     */
    void sendResourceMonitorMessage(const ResourceInfo* resourceInfoMessage,
                                    unsigned long long timestamp);

#if defined(__QNXNTO__)
    /**
     * Poco Timer mechanism
     * @param timer object
     * @see -
     * @return none
     */
    void onTimer(Poco::Timer& timer);
#endif

private:
    TargetAgent::PluginInterface::CMessageDispatcher* sender;
    TargetAgent::PluginInterface::CTimestampProvider* mTs;
    std::vector<int> observedProcesses;
    std::vector<std::string> unresolvedNames;
    Poco::Logger* logger;
    unsigned int samplingRateInMs;
    Poco::Activity<ResourceMonitor::ResourceMonitorScheduler> _activity;
    CPeriodicInfo timerInfo;
    unsigned int numCPU;
    unsigned long long cpuUsageSummedUp;
#if defined(__QNXNTO__)

    Poco::Timer timer;
#endif
private:
    unsigned long long lastIterationTs;
    void collectSystemInfo(ResourceInfo* resourceInfo);
    void collectProcessInfo(ResourceInfo* resourceInfo);
    bool collectContextInfo(sysCpuInfo* cpuInfo);
    bool collectThreadInfo(ProcessInfo* procInfoMessage, int pid);
    void updateObservedProcList(const sstat& entry);
    void runActivity();
    bool configureTimer(void);

    /**
     * overwrite copy constructor and assignment operator
     */
    ResourceMonitorScheduler(const ResourceMonitorScheduler& that);
    const ResourceMonitorScheduler& operator=(
        const ResourceMonitorScheduler& rhs) {
        return *this;
    }

};

}
;

#endif /* RESOURCE_MONITOR_SCHEDULER_ */
