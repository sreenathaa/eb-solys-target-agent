/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include <deque>

#include "Poco/Event.h"
#include "Poco/Mutex.h"
#include "TargetAgentDispatcherInterface.h"

namespace TargetAgent{
namespace MessageRecorder{
namespace Tests{

class DummyReceiver : public PluginInterface::CMessageDispatcher
{
public:
    DummyReceiver()
            : receivedMessagesQueueHasElement(false)
    {
    }

    virtual ~DummyReceiver()
    {
        if (!receivedMessages.empty())
        {
            std::cout << "DummyReceiver~DummyReceiver() deletes " << receivedMessages.size() << " queued messages" << std::endl;
            for(std::deque<const PluginInterface::ProtocolMessage*>::iterator it = receivedMessages.begin(); it != receivedMessages.end(); it++)
            {
                delete *it;
            }
        }
    }

    inline void sendMessage(PluginInterface::MessageType type, int payloadLength,
                            unsigned char* payloadBuffer) const {

    }

    inline void sendMessage(PluginInterface::MessageType type, int payloadLength,
                            unsigned char* payloadBuffer,
                            unsigned long long timestamp) const {

    }



    inline void sendMessage(const PluginInterface::ProtocolMessage* msg)
    {
        receivedMessagesMutex.lock();
        receivedMessages.push_back(msg);
        receivedMessagesQueueHasElement.set();
        receivedMessagesMutex.unlock();
    }

    inline const PluginInterface::ProtocolMessage* getNextMessage()
    {
        if (receivedMessagesQueueHasElement.tryWait(100))
        {
            receivedMessagesMutex.lock();
            const PluginInterface::ProtocolMessage* ptr = receivedMessages.front();
            receivedMessages.pop_front();
            if(receivedMessages.empty())
            {
                receivedMessagesQueueHasElement.reset();
            }
            receivedMessagesMutex.unlock();
            return ptr;
        }
        return NULL;
    }

private:
    std::deque<const PluginInterface::ProtocolMessage*> receivedMessages;
    Poco::Event receivedMessagesQueueHasElement;
    Poco::Mutex receivedMessagesMutex;
};

}
; // Tests
}
; // MessageRecorder
}
; // TargetAgent
