/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include <iostream>

#include "Poco/NumberParser.h"
#include "../inc/linux_app_stats_plugin.h"
#include <google/protobuf/text_format.h>

extern "C" void* createPluginInstance(
        const CMessageDispatcher* const senderHandle,
        const CTimestampProvider* tsProvider) {
    return new LinuxAppStatsPlugin::CLinuxAppStatsPlugin(senderHandle,
            tsProvider);
}

namespace LinuxAppStatsPlugin {

using namespace TargetAgent;

CLinuxAppStatsPlugin::CLinuxAppStatsPlugin(
    const CMessageDispatcher* const senderHandle,
    const CTimestampProvider* tsProvider) :
        mMsgSenderHDL(senderHandle), mTimestampProvider(tsProvider), mMsgTypeLinuxAppStats(
            Protocol::CommonDefinitions::MSG_TYPE_LINUX_APP_STATS_PLUGIN), mLogger(
        0),  mQuery(),mCollector(0) {
    mLogger = &(Poco::Logger::get("TargetAgent.ThreadHeapStatsPlugin")); // inherits configuration from Target Agent
}

bool CLinuxAppStatsPlugin::startPlugin() {
    mCollector->start();
    return true;
}

bool CLinuxAppStatsPlugin::setConfig(
    const std::map<std::string, std::string>& pluginConfiguration) {

    unsigned int samplingRate = 0;
    bool retVal = false;

    for (auto& kv : pluginConfiguration)
    {
        mLogger->information(
            Poco::format("key %s value %s", kv.first, kv.second));
        if (std::string::npos != (kv.first).find("samplingRate"))
        {
            try
            {
                samplingRate = Poco::NumberParser::parseUnsigned(kv.second);
                retVal = true;
            }
            catch (Poco::Exception& e)
            {
                mLogger->log(e);
            }
        }
        else if (std::string::npos != (kv.first).find("Query"))
        {
            mQuery = kv.second;
            retVal = retVal && true;
        }
    }

    mCollector = new CThreadStatsCollector(this, mQuery, samplingRate);
    return retVal;
}

void CLinuxAppStatsPlugin::onMessageReceived(int payloadLength,
        const unsigned char* payloadBuffer) {
    TargetAgent::Protocol::LinuxAppStats::AppConfiguration configItem;
    google::protobuf::TextFormat::ParseFromString(
        std::string((char*) payloadBuffer, payloadLength),
        &configItem);
    mLogger->error(
        Poco::format("parsed message %s",
                     configItem.Utf8DebugString()));
    mLogger->error(
        Poco::format("raw message %s",
                     std::string((char*) payloadBuffer, payloadLength)));

    for (int j = 0; j < configItem.configentry_size(); j++)
    {
        if (configItem.configentry(j).mkey()
            == TargetAgent::Protocol::LinuxAppStats::APP_STATISTICS_SET_RESOLUTION)
        {

            stopPlugin();
            mCollector = new CThreadStatsCollector(this, mQuery,
                                                   configItem.configentry(j).mvalue());
            startPlugin();
            break;

        }
        else if(configItem.configentry(j).mkey()
                == TargetAgent::Protocol::LinuxAppStats::APP_STATISTICS_TRIGGER)
        {
            mCollector->sendQuery(mQuery);

        }
    }

}

Protocol::CommonDefinitions::MessageType CLinuxAppStatsPlugin::MessageType() {
    return mMsgTypeLinuxAppStats;
}

void CLinuxAppStatsPlugin::sendRawTMessage(
    TargetAgent::Protocol::LinuxAppStats::AppStatistics* message) {

    unsigned char payload[message->ByteSize()];

    if (message->SerializeToArray((void*) payload, message->ByteSize()))
    {
        mMsgSenderHDL->sendMessage(mMsgTypeLinuxAppStats, message->ByteSize(),
                                   payload);

    }

}

void CLinuxAppStatsPlugin::onConnectionEstablished() {
}

void CLinuxAppStatsPlugin::onConnectionLost() {
    mLogger->warning(
        "Connection Lost: trigger metadata dispatch, currently none");
}

bool CLinuxAppStatsPlugin::shutdownPlugin() {
    if (mCollector)
        delete mCollector;
    return true;
}

bool CLinuxAppStatsPlugin::stopPlugin() {

    mCollector->stop();
    if (mCollector)
    {
        delete mCollector;
        mCollector = 0;
    }

    return true;
}
CLinuxAppStatsPlugin::~CLinuxAppStatsPlugin() {
}

}
;
