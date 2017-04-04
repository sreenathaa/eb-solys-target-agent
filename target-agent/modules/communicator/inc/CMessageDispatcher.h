/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef HOST_CONNECTION_H_
#define HOST_CONNECTION_H_

#include "Poco/Activity.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Mutex.h"
#include "Poco/Condition.h"
#include "Poco/NotificationQueue.h"
#include "Poco/Timestamp.h"

#include <deque>

#include "target_agent_prot_frame.pb.h"
#include "protocol_message.h"


namespace TargetAgent{
namespace Communicator{


class CCommMsgDispatcher  {
public:
    enum socket_error_codes_t {
        E_ERROR_UNDEFINED = 0,
        E_ERROR_PEER_CLOSED_CONNECTION,
        E_ERROR_RECEIVE_TIMEOUT,
        E_ERROR_OTHER,
    };
public:
    CCommMsgDispatcher(Poco::NotificationQueue& queue);
    virtual ~CCommMsgDispatcher();

    void closeConnection();

    void sendMessage(const PluginInterface::ProtocolMessage* msg);

protected:
    void runSenderActivity();
    void runReceiverActivity();

private:
    /* Message-Receiver related stuff */
    const PluginInterface::ProtocolMessage*  waitForNextCompleteProtocolMessage(socket_error_codes_t& errorCode);
    const PluginInterface::ProtocolMessage*  waitForProtocolMessagePayload (Protocol::Frame::Header& h,socket_error_codes_t& errorCode);
    bool waitForProtocolMessageHeader (Protocol::Frame::Header& h,socket_error_codes_t& errorCode);
    bool readNBytesFromSocket(unsigned char* buffer, int N, socket_error_codes_t& errorCode);

    /* Message-Sender related stuff */
    void pushMessageToQueue(const PluginInterface::ProtocolMessage* msg);
    void notifyMessagesAvailable();
    const PluginInterface::ProtocolMessage* waitForNextMessageToSend();
    void waitForMessagesAvailable();
    const PluginInterface::ProtocolMessage* popMessageFromQueue();
    bool transmitMessage(const PluginInterface::ProtocolMessage* msg) const;
    bool transmitMessageHeader(const PluginInterface::ProtocolMessage* msg) const;
    bool transmitMessagePayload(const PluginInterface::ProtocolMessage* msg) const;
    bool writeNBytesToSocket (const unsigned char* buffer, int N)const;
    void updateMessageAvailableFlag();
    void disposeMessage(const PluginInterface::ProtocolMessage* msg);

    /* Admin-Stuff */
    void shutdownSenderBranch();
    void shutdownReceiverBranch();
    void clearPendingMessages();

    CCommMsgDispatcher(const CCommMsgDispatcher& that);
    const CCommMsgDispatcher& operator=(const CCommMsgDispatcher& rhs);

    void adjustMessagePayloadReceiveBuffer(int expectedMessagePayloadSize);

private:
    Poco::Net::StreamSocket* mSocket;
    Poco::NotificationQueue& metaDataQueue;
    mutable Poco::Activity<CCommMsgDispatcher> receiverActivity;
    mutable Poco::Activity<CCommMsgDispatcher> senderActivity;
    std::deque<const PluginInterface::ProtocolMessage*> messageQueue;
    Poco::Mutex msgQMutex;
    Poco::Condition msgQCondition;
    bool messagesAvailable;
    unsigned char* messagePayloadReceiveBuffer;
    int allocatedMessagePayloadReceiveBufferSize;
    unsigned int sendQHighWatermark;
    const unsigned long long sendTimeout;
    const unsigned long long receiveTimout;
    const unsigned long long maxRetries;
    const unsigned long long maxHeaderLength;

#define MAX_HEADER_LENGTH 100
};


};
};



#endif /* HOST_CONNECTION_H_ */
