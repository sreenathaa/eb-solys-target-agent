/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef PROTOCOL_MESSAGE_SENDER_H_
#define PROTOCOL_MESSAGE_SENDER_H_

#if defined(_WIN32)
 #define LIBRARY_API __declspec(dllexport)
#else
 #define LIBRARY_API
#endif
#include <memory>
#include <iostream>
#include "Poco/Logger.h"
#include "Poco/SharedPtr.h"
/**
* Message Type for this specific plugin
*/
#include "../gen/target_agent_prot_common_definitions.pb.h"


#ifdef __cplusplus

namespace TargetAgent{
namespace PluginInterface{

using namespace Protocol::CommonDefinitions;
/**
* Target Agent Dispatcher Interface.
*/
class LIBRARY_API CMessageDispatcher {
public:
    /**
     *  Send Message to Target Agent stack
     * @param type:    predefined message type
     * @param payloadLength: message length expressed in bytes
     * @param payloadBuffer: pointer to message payload: guaranteed to be valid until function exists
     * @param timestamp:     target system time expressed in miliseconds
     * @param msgContextInfo map containing information that shall be used on tool side for screening of messages
     *        which meet specific criteria without needing to decode the complete message
     *
     * @see -
     * @return none
     */
    virtual void sendMessage(MessageType type,
                             int payloadLength,
                             unsigned char* payloadBuffer,
                             unsigned long long timestamp = 0,
                             const std::map<std::string, std::string>* msgContextInfo = 0) const = 0;


    virtual ~CMessageDispatcher() {
    }
};

};
};
#else

typedef
struct CMessageDispatcher
            CMessageDispatcher;
#endif


#endif /* PROTOCOL_MESSAGE_SENDER_H_ */
