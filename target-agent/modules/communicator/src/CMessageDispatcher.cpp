/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include <typeinfo>
#include <exception>
#include <iostream>

#include "Poco/Net/NetException.h"
#include "Poco/Thread.h"
#include "Poco/Timestamp.h"

#include "CMessageDispatcher.h"
#include "target_agent_version.hpp"
#include "CMessage.h"
#include "CLogger.hpp"
#include <CMediator.h>

namespace TargetAgent
{
namespace Communicator
{

using namespace Mediator;
using Poco::Timestamp;

const int PREALLOCATED_MESSAGE_RECEIVE_BUFFER_SIZE = 128;

CCommMsgDispatcher::CCommMsgDispatcher(Poco::NotificationQueue& queue) :
        mSocket(
            Mediator::CTargetAgentRuntime::getUniqueInstance()->getComController()->getStreamSocket()), metaDataQueue(
                queue), receiverActivity(this,
                                         &CCommMsgDispatcher::runReceiverActivity), senderActivity(this,
                                                 &CCommMsgDispatcher::runSenderActivity), messageQueue(), msgQMutex(), msgQCondition(), messagesAvailable(
                                                     false), messagePayloadReceiveBuffer(
                                                         new unsigned char[PREALLOCATED_MESSAGE_RECEIVE_BUFFER_SIZE]), allocatedMessagePayloadReceiveBufferSize(
                                                             PREALLOCATED_MESSAGE_RECEIVE_BUFFER_SIZE), sendQHighWatermark(
                                                                 0), sendTimeout(500000000), receiveTimout(500000), maxRetries(
                                                                     100), maxHeaderLength(100)
{
    receiverActivity.start();
    senderActivity.start();
}

CCommMsgDispatcher::~CCommMsgDispatcher()
{
    delete[] messagePayloadReceiveBuffer;
}

void CCommMsgDispatcher::runReceiverActivity()
{
#ifdef _POSIX_THREADS
    if (0 != pthread_setname_np(pthread_self(), "CCommMsgRecv"))
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().error("failed to set thread name for CCommMsgRecv");
    }
#endif

    try
    {
        mSocket->setReceiveTimeout(receiveTimout);
        while (!receiverActivity.isStopped())
        {
            socket_error_codes_t errorCode = E_ERROR_UNDEFINED;
            const PluginInterface::ProtocolMessage* msg =
                waitForNextCompleteProtocolMessage(errorCode);
            if (msg != NULL)
            {
                CMessage* notification = new CMessage(
                                             NTFY_NEW_MESSAGE_FROM_HOST_RECEIVED, msg);
                metaDataQueue.enqueueNotification(notification);
            }
            else
            {
                if (E_ERROR_PEER_CLOSED_CONNECTION == errorCode)
                {
                    CMessage* readFailedNtfy = new CMessage(
                                                   NTFY_SOCKET_CONNECTION_CLOSED_BY_HOST, 0);
                    metaDataQueue.enqueueNotification(readFailedNtfy);
                    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error(
                        "Handle connection closed");
                    break;
                }
                else
                {
                    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().information(
                        Poco::format("Following event occurred %d",
                                     ((int) errorCode)));
                }
            }
        }
    }
    catch (const Poco::Exception& e)
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().log(
            e);
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error(
            Poco::format("%s %s %d", std::string(e.what()),
                         std::string(__FILE__), __LINE__));
    }
}

const PluginInterface::ProtocolMessage* CCommMsgDispatcher::waitForNextCompleteProtocolMessage(
    socket_error_codes_t& errorCode)
{
    Protocol::Frame::Header header;
    if (waitForProtocolMessageHeader(header, errorCode))
    {
        const PluginInterface::ProtocolMessage* msg =
            waitForProtocolMessagePayload(header, errorCode);
        return msg;
    }
    return NULL;
}

bool CCommMsgDispatcher::waitForProtocolMessageHeader(
    Protocol::Frame::Header& h, socket_error_codes_t& errorCode)
{
    unsigned char lenByte = 0;
    if (readNBytesFromSocket(&lenByte, 1, errorCode))
    {
        unsigned char header_buffer[MAX_HEADER_LENGTH];
        if (readNBytesFromSocket(header_buffer, lenByte, errorCode))
        {
            return h.ParseFromArray((void*) header_buffer, (int) lenByte);
        }
    }
    return false;
}

const PluginInterface::ProtocolMessage* CCommMsgDispatcher::waitForProtocolMessagePayload(
    Protocol::Frame::Header& h, socket_error_codes_t& errorCode)
{
    if (h.length() > 0)
    {
        adjustMessagePayloadReceiveBuffer(h.length());
        if (readNBytesFromSocket(messagePayloadReceiveBuffer, h.length(),
                                 errorCode))
        {
            return new PluginInterface::ProtocolMessage(h.type(), h.length(),
                    messagePayloadReceiveBuffer);
        }
    }
    else
    {
        return new PluginInterface::ProtocolMessage(h.type(), 0, NULL);
    }
    return NULL;
}

void CCommMsgDispatcher::adjustMessagePayloadReceiveBuffer(
    int expectedMessagePayloadSize)
{
    if (expectedMessagePayloadSize > allocatedMessagePayloadReceiveBufferSize)
    {
        delete[] messagePayloadReceiveBuffer;

        messagePayloadReceiveBuffer =
            new unsigned char[expectedMessagePayloadSize];
        allocatedMessagePayloadReceiveBufferSize = expectedMessagePayloadSize;
    }
}

bool CCommMsgDispatcher::readNBytesFromSocket(unsigned char* buffer, int N,
        socket_error_codes_t& errorCode)
{

    int bytesAlreadyReceived = 0;
    bool retVal = true;

    if (buffer != NULL)
    {
        while (bytesAlreadyReceived != N && retVal)
        {
            try
            {
                int count = mSocket->receiveBytes(
                                (void*) (&buffer[bytesAlreadyReceived]),
                                N - bytesAlreadyReceived);
#ifdef _WIN32

                if (count == -1)
                {
                    return false;
                }
#endif
                if (0 == count)
                {
                    /* connection gracefully closed by peer*/
                    errorCode = E_ERROR_PEER_CLOSED_CONNECTION;
                    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().error(
                        Poco::format("Zero bytes receiveBytes %s %d",
                                     std::string(__FILE__), __LINE__));
                    retVal = false;
                }
                else if (count < 0)
                {
                    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().error(
                        Poco::format("%d read %s %d", count,
                                     std::string(__FILE__),
                                     __LINE__));
                    errorCode = E_ERROR_OTHER;
                    retVal = false;
                }

                bytesAlreadyReceived += count;
            }
            catch (Poco::Net::ConnectionResetException& e)
            {
                Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().log(
                    e);
                Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error(
                    Poco::format("%s %s %d", std::string(e.what()),
                                 std::string(__FILE__), __LINE__));
                errorCode = E_ERROR_PEER_CLOSED_CONNECTION;
                retVal = false;
            }
            catch (Poco::TimeoutException& e)
            {
                Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().debug(
                    Poco::format("%s %s %d", std::string(e.what()),
                                 std::string(__FILE__), __LINE__));
                errorCode = E_ERROR_RECEIVE_TIMEOUT;
                retVal = false;
            }
            catch (Poco::Exception& e)
            {
                Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().log(
                    e);
                Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error(
                    Poco::format("%s %s %d", std::string(e.what()),
                                 std::string(__FILE__), __LINE__));
                errorCode = E_ERROR_RECEIVE_TIMEOUT;
                retVal = false;
            }
            catch (std::exception& e)
            {
                Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error(
                    Poco::format("Exception occurred %s",
                                 std::string(e.what())));
            }
            catch (...)
            {
                Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error(
                    "error");
            }
        }
    }
    else
    {
        retVal = false;
    }
    return retVal;
}

void CCommMsgDispatcher::closeConnection()
{
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().debug(
        Poco::format("shutdownSenderBranch %s %d begin",
                     std::string(__FILE__),
                     __LINE__));
    shutdownSenderBranch();
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().debug(
        Poco::format("shutdownSenderBranch %s %d end",
                     std::string(__FILE__), __LINE__));
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().debug(
        Poco::format("shutdownReceiverBranch %s %d begin",
                     std::string(__FILE__),
                     __LINE__));
    shutdownReceiverBranch();
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().debug(
        Poco::format("shutdownReceiverBranch %s %d end",
                     std::string(__FILE__),
                     __LINE__));
    try
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().debug(
            Poco::format("shutdown socket %s %d begin",
                         std::string(__FILE__), __LINE__));
        mSocket->shutdown();
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().debug(
            Poco::format("shutdown socket %s %d end", std::string(__FILE__),
                         __LINE__));
    }
    catch (Poco::Net::NetException& ne)
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().log(
            ne);
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error(
            Poco::format("%s %s %d", std::string(ne.what()),
                         std::string(__FILE__), __LINE__));
    }
}

void CCommMsgDispatcher::sendMessage(
    const PluginInterface::ProtocolMessage* msg)
{
    if (msg->isValid() && senderActivity.isRunning())
    {
        pushMessageToQueue(msg);
        notifyMessagesAvailable();
    }
    else
    {
        if (!senderActivity.isRunning())
        {
            Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().information(
                "Sender Activity not running");
        }
        delete msg;
    }
}

void CCommMsgDispatcher::pushMessageToQueue(
    const PluginInterface::ProtocolMessage* msg)
{

    static unsigned int highWatermark = 100;

    Poco::Mutex::ScopedLock lock(msgQMutex);
    {
        if (msg->isUrgent())
        {
            messageQueue.push_front(msg);
        }
        else
        {
            messageQueue.push_back(msg);
        }

        if (messageQueue.size() > sendQHighWatermark)
        {
            sendQHighWatermark = messageQueue.size();
            if (sendQHighWatermark > highWatermark)
            {
                highWatermark = highWatermark * 2;
            }
        }
    }
}

void CCommMsgDispatcher::notifyMessagesAvailable()
{
    Poco::Mutex::ScopedLock lock(msgQMutex);
    {
        if (!messagesAvailable)
        {
            messagesAvailable = true;
            msgQCondition.signal();
            Poco::Thread::yield(); /* yield consumer-thread to speed up sending. */
        }
    }
}

void CCommMsgDispatcher::runSenderActivity()
{
    static unsigned long long lastTimestamp = 0;

#ifdef _POSIX_THREADS

    if (0 != pthread_setname_np(pthread_self(), "CCommMsgSender"))
    {
    	Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error("Failed to set thread name for CCommMsgSender");
    }
#endif
    try
    {
        unsigned int retries = 0;
        mSocket->setSendTimeout(sendTimeout);
        while (!senderActivity.isStopped())
        {
            const PluginInterface::ProtocolMessage* msg =
                waitForNextMessageToSend();
            if ((msg != NULL)
                && (msg->getMessageType()
                    == Protocol::CommonDefinitions::MSG_TYPE_RESOURCE_MONITOR))
            {
                long long diff = msg->getTimestamp() - lastTimestamp;
                Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().information(
                    Poco::format("The timestamp %Lu \n", msg->getTimestamp()));
                if (diff <= 0)
                {

                    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error(
                        Poco::format(
                            "Error in Resource Monitor Message Sequence lastTimestamp %Lu current %Lu diff %Ld",
                            lastTimestamp, msg->getTimestamp(), diff));
                }

                lastTimestamp = msg->getTimestamp();

            }

            if (!transmitMessage(msg))
            {
                Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().error(
                    Poco::format(
                        "Fail to transmit message %s %d drop message",
                        std::string(__FILE__), __LINE__));
                retries++;

            }
            else
                retries = 0;

            disposeMessage(msg);

            updateMessageAvailableFlag();

            if (retries == maxRetries)
            {
                CMessage* readFailedNtfy = new CMessage(
                                               NTFY_SOCKET_CONNECTION_CLOSED_BY_HOST, 0);
                metaDataQueue.enqueueNotification(readFailedNtfy);
                Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error(
                    "Handle abrupt connection closed");
                retries = 0;
                break;
            }
        }
    }
    catch (Poco::Exception& e)
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().log(
            e);
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error(
            Poco::format("%s %s %d", std::string(e.what()),
                         std::string(__FILE__), __LINE__));
    }
}

const PluginInterface::ProtocolMessage* CCommMsgDispatcher::waitForNextMessageToSend()
{
    waitForMessagesAvailable();
    return popMessageFromQueue();
}

void CCommMsgDispatcher::waitForMessagesAvailable()
{
    Poco::Mutex::ScopedLock lock(msgQMutex);
    {
        while (!messagesAvailable)
            msgQCondition.wait(msgQMutex);
    }
}

const PluginInterface::ProtocolMessage* CCommMsgDispatcher::popMessageFromQueue()
{
    const PluginInterface::ProtocolMessage* msg = NULL;
    Poco::Mutex::ScopedLock lock(msgQMutex);
    {
        if (messageQueue.size() > 0)
        {
            msg = messageQueue.front();
            messageQueue.pop_front();
        }
    }
    return msg;
}

bool CCommMsgDispatcher::transmitMessage(
    const PluginInterface::ProtocolMessage* msg) const
{
    bool transmissionSuccessful = false;

    if (msg != NULL)
    {
        transmissionSuccessful = transmitMessageHeader(msg);
        if (transmissionSuccessful)
        {
            transmissionSuccessful = transmitMessagePayload(msg);
        }
    }
    else
    {
        transmissionSuccessful = true; /* A NULL-message shall not cause an connection-abort. */
    }

    return transmissionSuccessful;
}

bool CCommMsgDispatcher::transmitMessageHeader(
    const PluginInterface::ProtocolMessage* msg) const
{
    unsigned char headerTransmitBuffer[MAX_HEADER_LENGTH];

    Protocol::Frame::Header header;
    header.set_type(msg->getMessageType());
    header.set_length(msg->getMessageLength());
    header.set_timestamp(msg->getTimestamp());
    header.set_versiontoken(msg->getVersionToken());
    std::map<std::string,std::string> contextMap = msg->getContextInfo();
    for(std::map<std::string,std::string>::iterator it=contextMap.begin(); it!=contextMap.end(); ++it)
    {
        TargetAgent::Protocol::Frame::MetaData* metadata = header.add_metadatainfo();
        metadata->set_key(it->first);
        metadata->set_value(it->second);
    }

    unsigned char headerLength = header.ByteSize();

    headerTransmitBuffer[0] = (unsigned char) headerLength;
    if (header.SerializeToArray((void*) (&headerTransmitBuffer[1]),
                                headerLength))
    {
        return writeNBytesToSocket(headerTransmitBuffer, headerLength + 1);
    }

    return false;
}
bool CCommMsgDispatcher::transmitMessagePayload(
    const PluginInterface::ProtocolMessage* msg) const
{
    return writeNBytesToSocket(msg->getPayloadData(), msg->getMessageLength());
}

bool CCommMsgDispatcher::writeNBytesToSocket(const unsigned char* buffer,
        int N) const
{

    int bytesAlreadySent = 0;
    int turn = 0;
    bool retVal = true;

    while (retVal && bytesAlreadySent != N)
    {

        try
        {
            int writtenBytes = mSocket->sendBytes(
                                   (const void*) (&buffer[bytesAlreadySent]),
                                   N - bytesAlreadySent);

            if ((writtenBytes < 0) || (turn > 100))
            {
                Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().error(
                    Poco::format("writtenBytes %d turn %d %s %d",
                                 writtenBytes, turn, std::string(__FILE__),
                                 __LINE__));
                retVal = false;
            }
            else
            {

                bytesAlreadySent += writtenBytes;
                turn++;

            }

        }
        catch (Poco::Exception& e)
        {
            Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().log(
                e);
            Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error(
                Poco::format("%s %s %d", std::string(e.what()),
                             std::string(__FILE__), __LINE__));
            retVal = false;
        }
    }

    return retVal;
}

void CCommMsgDispatcher::disposeMessage(
    const PluginInterface::ProtocolMessage* msg)
{
    if (msg != NULL)
    {
        delete msg;
    }
}

void CCommMsgDispatcher::updateMessageAvailableFlag()
{
    Poco::Mutex::ScopedLock lock(msgQMutex);
    {
        if (messageQueue.size() == 0)
        {
            messagesAvailable = false;
        }
    }
}

void CCommMsgDispatcher::shutdownSenderBranch()
{
    senderActivity.stop();
    notifyMessagesAvailable();
    senderActivity.wait();

    clearPendingMessages();
}

void CCommMsgDispatcher::shutdownReceiverBranch()
{
    receiverActivity.stop();
    receiverActivity.wait();
}

void CCommMsgDispatcher::clearPendingMessages()
{
    Poco::Mutex::ScopedLock lock(msgQMutex);
    {
        if (messageQueue.size() > 0)
        {
            std::deque<const PluginInterface::ProtocolMessage*>::iterator messageIterator =
                messageQueue.begin();
            for (; messageIterator != messageQueue.end(); messageIterator++)
            {
                disposeMessage(*messageIterator);
            }
            messageQueue.clear();
        }
    }
}

}
;
}
;

