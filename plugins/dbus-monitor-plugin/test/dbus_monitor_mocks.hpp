/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef MESSAGE_DISPATCHER_MOCKER_HPP_
#define MESSAGE_DISPATCHER_MOCKER_HPP_

#include <iostream>

#include "gmock/gmock.h"
#include "../gen/target_agent_prot_common_definitions.pb.h"

/*this file does not belong here: it shall be visible for every single plugin*/
namespace TaDispatcherMocker {

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
};


}
;

#endif /* MESSAGE_DISPATCHER_MOCKER_HPP_ */
