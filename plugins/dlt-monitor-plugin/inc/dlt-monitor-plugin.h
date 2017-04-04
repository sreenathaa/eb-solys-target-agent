/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#ifndef DLT_MONITOR_PLUGIN_H_
#define DLT_MONITOR_PLUGIN_H_

#if defined(_WIN32)
#define LIBRARY_API __declspec(dllexport)
#else
#define LIBRARY_API
#endif

#include "TargetAgentPluginInterface.h"
#include "dlt-client-activity.h"
#include "dlt-client-message-sender.h"
#include "target_agent_prot_dlt_monitor_plugin.pb.h"
#include "target_agent_prot_most_spy.pb.h"
#include <iostream>
#include <fstream>
#include "Poco/Logger.h"

using namespace std;
using namespace TargetAgent::PluginInterface;

extern "C" void* createPluginInstance(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider);

namespace DltMonitorPlugin
{

using namespace TargetAgent;
using namespace PluginInterface;

class DltMonitorPlugin: public PluginInterface::TargetAgentPluginInterface,
    public DltClientMessageSender {
public:

    DltMonitorPlugin(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider);
    virtual ~DltMonitorPlugin();

    void sendDLTMessage(
        TargetAgent::Protocol::DLTLogInspector::DLTLogInspectorMessage* message);

    void onMessageReceived(int payloadLength,
                           const unsigned char* payloadBuffer);

    void sendMOSTMessage(
        TargetAgent::Protocol::MostSpy::MostSpyApplicationMessage* message);
    Protocol::CommonDefinitions::MessageType MessageType();

    bool shutdownPlugin();
    void onConnectionEstablished();
    void onConnectionLost();
    bool startPlugin();
    bool stopPlugin();
    bool setConfig(const std::map<std::string, std::string>& pluginConfiguration);

protected:
    void initializePlugin();

private:
    const CMessageDispatcher * const mMsgSenderHDL;
    const CTimestampProvider * const  mTimestampProvider;
    Protocol::CommonDefinitions::MessageType messageTypeDLT;
    Protocol::CommonDefinitions::MessageType messageTypeMOST;
    Poco::Logger* logger;

public:
    DltMessageHandler* dltMessageHandler;

};

}
;

#endif /* DLT_MONITOR_PLUGIN_H_ */
