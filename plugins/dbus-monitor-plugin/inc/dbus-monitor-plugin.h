/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#ifndef DBUS_MONITOR_PLUGIN_
#define DBUS_MONITOR_PLUGIN_

#include <iostream>

#include "Poco/Timer.h"
#include "Poco/Logger.h"
#include "Poco/SharedPtr.h"

#include "TargetAgentPluginInterface.h"
#include "dbus-monitor-plugin-message-sender.h"
#include "target_agent_prot_dbus_monitor.pb.h"

namespace TargetAgentDbusMonitor {

class CDBusHandler;

using namespace TargetAgent;
using namespace PluginInterface;

class CDbusMonitorPlugin: public PluginInterface::TargetAgentPluginInterface,
    public DbusMonitorMessageSender {
public:

    CDbusMonitorPlugin(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider);
    virtual ~CDbusMonitorPlugin();

    void onMessageReceived(int payloadLength,
                           const unsigned char* payloadBuffer);
    Protocol::CommonDefinitions::MessageType MessageType();

    bool startPlugin();
    bool stopPlugin();
    bool shutdownPlugin();
    virtual void sendApplicationMessage(
        DBusApplicationMessage* evtTraceMessage);
    void onConnectionEstablished(void);

    void onConnectionLost();

    bool setConfig(
        const std::map<std::string, std::string>& pluginConfiguration);

private:
    const CMessageDispatcher * const mMsgSenderHDL;
    const CTimestampProvider * const  mTimestampProvider;
    Protocol::CommonDefinitions::MessageType mMessageType;
    Poco::Logger* mDbusLogger;
    Poco::SharedPtr<CDBusHandler> mDbusManager;
};

}
;
#endif

