/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#include <iostream>
#include "dlt-monitor-plugin.h"
#include "target_agent_prot_dlt_monitor_plugin.pb.h"

using namespace TargetAgent::PluginInterface;

extern "C" void* createPluginInstance(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider) {
    return new DltMonitorPlugin::DltMonitorPlugin(senderHandle, tsProvider);
}

namespace DltMonitorPlugin {

using namespace TargetAgent;

DltMonitorPlugin::DltMonitorPlugin(const CMessageDispatcher* const senderHandle,
                                   const CTimestampProvider* tsProvider) :
        mMsgSenderHDL(senderHandle), mTimestampProvider(tsProvider), messageTypeDLT(
            Protocol::CommonDefinitions::MSG_TYPE_GENIVI_DLT_MONITOR_PLUGIN), messageTypeMOST(
                Protocol::CommonDefinitions::MSG_TYPE_MOST_SPY_MONITOR_PLUGIN), logger(
            0), dltMessageHandler(0) {
    logger = &(Poco::Logger::get("TargetAgent.DltMonitorPlugin")); // inherits configuration from Target Agent
}

DltMonitorPlugin::~DltMonitorPlugin() {
}


bool DltMonitorPlugin::setConfig(
    const std::map<std::string, std::string>& pluginConfiguration) {
    std::ofstream ofs("filter.txt", std::ofstream::out);
    for (auto& kv : pluginConfiguration)
    {
        logger->information(
            Poco::format("key %s value %s", kv.first, kv.second));
        if (std::string::npos != (kv.first).find("dltFilter"))
        {
            logger->information(Poco::format("dlt filter %s", kv.second));
            ofs << kv.second << std::endl;

        }
    }
    ofs.close();
    return true;
}

void DltMonitorPlugin::onMessageReceived(int payloadLength,
        const unsigned char* payloadBuffer) {

}

Protocol::CommonDefinitions::MessageType DltMonitorPlugin::MessageType() {
    return messageTypeDLT;
}

void DltMonitorPlugin::sendDLTMessage(
    TargetAgent::Protocol::DLTLogInspector::DLTLogInspectorMessage* message) {

    unsigned char payload[message->ByteSize()];

    if (message->SerializeToArray((void*) payload, message->ByteSize()))
    {
        mMsgSenderHDL->sendMessage(messageTypeDLT, message->ByteSize(),
                                   payload);

    }

}
void DltMonitorPlugin::sendMOSTMessage(
    TargetAgent::Protocol::MostSpy::MostSpyApplicationMessage* message) {
    unsigned char payload[message->ByteSize()];

    if (message->SerializeToArray((void*) payload, message->ByteSize()))
    {
        mMsgSenderHDL->sendMessage(messageTypeMOST, message->ByteSize(),
                                   payload);

    }

}
void DltMonitorPlugin::initializePlugin()
{

    dltMessageHandler = new DltMessageHandler(*this);
}

bool DltMonitorPlugin::shutdownPlugin() {
    if (dltMessageHandler)
        delete dltMessageHandler;
    return true;
}

void DltMonitorPlugin::onConnectionEstablished() {
}

void DltMonitorPlugin::onConnectionLost() {
    logger->warning(
        "Connection Lost: trigger metadata dispatch, currently none");
}

bool DltMonitorPlugin::startPlugin() {
    dltMessageHandler = new DltMessageHandler(*this);
    dltMessageHandler->start();
    return true;
}

bool DltMonitorPlugin::stopPlugin() {
    logger->information("Stop DltMonitorPlugin before");
    dltMessageHandler->stop();
    logger->information("Stop DltMonitorPlugin after");
    return true;
}

}
;

