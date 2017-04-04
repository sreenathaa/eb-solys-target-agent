/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef TARGET_AGENT_MODULES_COMMUNICATOR_INC_COMMANDCONTROLSERVER_HPP_
#define TARGET_AGENT_MODULES_COMMUNICATOR_INC_COMMANDCONTROLSERVER_HPP_

#include <google/protobuf/text_format.h>

#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/NumberParser.h"
#include "Poco/NObserver.h"
#include "Poco/Util/ServerApplication.h"

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

namespace TargetAgent
{
namespace Communicator
{

class CCommanControlServer

{
public:
    CCommanControlServer(StreamSocket& socket, SocketReactor& reactor);

    void setQueue(Poco::NotificationQueue* queue);

    ~CCommanControlServer();

    void resetCounters();

    void extractHeaderLength(void);

    void extractHeaderContent(void);

    void dispatchMessage(void);

    void onSocketReadable(const AutoPtr<ReadableNotification>& pNf);

    void onSocketWritable(const AutoPtr<WritableNotification>& pNf);

    void onSocketShutdown(const AutoPtr<ShutdownNotification>& pNf);
private:
    static const unsigned int BUFFER_SIZE = 1024*10;
    static const unsigned int HEADER_LENGTH_SIZE = 2;
    static const unsigned int DELIMITATOR_SIZE = 1;
    StreamSocket _socket;
    SocketReactor& _reactor;
    unsigned int bytesAlreadyReceived;
    unsigned int totalAmountOfbytes;
    unsigned int payloadLength;
    unsigned int headerLength;
    unsigned int cmdId;
    char *payloadbuffer;
    unsigned int payloadbufferSize;
    Poco::NotificationQueue* metaDataQueue;
    Application& app;
};


class CCSocketAcceptor: public SocketAcceptor<CCommanControlServer>
{
public:
    CCSocketAcceptor(ServerSocket& socket, SocketReactor& reactor,
                     Poco::NotificationQueue* queue);

    CCommanControlServer* createServiceHandler(StreamSocket& socket);

public:
    Poco::NotificationQueue* metaDataQueue;
};
}
}


#endif /* TARGET_AGENT_MODULES_COMMUNICATOR_INC_COMMANDCONTROLSERVER_HPP_ */
