/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef COM_CONTROLLER_H_
#define COM_CONTROLLER_H_

#include <iostream>
#include <memory>
#include <map>
#include <list>

#include "Poco/NotificationQueue.h"
#include "Poco/Activity.h"
#include "Poco/Mutex.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/SocketAcceptor.h"

#include "CComServer.h"
#include "CMessageDispatcher.h"
#include "CMessage.h"
#include "TargetAgentDispatcherInterface.h"
#include "TargetAgentPluginInterface.h"
#include "plugin_notifier.h"
#include "target_agent_prot_ctrl.pb.h"

#include "CommandControlServer.hpp"



namespace TargetAgent {
namespace Communicator {

class ComController {

public:
    ComController();
    ~ComController();

    void start();
    void stop();

    void sendMessage(const PluginInterface::ProtocolMessage* msg);
    inline void setStreamSocket(Poco::Net::StreamSocket *socket) {
        mSocket = socket;
    }
    inline Poco::Net::StreamSocket*  getStreamSocket( void) {
        return mSocket;
    }

protected:
    void runEventLoop();

private:
    void dispatchCommunicatorEvent(CMessage* event);
    void sendVersionInformation();
    void sendProtHndlrControlMessageToHost(
        Protocol::Ctrl::ProtHandlerCtrlMessage& ctrlMsg);
    void adjustMessagePayloadBufferSize(int currentMessagePayloadSize);
    void onClientConnected(CMessage* ntfy);
    void onMessageReceived(const PluginInterface::ProtocolMessage* msg);
    void onNewHostMessageReceivedNotification(CMessage* ntfy);
    void oConnectionClosedbyClient();
    void onWatchdogTimeoutNotification();
    void sendHeartbeatResponse();
    void sendChronographCalibrationTimestampMessage();


    ComController(const ComController& that);
    const ComController& operator=(const ComController& rhs);

private:
    Poco::NotificationQueue eventQueue;
    Poco::Mutex eventLoopMutex;
    Poco::Net::StreamSocket* mSocket;
    Poco::Activity<ComController> eventLoop;
    CCommMsgDispatcher* messageHandler;
    CComServer comServer;
    unsigned char* messagePayloadBuffer;
    int allocatedMessagePayloadBufferSize;
    Poco::Net::ServerSocket commandControlPort;
    Poco::Net::SocketReactor reactor;
    Poco::Thread reactorThread;
    Poco::Net::SocketAcceptor<CCommanControlServer> *connectionAcceptor;
};

}
;
}
;

#endif /* COM_CONTROLLER_H_ */
