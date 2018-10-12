/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef LINUX_APP_STATS_PLUGIN_H_
#define LINUX_APP_STATS_PLUGIN_H_

#include <iostream>
#include <fstream>

#include "Poco/Logger.h"

#include "TargetAgentPluginInterface.h"

#include "../../linux-app-stats-plugin/gen/target_agent_prot_linux_app_stats.pb.h"
#include "../../linux-app-stats-plugin/inc/stats_collector.h"

using namespace std;

using namespace TargetAgent;
using namespace PluginInterface;

extern "C" void* createPluginInstance(
        const CMessageDispatcher* const senderHandle,
        const CTimestampProvider* tsProvider);

namespace LinuxAppStatsPlugin {

class CLinuxAppStatsPlugin: public PluginInterface::TargetAgentPluginInterface {
public:

    CLinuxAppStatsPlugin(const CMessageDispatcher* const senderHandle,
                         const CTimestampProvider* tsProvider);
    virtual ~CLinuxAppStatsPlugin();

    void sendRawTMessage(
        TargetAgent::Protocol::LinuxAppStats::AppStatistics * message);

    void onMessageReceived(int payloadLength,
                           const unsigned char* payloadBuffer);

    Protocol::CommonDefinitions::MessageType MessageType();

    bool shutdownPlugin();
    void onConnectionEstablished();
    void onConnectionLost();
    bool startPlugin();
    bool stopPlugin();
    bool setConfig(
        const std::map<std::string, std::string>& pluginConfiguration);

private:
    const CMessageDispatcher * const mMsgSenderHDL;
    const CTimestampProvider * const mTimestampProvider;
    Protocol::CommonDefinitions::MessageType mMsgTypeLinuxAppStats;
    Poco::Logger* mLogger;
    std::string mQuery;

public:
    CThreadStatsCollector* mCollector;

};

}

#endif /* LINUX_APP_STATS_PLUGIN_H_ */
