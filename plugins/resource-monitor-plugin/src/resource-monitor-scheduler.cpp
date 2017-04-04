/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

/*! \file resource-monitor-scheduler.cpp
 \brief gathers the content of proc file system periodically
 and forwards the content to the plugin
 
 known limitations:
 *timer configuration has to be improved(e.g. nice level)
 */
#if not defined(__QNXNTO__)
#include <sys/timerfd.h>
#endif
#include <unistd.h>
#include "Poco/DateTime.h"

#include "procFsReader.h"
#include "resource-monitor-scheduler.h"
#include "resource-monitor-plugin.h"
#include "Poco/Environment.h"

namespace ResourceMonitor {
using namespace TargetAgent;
ResourceMonitorScheduler::ResourceMonitorScheduler(
    const TargetAgent::PluginInterface::CMessageDispatcher* senderHandle,
    const TargetAgent::PluginInterface::CTimestampProvider* ts) :
        sender(
            const_cast<TargetAgent::PluginInterface::CMessageDispatcher*>(senderHandle)), mTs(
                const_cast<TargetAgent::PluginInterface::CTimestampProvider*>(ts)), observedProcesses(), unresolvedNames(), logger(
                    0), samplingRateInMs(1000), _activity(this,
                                                          &ResourceMonitorScheduler::runActivity), timerInfo(), numCPU(
                                                      Poco::Environment::processorCount()), cpuUsageSummedUp(0),lastIterationTs(0) {
    logger = &(Poco::Logger::get("TargetAgent.RessourceMonitorPlugin")); // inherits configuration from Target Agent
}

ResourceMonitorScheduler::~ResourceMonitorScheduler() {
    std::map<int, int>::iterator it;

    _activity.stop(); // request stop
    _activity.wait(); // wait until activity actually stops
}

bool ResourceMonitorScheduler::startMonitoring() {


    logger->information("ResourceMonitorScheduler::startMonitoring");

    _activity.start();

    return true;
}

bool ResourceMonitorScheduler::stopMonitoring() {

    _activity.stop(); // request stop
    _activity.wait(); // wait until activity actually stops

    return true;
}

#if defined(__QNXNTO__)
void ResourceMonitorScheduler::onTimer(Poco::Timer& timer) {
    ResourceInfo resourceInfoMessage;

    collectSystemInfo(&resourceInfoMessage);
    collectProcessInfo(&resourceInfoMessage);
    sendResourceMonitorMessage(&resourceInfoMessage,
                               mTs->createTimestamp());
}
#endif

bool ResourceMonitorScheduler::configureTimer(void) {

    bool retVal = true;
#if defined(__QNXNTO__)

    timer.setPeriodicInterval(samplingRateInMs);
    timer.setStartInterval(samplingRateInMs);
#else

    int ret;
    unsigned long int ns;
    unsigned long int sec;
    int fd;
    struct itimerspec itval;

#ifdef _POSIX_THREADS

    if (0 != pthread_setname_np(pthread_self(), "PResMonSkd"))
    {
        logger->error("failed to set the thread name for PResMonSkd");
    }
#endif
    fd = timerfd_create(CLOCK_MONOTONIC, 0);
    timerInfo.wakeups_missed = 0;
    timerInfo.timer_fd = fd;
    if (fd == -1)
    {
        logger->error("Set time file descriptor");
        retVal = false;
    }

    sec = samplingRateInMs / 1000;
    ns = (samplingRateInMs - (sec * 1000)) * 1000000;

    itval.it_interval.tv_sec = sec;
    itval.it_interval.tv_nsec = ns;
    itval.it_value.tv_sec = sec;
    itval.it_value.tv_nsec = ns;
    ret = timerfd_settime(fd, 0, &itval, NULL);

    if (ret == -1)
    {
        logger->error("Set time file descriptor");
        retVal = false;
    }

#endif

    return retVal;

}
void ResourceMonitorScheduler::runActivity() {

    if (!configureTimer())
    {
        logger->error("Fail to configure: Resource Monitor will not start");
    }
    else
    {

        while (!_activity.isStopped())
        {
#if not defined(__QNXNTO__)
            unsigned long long currentIterationTs = mTs->createTimestamp();
            int ret = 0;
            unsigned long long missedIteration;

            logger->information(
                Poco::format("Triggered after %Lu",
                             (currentIterationTs - lastIterationTs)));
            lastIterationTs = currentIterationTs;

            ret = read(timerInfo.timer_fd, &missedIteration,
                       sizeof(missedIteration));
            if (ret == -1)
            {
                logger->information(
                    "Error occurred while waiting for the next iteration");
            }

            if (missedIteration > 0)
            {
                timerInfo.wakeups_missed += (missedIteration - 1);
                logger->warning(
                    Poco::format("Missed %Lu", timerInfo.wakeups_missed));
            }

            ResourceInfo resourceInfoMessage;

            collectSystemInfo(&resourceInfoMessage);
            collectProcessInfo(&resourceInfoMessage);

            sendResourceMonitorMessage(&resourceInfoMessage,
                                       mTs->createTimestamp());
#else

            timer.start(Poco::TimerCallback<ResourceMonitorScheduler>(*this, &ResourceMonitorScheduler::onTimer));
#endif

        }
    }
}

void ResourceMonitorScheduler::collectSystemInfo(ResourceInfo* resourceInfo) {

    sysMemInfo memInfo = { 0 };
    bool result = readMeminfo(&memInfo);
    if (result)
    {
        SystemInfo* sysInfoMessage = resourceInfo->mutable_system();
        sysInfoMessage->set_numberofcores(numCPU);
        sysInfoMessage->set_totalvmmembytes(memInfo.totalVirtualMem);
        sysInfoMessage->set_usedvmmembytes(memInfo.virtualMemUsed);
        sysInfoMessage->set_totalpmmembytes(memInfo.totalPhysMem);
        sysInfoMessage->set_usedpmmembytes(memInfo.physMemUsed);
    }
    else
        logger->error("Failed to read memory information");

}

bool ResourceMonitorScheduler::collectContextInfo(sysCpuInfo* cpuInfo) {

    bool result = false;

    result = readCpuInfo(cpuInfo);

    if (!result)
    {
        logger->error("Failed to read memory information");
    }
    else
        result = true;

    return result;

}

bool ResourceMonitorScheduler::setConfiguration(std::vector<int>& observed,
        unsigned int samplingRate, std::vector<std::string>& unresolvedNames) {
    bool status = true;

    observedProcesses = observed;

    this->samplingRateInMs = samplingRate;

    this->unresolvedNames = unresolvedNames;

    return status;
}

bool ResourceMonitorScheduler::collectThreadInfo(ProcessInfo* procInfoMessage,
        int pid) {

    bool readingValid = true;
    sstat statInfo = { 0 };
    sstatm statmInfo = { 0 };
    descriptor thrEntryInfo = { 0 };
    char taskRootFolder[NAME_MAX];

    UPDATE_TASK_ROOT(taskRootFolder, pid);

    thrEntryInfo.dir = opendir(taskRootFolder);
    thrEntryInfo.dp = NULL;
    thrEntryInfo.pid = pid;

    do
    {
        readingValid = readProcFs(&thrEntryInfo, &statInfo, &statmInfo);

        if (readingValid)
        {

            ThreadInfo* threadInfoMessage = procInfoMessage->add_threads();
            threadInfoMessage->set_tid(statInfo.pid);
            threadInfoMessage->set_name(std::string(statInfo.fullName));
            threadInfoMessage->set_timeinusermodems(statInfo.new_utime);
            threadInfoMessage->set_timeinkernelmodems(statInfo.new_stime);
            threadInfoMessage->set_state(std::string(1, statInfo.state));
            threadInfoMessage->set_vmbytes(statmInfo.Rss );
        }
    }
    while (0 != thrEntryInfo.dp)
        ;

    closedir(thrEntryInfo.dir);
    return readingValid;

}

void ResourceMonitorScheduler::updateObservedProcList(const sstat& entry) {

    for (std::vector<std::string>::iterator it = unresolvedNames.begin();
         it != unresolvedNames.end(); ++it)
    {
        bool doesNameMatch =
            (0 == std::string(entry.fullName).compare(*it)) ? true : false;
        if (doesNameMatch)
        {

            bool instanceNotAlreadyThere =
                (observedProcesses.end()
                 == find(observedProcesses.begin(),
                         observedProcesses.end(), entry.pid)) ?
                true : false;

            if (instanceNotAlreadyThere)
            {
                observedProcesses.push_back(entry.pid);
                logger->error(
                    Poco::format("Pid for %s resolved %d", *it, entry.pid));
            }
        }
    }
}

void ResourceMonitorScheduler::collectProcessInfo(ResourceInfo* resourceInfo) {

    descriptor entryInfo = { 0 };
    sstat statInfo = { 0 };
    sstatm statmInfo = { 0 };
    cpuUsageSummedUp = 0;
    entryInfo.dir = opendir(PROC_FS_ROOT);
    entryInfo.dp = NULL;
    entryInfo.pid = 0;

    bool readingValid = false;

    do
    {
        readingValid = readProcFs(&entryInfo, &statInfo, &statmInfo);

        if (readingValid)
        {

            ProcessInfo* procInfoMessage = resourceInfo->add_process();
            procInfoMessage->set_pid(statInfo.pid);
            procInfoMessage->set_name(std::string(statInfo.fullName));
            procInfoMessage->set_timeinusermodems(statInfo.new_utime);
            procInfoMessage->set_timeinkernelmodems(statInfo.new_stime);
            procInfoMessage->set_vmusagebytes(
                statmInfo.Rss);
            cpuUsageSummedUp += statInfo.new_stime + statInfo.new_utime;

            updateObservedProcList(statInfo);
            if (std::find(observedProcesses.begin(), observedProcesses.end(),
                          statInfo.pid) != observedProcesses.end())
            {

                readingValid = collectThreadInfo(procInfoMessage, statInfo.pid);

            }
        }
    }
    while (0 != entryInfo.dp)
        ;

    logger->information("End of iteration");

    closedir(entryInfo.dir);
}

void ResourceMonitorScheduler::sendResourceMonitorMessage(
    const ResourceInfo* resourceInfoMessage, unsigned long long timestamp) {

    unsigned char payload[resourceInfoMessage->ByteSize()];

    sysCpuInfo cpuInfo = { 0 };
    collectContextInfo(&cpuInfo);

    unsigned long long cpuSystem = (cpuInfo.userModeTime
                                    + cpuInfo.niceUserModeTime + cpuInfo.kernelModeTime) / numCPU;
    std::map < std::string, std::string > contextInfo;
    contextInfo.insert(
        std::make_pair<std::string, std::string>(
            std::string("cpu.system.raw"),
            Poco::format("%Lu", cpuSystem)));

    if (resourceInfoMessage->SerializeToArray((void*) payload,
            resourceInfoMessage->ByteSize()))
    {
        sender->sendMessage(
            Protocol::CommonDefinitions::MSG_TYPE_RESOURCE_MONITOR,
            resourceInfoMessage->ByteSize(), payload, timestamp,
            &contextInfo);
    }

}
}
;

