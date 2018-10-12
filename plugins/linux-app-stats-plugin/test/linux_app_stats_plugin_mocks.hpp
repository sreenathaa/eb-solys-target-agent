/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "gmock/gmock.h"
#include "Poco/Logger.h"

#include "TargetAgentPluginInterface.h"
#include "../gen/target_agent_prot_common_definitions.pb.h"
#include "TargetAgentTsProviderInterface.h"



using Poco::Logger;
using namespace TargetAgent;
using namespace PluginInterface;

#include "../gen/target_agent_prot_linux_app_stats.pb.h"


namespace SocketReaderMocks {

using namespace TargetAgent;


using namespace TargetAgent;

class TaDispatcher: public PluginInterface::CMessageDispatcher {
public:
    MOCK_CONST_METHOD3(sendMessage, void(Protocol::CommonDefinitions::MessageType type, int payloadLength, unsigned char* payloadBuffer));
    MOCK_CONST_METHOD4(sendMessage, void(Protocol::CommonDefinitions::MessageType type, int payloadLength,
                                         unsigned char* payloadBuffer,
                                         unsigned long long timestamp));

};

class CTimestampProviderMock: public PluginInterface::CTimestampProvider {
public:
    MOCK_CONST_METHOD0(createTimestamp, unsigned long long(void));
};


class RMMock: public PluginInterface::TargetAgentPluginInterface {
public:
    MOCK_METHOD1(initialize, bool(std::shared_ptr<CMessageDispatcher> senderHandle));
    MOCK_METHOD1(setConfig, bool(const std::map<std::string,std::string>& pluginConfiguration));
    MOCK_METHOD2(onMessageReceived, void(int payloadLength,const unsigned char* payloadBuffer));
    MOCK_METHOD0(MessageType, Protocol::CommonDefinitions::MessageType());
    MOCK_METHOD0(onConnectionEstablished, void());
    MOCK_METHOD0(onConnectionLost, void());
    MOCK_METHOD0(startPlugin, bool());
    MOCK_METHOD0(stopPlugin, bool());
    MOCK_METHOD1(sendRawTMessage, void(TargetAgent::Protocol::LinuxAppStats::AppStatistics* message));
};

}
;
