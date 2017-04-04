/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#include "dbus-monitor-plugin.h"
#include "dbus_handler.h"
#include "dbus_data_pool.h"
#include "dbus_consumer.h"
#include "dbus_producer.h"

#include "target_agent_prot_dbus_monitor.pb.h"

using namespace TargetAgent::PluginInterface;

extern "C" void* createPluginInstance(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider) {
    return new TargetAgentDbusMonitor::CDbusMonitorPlugin(senderHandle, tsProvider);
}

namespace TargetAgentDbusMonitor {

using namespace TargetAgent;

CDbusMonitorPlugin::CDbusMonitorPlugin(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider):mMsgSenderHDL(senderHandle),mTimestampProvider(tsProvider),
mMessageType(Protocol::CommonDefinitions::MSG_TYPE_DBUS), mDbusLogger(0), mDbusManager() {
    mDbusLogger = &(Poco::Logger::get("TargetAgent.DbusPlugin")); // inherits configuration from Target Agent

    mDbusManager = new CDBusHandler(this);

    mDbusLogger->information("Dbus Monitor Ctor");
}

CDbusMonitorPlugin::~CDbusMonitorPlugin() {

}

void CDbusMonitorPlugin::sendApplicationMessage(
    DBusApplicationMessage* evtTraceMessage) {

    unsigned char payload[evtTraceMessage->ByteSize()];

    if (evtTraceMessage->SerializeToArray((void*) payload,
                                          evtTraceMessage->ByteSize()))
    {
        mMsgSenderHDL->sendMessage(
            Protocol::CommonDefinitions::MSG_TYPE_DBUS,
            evtTraceMessage->ByteSize(), payload);

    }
}

void CDbusMonitorPlugin::onConnectionEstablished(void) {

}

void CDbusMonitorPlugin::onConnectionLost() {
    mDbusLogger->error("Connection Lost: trigger metadata dispatch");
}

bool CDbusMonitorPlugin::startPlugin() {
    mDbusManager->start();

    return true;
}

bool CDbusMonitorPlugin::stopPlugin() {
    mDbusManager->stop();
    return true;
}

void CDbusMonitorPlugin::onMessageReceived(int payloadLength,
        const unsigned char* payloadBuffer) {
    mDbusLogger->information("Message Received by Plugin");

}

Protocol::CommonDefinitions::MessageType CDbusMonitorPlugin::MessageType() {
    return mMessageType;
}

bool CDbusMonitorPlugin::setConfig(
    const std::map<std::string, std::string>& pluginConfiguration) {
    bool retVal = true;
    mDbusLogger->information("Set Configuration");


    for (std::map<std::string, std::string>::const_iterator it=pluginConfiguration.begin(); it!=pluginConfiguration.end(); ++it)
    {

        mDbusLogger->information(
            Poco::format("configuration key %s value %s", it->first,
                         it->second));

        if (std::string::npos != (it->first).find("DBUS_BUS"))
        {

            if (std::string::npos != (it->second).find("DBUS_BUS_SESSION"))
            {
                mDbusManager->getDataPool()->setObservedBus(DBUS_BUS_SESSION);
            }
            else if (std::string::npos
                     != (it->second).find("DBUS_BUS_SYSTEM"))
            {
                mDbusManager->getDataPool()->setObservedBus(DBUS_BUS_SYSTEM);
            }
            else
            {
                mDbusLogger->error("Invalid Dbus bus");

                retVal = false;
            }
            mDbusLogger->information(Poco::format("Dbus Bus %s", it->second));

        }
    }
    return retVal;
}

bool CDbusMonitorPlugin::shutdownPlugin() {
    return true;
}

}
;

