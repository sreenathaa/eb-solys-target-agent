/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "CComServer.h"
#include "CMessage.h"
#include "CLogger.hpp"
#include "Poco/Net/ServerSocket.h"
#include "Poco/ScopedLock.h"
#include "Poco/Net/NetException.h"
#include "Poco/Thread.h"
#include <CMediator.h>
#include <iostream>

namespace TargetAgent {
namespace Communicator {

CComServer::CComServer(Poco::NotificationQueue& notificationQueue) :
        port(0), mSocket(), notificationQueue(notificationQueue), connectionMutex(), connectionCond(), connectionPredicate(
    false), comServerActivity(this, &CComServer::runComServer) {

}

CComServer::~CComServer() {

}

void CComServer::start(Poco::UInt16 port) {
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().warning(Poco::format("Communication server bind on port %hu",port));
    this->port = port;
    comServerActivity.start();
}

void CComServer::stop() {
    comServerActivity.stop();
    wakeUpComServerActivity();
    comServerActivity.wait();
}

void CComServer::signalNextServerRun() {
    wakeUpComServerActivity();
}

void CComServer::runComServer() {

#ifdef _POSIX_THREADS
    if (0 != pthread_setname_np(pthread_self(), "CCommServer"))
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().error("failed to set thread name for CCommServer");
    }
#endif

    try
    {
        Poco::Net::ServerSocket server(port);

        while (!comServerActivity.isStopped())
        {
            Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().warning("Waiting for client to connect");
            mSocket = server.acceptConnection();
            Mediator::CTargetAgentRuntime::getUniqueInstance()->getComController()->setStreamSocket(
                &mSocket);
            Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().warning("Connection established");
            notifyNewHostConnection();
            waitForCurrentConnectionExpires();
        }

        server.close();
    }
    catch (const Poco::Net::NetException& e)
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().log(e);
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().error("Another connection already open ?");
    }
    catch (Poco::Exception& e)
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().log(e);
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().error("Another Exception Occured in CComServer::runComServer()");
    }
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().warning("Target Agent Server Closed");
}

void CComServer::notifyNewHostConnection() {
    CMessage* notification = new CMessage(NTFY_NEW_HOST_CONNECTION_ESTABLISHED,
                                          0);
    notificationQueue.enqueueNotification(notification);
}

void CComServer::waitForCurrentConnectionExpires() {
    Poco::Mutex::ScopedLock lock(connectionMutex);
    {
        connectionPredicate = false;
        while (!connectionPredicate)
        {
            connectionCond.wait(connectionMutex);
        }
    }
}

void CComServer::wakeUpComServerActivity() {
    Poco::Mutex::ScopedLock lock(connectionMutex);
    {
        connectionPredicate = true;
        connectionCond.signal();
    }
}

}
;
}
;

