/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

/*! \file resource-monitor-plugin.cpp
 \brief * Resource Monitor Plugin Class: data collector for system resources information at different levels:
 * system wise
 * process level
 * thread level
 
 known limitations:
 *-the members data from resource monitor and scheduler are somehow redundant, design has to be improved,
 *-the e.g. : no copy in the plugin but only in the scheduler
 */

#include <iostream>
#include <fstream>

#include "Poco/NumberParser.h"

#include "resource-monitor-plugin.h"
#include "resource-monitor-scheduler.h"
#include "procFsReader.h"

extern "C" void* createPluginInstance(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider) {
    return new ResourceMonitor::ResourceMonitorPlugin(senderHandle, tsProvider);
}

namespace ResourceMonitor {

using namespace TargetAgent;

const unsigned int ResourceMonitorPlugin::MIN_SAMPLING_RATE = 50;
const unsigned int ResourceMonitorPlugin::MAX_SAMPLING_RATE = 5000;
const unsigned int ResourceMonitorPlugin::DEFAULT_SAMPLING_RATE = 1000;

ResourceMonitorPlugin::ResourceMonitorPlugin(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider) :mMsgSenderHDL(senderHandle),mTimestampProvider(tsProvider),
messageType(Protocol::CommonDefinitions::MSG_TYPE_RESOURCE_MONITOR), scheduler(NULL), logger(0) {
    logger = &(Poco::Logger::get("TargetAgent.RessourceMonitorPlugin")); // inherits configuration from Target Agent
}

ResourceMonitorPlugin::~ResourceMonitorPlugin() {
}

bool ResourceMonitorPlugin::storeSamplingRate(const std::string& samplingAsText,
        unsigned int& samplingAsValue) const {
    bool retVal = false;

    try
    {
        unsigned int rate = Poco::NumberParser::parseUnsigned(samplingAsText);

        if ((MIN_SAMPLING_RATE >= rate) || (MAX_SAMPLING_RATE <= rate))
        {
            logger->warning(
                Poco::format(
                    "Provided sampling rate,%s ms, is not within the recommended range [50ms,5000ms]",
                    samplingAsText));
        }
        samplingAsValue = rate;
        retVal = true;
    }
    catch (Poco::Exception& e)
    {
        logger->log(e);
    }

    return retVal;

}
bool ResourceMonitorPlugin::storeProcById(const std::string& procID,
        std::vector<int>& observed) const {
    int pid = -1;
    bool retVal = false;
    pid = strtol(procID.c_str(), 0, 10);
    if (-1 != pid)
    {
        logger->information(
            Poco::format("Observe threads for the following process %d",
                         pid));
        observed.push_back(pid);
        retVal = true;
    }
    else
    {
        logger->error(Poco::format("Invalid process id %d %d", pid, procID));
    }
    return retVal;
}

bool ResourceMonitorPlugin::storeProcByName(const std::string& procName,
        std::vector<int>& observed) const {
    int pid = -1;
    bool retVal = false;
    int pidArray[PIDS_MAX];
    unsigned int pidArraySize = 0;

    if (getProcIDbyName(procName.c_str(), PIDS_MAX, pidArray, &pidArraySize))
    {

        for (unsigned int i = 0; i < pidArraySize; i++)
        {
            logger->information(Poco::format("Pidof %s is %d", procName, pid));
            observed.push_back(pidArray[i]);
        }

        retVal = true;
    }
    else
    {
        logger->error(Poco::format("Pidof %s not found", procName));
    }
    return retVal;
}

bool ResourceMonitorPlugin::setConfig(
    const std::map<std::string, std::string>& pluginConfiguration) {
    std::vector<int> observed;
    std::vector<std::string> unresolved;
    unsigned int samplingRate = DEFAULT_SAMPLING_RATE;

    for (std::map<std::string, std::string>::const_iterator it=pluginConfiguration.begin(); it!=pluginConfiguration.end(); ++it)
    {
        logger->information(
            Poco::format("key %s value %s", it->first, it->second));
        if (std::string::npos != (it->first).find("procName"))
        {
            /*process id will be resolved at the first iteration*/
            unresolved.push_back(it->second);
        }
        else if (std::string::npos != (it->first).find("samplingRate"))
        {
            if (!storeSamplingRate(it->second, samplingRate))
            {
                logger->error(
                    "Error occurred during computation of sampling rate");
            }
        }
        else if (std::string::npos != (it->first).find("procID"))
        {
            if (!storeProcById(it->second, observed))
            {
                logger->error(
                    "Error occurred during computation of process identifier");
            }
        }
    }
    scheduler = new ResourceMonitorScheduler(mMsgSenderHDL,
                mTimestampProvider);

    scheduler->setConfiguration(observed, samplingRate,unresolved);
    return true;
}

void ResourceMonitorPlugin::onMessageReceived(int payloadLength,
        const unsigned char* payloadBuffer) {

    /*nothing to be done yet*/
}

Protocol::CommonDefinitions::MessageType ResourceMonitorPlugin::MessageType() {
    return messageType;
}

void ResourceMonitorPlugin::onConnectionEstablished() {
    /*nothing to be done yet*/
}

void ResourceMonitorPlugin::onConnectionLost() {
    logger->warning(
        "Connection Lost: trigger metadata dispatch, currently none");
}

bool ResourceMonitorPlugin::startPlugin() {
    logger->information("Start Monitoring");

    return scheduler->startMonitoring();
}

bool ResourceMonitorPlugin::stopPlugin() {
    return scheduler->stopMonitoring();
}

}
;

