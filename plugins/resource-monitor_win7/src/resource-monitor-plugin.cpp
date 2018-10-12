/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/


#include <iostream>
#include "resource-monitor-plugin.h"
#include "resource-monitor-scheduler.h"
#include "Poco/Timestamp.h"

#define WIDE_STRING_LENGTH 100

extern "C" void* createPluginInstance(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider)
{
    return new ResourceMonitor::ResourceMonitorPlugin(senderHandle, tsProvider);
}

namespace ResourceMonitor {

using namespace TargetAgent;

ResourceMonitorPlugin::ResourceMonitorPlugin(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider)
        : mMsgSenderHDL(senderHandle),
        mTimestampProvider(tsProvider),
        messageType(Protocol::CommonDefinitions::MSG_TYPE_RESOURCE_MONITOR),
        scheduler(NULL)
{
    samplingRate = DEFAULT_RATE;
    logger = &(Poco::Logger::get("TargetAgent.CResourceMonitorPlugin")); // inherits configuration from Target Agent
}

ResourceMonitorPlugin::~ResourceMonitorPlugin()
{
}

void ResourceMonitorPlugin::onMessageReceived(INT payloadLength, const UCHAR* payloadBuffer)
{
}

bool ResourceMonitorPlugin::setConfig(const std::map<std::string, std::string>& pluginConfiguration)
{

    bool result = TRUE;
    typedef std::map<std::string, std::string>::const_iterator it_type;
    for (it_type iter = pluginConfiguration.begin(); iter != pluginConfiguration.end(); ++iter)
    {

        if (std::string::npos != (iter->first).find("samplingRate"))
        {
            UINT rate = Poco::NumberParser::parseUnsigned(iter->second.c_str());
            if( MIN_RATE <= rate && rate <= MAX_RATE )
            {
                samplingRate = rate;
            }
            else
            {
                result = FALSE;
            }
        }
    }
    return result;
}

void ResourceMonitorPlugin::sendResourceMonitorMessage(const ResourceInfo* resourceInfoMessage)
{
    UCHAR* payload = new UCHAR[resourceInfoMessage->ByteSize()];
    if (resourceInfoMessage->SerializeToArray( payload, resourceInfoMessage->ByteSize() ) )
        mMsgSenderHDL->sendMessage(messageType, resourceInfoMessage->ByteSize(), payload, mTimestampProvider->createTimestamp());
    delete[] payload;
}

Protocol::CommonDefinitions::MessageType ResourceMonitorPlugin::MessageType()
{
    return messageType;
}

void ResourceMonitorPlugin::onConnectionEstablished()
{
}

void ResourceMonitorPlugin::onConnectionLost()
{
}

bool ResourceMonitorPlugin::startPlugin()
{
    logger->debug("ResourceMonitorPlugin start plugin");

    scheduler = new ResourceMonitorScheduler(mMsgSenderHDL,mTimestampProvider);
    return scheduler->startMonitoring( samplingRate);
}

bool ResourceMonitorPlugin::stopPlugin()
{
    logger->debug("ResourceMonitorPlugin stop plugin");
    return scheduler->stopMonitoring();
}

};
