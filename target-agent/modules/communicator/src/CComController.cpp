/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "CComController.h"
#include "CLogger.hpp"
#include <iostream>
#include "Poco/AutoPtr.h"
#include "Poco/Thread.h"
#include <CMediator.h>
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/ServerSocket.h"

namespace TargetAgent {
namespace Communicator {

const int PREALLOCATED_MESSAGE_PAYLOAD_BUFFER_SIZE = 128;

using namespace Mediator;

ComController::ComController(void) :
        eventQueue(), eventLoopMutex(), mSocket(0), eventLoop(this,
                &ComController::runEventLoop), messageHandler(NULL), comServer(
                    eventQueue), messagePayloadBuffer(
                        new unsigned char[PREALLOCATED_MESSAGE_PAYLOAD_BUFFER_SIZE]), allocatedMessagePayloadBufferSize(
                            PREALLOCATED_MESSAGE_PAYLOAD_BUFFER_SIZE), commandControlPort(
                        Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->CmdCtrlPort()), reactor() {
}

ComController::~ComController() {

}

void ComController::start() {
    eventLoop.start();
    comServer.start(
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->StreamingPort());
    connectionAcceptor = new CCSocketAcceptor(commandControlPort, reactor,
                         &eventQueue);

    reactorThread.start(reactor);
#ifdef _POSIX_THREADS

    if (0 != pthread_setname_np(reactorThread.tid(), "CCommReactor"))
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().error("failed to set thread name for CCommReactor");
    }
#endif

}

void ComController::stop() {
    if (messageHandler != NULL)
    {
        messageHandler->closeConnection();
        comServer.stop();
        reactor.stop();
        reactorThread.join();
    }
}

void ComController::sendMessage(const PluginInterface::ProtocolMessage* msg) {
    if (msg != NULL)
    {
        if (messageHandler != NULL)
        {
            messageHandler->sendMessage(msg);
        }
        else
        {
            delete msg;
        }
    }
}

void ComController::runEventLoop() {

#ifdef _POSIX_THREADS
    if (0 != pthread_setname_np(pthread_self(), "CCommController"))
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().error("failed to set thread name for CCommController");
    }
#endif

    while (eventLoop.isRunning())
    {
        try
        {
            Poco::AutoPtr<Poco::Notification> notification(
                eventQueue.waitDequeueNotification());
            while (!notification.isNull())
            {
                eventLoopMutex.lock();
                {
                    Communicator::CMessage* baseNotification =
                        dynamic_cast<Communicator::CMessage*>(notification.get());

                    if (baseNotification != NULL)
                        dispatchCommunicatorEvent(baseNotification);
                }
                eventLoopMutex.unlock();

                notification = eventQueue.waitDequeueNotification();
            }
        }
        catch (const Poco::Exception& e)
        {
            Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().log(
                e);
        }
    }
}

void ComController::sendVersionInformation() {
    Protocol::Ctrl::ProtHandlerCtrlMessage ctrlMsg;
    ctrlMsg.set_id(Protocol::Ctrl::PROT_HNDLR_CTRL_EVT_VERSION_INFO_ID);

    Protocol::Ctrl::ProtHandlerCtrlEvtVersionInfo* versionInfoMsg =
        ctrlMsg.mutable_versioninfo();
    versionInfoMsg->set_majorversion(
        Protocol::Ctrl::PROT_HNDLR_CTRL_MAJOR_VERSION);
    versionInfoMsg->set_minorversion(
        Protocol::Ctrl::PROT_HNDLR_CTRL_MINOR_VERSION);

    sendProtHndlrControlMessageToHost(ctrlMsg);
}

void ComController::adjustMessagePayloadBufferSize(
    int currentMessagePayloadSize) {
    if (currentMessagePayloadSize > allocatedMessagePayloadBufferSize)
    {
        delete[] messagePayloadBuffer;
        messagePayloadBuffer = new unsigned char[currentMessagePayloadSize];
        allocatedMessagePayloadBufferSize = currentMessagePayloadSize;
    }
}

void ComController::sendProtHndlrControlMessageToHost(
    Protocol::Ctrl::ProtHandlerCtrlMessage& ctrlMsg) {

    adjustMessagePayloadBufferSize(ctrlMsg.ByteSize());

    if (ctrlMsg.SerializeToArray((void*) messagePayloadBuffer,
                                 ctrlMsg.ByteSize()))
    {
        bool messageIsUrgent = true;
        const PluginInterface::ProtocolMessage* reply =
            new PluginInterface::ProtocolMessage(
                Protocol::CommonDefinitions::MSG_TYPE_PROT_HNDLR_CONTROL,
                ctrlMsg.ByteSize(), messagePayloadBuffer,
                messageIsUrgent);
        messageHandler->sendMessage(reply);
    }
}

void ComController::onMessageReceived(
    const PluginInterface::ProtocolMessage* msg) {
    if (msg->getMessageType()
        != Protocol::CommonDefinitions::MSG_TYPE_PROT_HNDLR_CONTROL)
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getPluginManager()->onMessageReceived(
            msg);
    }
    else
    {
        Protocol::Ctrl::ProtHandlerCtrlMessage ctrlMsg;

        if (ctrlMsg.ParseFromArray(msg->getPayloadData(),
                                   msg->getMessageLength()))
        {
            switch (ctrlMsg.id())
            {
            case Protocol::Ctrl::PROT_HNDLR_CTRL_CMD_HEARTBEAT_REQUEST_MSG_ID:
                sendHeartbeatResponse();
                break;

            case Protocol::Ctrl::PROT_HNDLR_CTRL_EVT_HEARTBEAT_RESPONSE_MSG_ID:
            case Protocol::Ctrl::PROT_HNDLR_CTRL_CMD_TERMINATE_CONNECTION_ID:
            case Protocol::Ctrl::PROT_HNDLR_CTRL_CMD_GET_VERSION_INFO_ID:
                Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().error(
                    Poco::format("No Handler for the event %d",
                                 ctrlMsg.id()));
                break;

            default:
                break;
            }
        }
    }
    if (msg != NULL)
    {
        delete msg;
        msg = NULL;
    }
}

void ComController::sendHeartbeatResponse() {
    Protocol::Ctrl::ProtHandlerCtrlMessage ctrlMsg;
    ctrlMsg.set_id(
        Protocol::Ctrl::PROT_HNDLR_CTRL_EVT_HEARTBEAT_RESPONSE_MSG_ID);

    sendProtHndlrControlMessageToHost(ctrlMsg);

}
void ComController::dispatchCommunicatorEvent(CMessage* event) {
    if (0 != event)
    {
        switch (event->getType())
        {
        case Communicator::NTFY_NEW_HOST_CONNECTION_ESTABLISHED:
            {
                onClientConnected(event);
            }
            break;

        case Communicator::NTFY_NEW_MESSAGE_FROM_HOST_RECEIVED:
            {
                onMessageReceived(event->getMessage());
            }
            break;

        case Communicator::NTFY_SOCKET_CONNECTION_CLOSED_BY_HOST:
            oConnectionClosedbyClient();
            break;

        default:
            break;
        }
    }
}

void ComController::onClientConnected(CMessage* ntfy) {
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().trace(
        "onClientConnected begin");
    messageHandler = new CCommMsgDispatcher(eventQueue);
    sendChronographCalibrationTimestampMessage();
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getPluginManager()->onHostConnectionEstablished();
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().trace(
        "onClientConnected end");
}

void ComController::oConnectionClosedbyClient() {
    if (messageHandler != NULL)
    {
        // observer.stop();
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getPluginManager()->onHostConnectionLost();
        messageHandler->closeConnection();
        delete messageHandler;
        messageHandler = NULL;
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().error(
            "Connection closed");
        comServer.signalNextServerRun();
    }
}

void ComController::sendChronographCalibrationTimestampMessage() {
    const PluginInterface::ProtocolMessage* msg =
        new PluginInterface::ProtocolMessage(
            Protocol::CommonDefinitions::MSG_TYPE_CHRONOGRAPH_CALIBRATION,
            0, NULL, true);
    messageHandler->sendMessage(msg);
}

}
;
}
;

