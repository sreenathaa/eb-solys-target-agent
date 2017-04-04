/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#ifndef SOCKET_READER_ACTIVITY
#define SOCKET_READER_ACTIVITY

#include <iostream>

#include "Poco/Activity.h"
#include "Poco/Thread.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/SocketStream.h"
#include <Poco/Net/DatagramSocket.h>
#include "Poco/Condition.h"
#include <Poco/RegularExpression.h>
#include "Poco/Logger.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/NumberParser.h"
#include "Poco/NObserver.h"
#include "Poco/Util/ServerApplication.h"
#include "target_agent_prot_socket_reader_plugin.pb.h"
#include "TargetAgentDispatcherInterface.h"

namespace TargetAgentLog4jPlugin
{

using Poco::Net::SocketReactor;
using Poco::Net::SocketAcceptor;
using Poco::Net::ReadableNotification;
using Poco::Net::WritableNotification;
using Poco::Net::ShutdownNotification;
using Poco::Net::ServerSocket;
using Poco::Net::StreamSocket;
using Poco::NObserver;
using Poco::AutoPtr;
using Poco::Thread;
using Poco::Util::Application;


class CSocketReaderServer

{
public:
    CSocketReaderServer(StreamSocket& socket, SocketReactor& reactor);

    ~CSocketReaderServer();

    void onSocketReadable(const AutoPtr<ReadableNotification>& pNf);

    void onSocketWritable(const AutoPtr<WritableNotification>& pNf);

    void onSocketShutdown(const AutoPtr<ShutdownNotification>& pNf);

    void sendRawTMessage(
        TargetAgent::Protocol::SocketReader::SocketReaderMessage* message);
    void setMessageSender(const TargetAgent::PluginInterface::CMessageDispatcher* const msgSenderHDL){
        mMsgSenderHDL = const_cast<TargetAgent::PluginInterface::CMessageDispatcher*>(msgSenderHDL);
    }
private:
    void resetCounters(void);
    void extractHeaderContent(void);
    void dispatchMessage(void);
private:
    static const unsigned int BUFFER_SIZE = 4096;
    static const unsigned int HEADER_LENGTH_SIZE = 53;
    StreamSocket _socket;
    SocketReactor& _reactor;
    TargetAgent::Protocol::SocketReader::SocketReaderMessageType messageIdentifier;
    char payloadbuffer[BUFFER_SIZE];
    unsigned int bytesAlreadyReceived;
    unsigned int payloadLength;
    unsigned int expectedAmountOfBytes;
    TargetAgent::PluginInterface::CMessageDispatcher * mMsgSenderHDL;
};

class CMessageSender: public SocketAcceptor<CSocketReaderServer> {
public:
    CMessageSender(ServerSocket& socket, SocketReactor& reactor,
                   const TargetAgent::PluginInterface::CMessageDispatcher * const msgSenderHDL);

    CSocketReaderServer* createServiceHandler(StreamSocket& socket);

public:
    const TargetAgent::PluginInterface::CMessageDispatcher * const mMsgSenderHDL;
};




}
;

#endif
