/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "TargetAgentDispatcherInterface.h"
#include "Poco/Runnable.h"

#include <sstream>

namespace TargetAgent{
namespace MessageRecorder{
namespace Tests{

class TestMessageSenderPlugin : public PluginInterface::CMessageDispatcher,
            public Poco::Runnable
{
public:
    TestMessageSenderPlugin(PluginInterface::ProtocolMessage* messageToSend, int numberOfTimes, PluginInterface::CMessageDispatcher* sender, long sleepBetweenMessageInMilliseconds)
            : messageToSend(messageToSend),
            numberOfTimes(numberOfTimes),
            sender(sender),
            sleepTimeInMilliseconds(sleepBetweenMessageInMilliseconds),
            writeMessageNumberAsPayload(false)
    {
    }

    TestMessageSenderPlugin(PluginInterface::ProtocolMessage* messageToSend, int numberOfTimes, PluginInterface::CMessageDispatcher* sender, long sleepBetweenMessageInMilliseconds, bool writeMessageNumberAsPayload)
            : messageToSend(messageToSend),
            numberOfTimes(numberOfTimes),
            sender(sender),
            sleepTimeInMilliseconds(sleepBetweenMessageInMilliseconds),
            writeMessageNumberAsPayload(writeMessageNumberAsPayload)
    {
    }

    virtual ~TestMessageSenderPlugin() {
    }

    virtual void run()
    {
        for (int i = 0; i < numberOfTimes; i++)
        {
            if (writeMessageNumberAsPayload)
            {
                int actualMessageNumber = i + 1;
                std::stringstream ss;
                ss << "MessageNumber_" << actualMessageNumber << "_";
                std::string messageToSendOriginalPayload(
                    (char*) messageToSend->getPayloadData(),
                    messageToSend->getMessageLength());
                ss << messageToSendOriginalPayload;
                std::string newPayloadString = ss.str();
                sender->sendMessage(messageToSend->getMessageType(),
                                    newPayloadString.size(),
                                    (unsigned char*) newPayloadString.c_str());
            }
            else
            {
                //sender->sendMessage(new PluginInterface::ProtocolMessage(*messageToSend));
            }
            Poco::Thread::sleep(sleepTimeInMilliseconds);
        }
    }

    inline void sendMessage(const PluginInterface::ProtocolMessage* msg)
    {
        std::cout << "dummy_plugin sendMessage() shouldn't be called!!!" << std::endl;
        ASSERT_TRUE(false);
    }

private:
    PluginInterface::ProtocolMessage* messageToSend;
    const int numberOfTimes;
    CMessageDispatcher* sender;
    long sleepTimeInMilliseconds;
    bool writeMessageNumberAsPayload;
};

}
; // Tests
}
; // MessageRecorder
}
; // TargetAgent
