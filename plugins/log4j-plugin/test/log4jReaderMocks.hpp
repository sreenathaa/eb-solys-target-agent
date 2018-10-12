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

namespace Log4jReaderMocks {

using namespace TargetAgent;

using namespace TargetAgent;

class TaDispatcher: public PluginInterface::CMessageDispatcher {
public:
    MOCK_CONST_METHOD5(sendMessage, void(Protocol::CommonDefinitions::MessageType type,
                                         int payloadLength,
                                         unsigned char* payloadBuffer,
                                         unsigned long long timestamp,
                                         const std::map<std::string, std::string>* msgContextInfo));
};

class CTimestampProviderMock: public PluginInterface::CTimestampProvider {
public:
    MOCK_CONST_METHOD0(createTimestamp, unsigned long long(void));
    MOCK_CONST_METHOD1(createTimestampFromDateTime, unsigned long long(const std::string& date_time));
};


}
;




#endif /* PLUGINS_PLUGIN_TEST_LOG4J_MOCKS_HPP_ */
