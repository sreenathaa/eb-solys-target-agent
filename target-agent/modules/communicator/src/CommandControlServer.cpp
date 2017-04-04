/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#include "Poco/NotificationQueue.h"

#include "protocol_message.h"
#include "CMessage.h"
#include "CommandControlServer.hpp"
#include <stdio.h>
namespace TargetAgent {
namespace Communicator {



CCommanControlServer::CCommanControlServer(StreamSocket& socket,
        SocketReactor& reactor) :
        _socket(socket), _reactor(reactor), bytesAlreadyReceived(0), totalAmountOfbytes(
            0), payloadLength(0),headerLength(0), cmdId(0), payloadbuffer(),payloadbufferSize(0), metaDataQueue(), app(
        Application::instance()) {

    payloadbuffer = (char*)malloc(sizeof(char)*BUFFER_SIZE);

    if(0 == payloadbuffer)
    {
        app.logger().error("failed to allocate heap memory");
    }
    else
        payloadbufferSize = BUFFER_SIZE;

    app.logger().information(
        "Connection from " + socket.peerAddress().toString());

    _reactor.addEventHandler(_socket,
                             NObserver<CCommanControlServer, ReadableNotification>(*this,
                                     &CCommanControlServer::onSocketReadable));
    _reactor.addEventHandler(_socket,
                             NObserver<CCommanControlServer, ShutdownNotification>(*this,
                                     &CCommanControlServer::onSocketShutdown));

}

void CCommanControlServer::setQueue(Poco::NotificationQueue* queue) {
    metaDataQueue = queue;
}

CCommanControlServer::~CCommanControlServer() {
    Application& app = Application::instance();
    try
    {
        app.logger().information(
            "Disconnecting " + _socket.peerAddress().toString());
    }
    catch (...)
    {
    }
    _reactor.removeEventHandler(_socket,
                                NObserver<CCommanControlServer, ReadableNotification>(*this,
                                        &CCommanControlServer::onSocketReadable));
    _reactor.removeEventHandler(_socket,
                                NObserver<CCommanControlServer, ShutdownNotification>(*this,
                                        &CCommanControlServer::onSocketShutdown));

}

void CCommanControlServer::resetCounters() {

    totalAmountOfbytes = 0;
    bytesAlreadyReceived = 0;
    payloadLength = 0;
    headerLength = 0;
    char * tmpBuf = (char*)realloc(payloadbuffer,BUFFER_SIZE*sizeof(char));
    if(0 != tmpBuf)
    {
        payloadbuffer = tmpBuf;
        payloadbufferSize = BUFFER_SIZE;
    }
    else
        app.logger().error("realloc failed");
}

void CCommanControlServer::extractHeaderLength(void) {
    if (bytesAlreadyReceived > HEADER_LENGTH_SIZE && !headerLength)
    {
        /*we got the length*/
        int successfulReads = sscanf(payloadbuffer, "%x", &headerLength);
        app.logger().error(Poco::format("successfulReads %d headerLength %u",
                                        successfulReads, headerLength));
    }
}

void CCommanControlServer::extractHeaderContent() {
    if ((bytesAlreadyReceived
         > (HEADER_LENGTH_SIZE + DELIMITATOR_SIZE + headerLength)
         && !payloadLength))
    {
        /*we have a complete header*/
        /*length: 4677 type: 1 timestamp: 15314117 versionToken: 3043 */
        unsigned long int ts;
        unsigned int ver;
        app.logger().error(Poco::format("received %s",
                                        std::string(
                                            payloadbuffer
                                            + (HEADER_LENGTH_SIZE + DELIMITATOR_SIZE))));
        int successfulReads = sscanf(
                                  payloadbuffer + (HEADER_LENGTH_SIZE + DELIMITATOR_SIZE),
                                  "length: %u type: %u timestamp: %lu versionToken: %u",
                                  &payloadLength, &cmdId, &ts, &ver);

        if(successfulReads <2)
        {
            app.logger().error(Poco::format("invalid request; mandatory fields  not provided: length type %d",successfulReads));
            resetCounters();
        }
        totalAmountOfbytes = HEADER_LENGTH_SIZE + DELIMITATOR_SIZE
                             + headerLength + payloadLength;

        if(totalAmountOfbytes > BUFFER_SIZE)
        {
            char * tmp = (char*)realloc(payloadbuffer,sizeof(char)*totalAmountOfbytes);
            if(0 != tmp)
            {
                payloadbuffer = tmp;
                payloadbufferSize = totalAmountOfbytes;
            }
        }


        app.logger().error(Poco::format(
                               "successfulReads %d payloadLength %u cmdId %u ts %u ver %u",
                               successfulReads, payloadLength, cmdId, ts, ver));
    }
}

void CCommanControlServer::dispatchMessage() {

    if (bytesAlreadyReceived && (bytesAlreadyReceived >= totalAmountOfbytes))
    {

        app.logger().error(Poco::format("received %u / expected %u ", bytesAlreadyReceived,
                                        totalAmountOfbytes));

        app.logger().error(Poco::format("payload %s",
                                        std::string(
                                            &payloadbuffer[HEADER_LENGTH_SIZE + 2 * DELIMITATOR_SIZE
                                                           + headerLength], payloadLength)));

        PluginInterface::ProtocolMessage* msg =
            new PluginInterface::ProtocolMessage(
                static_cast<TargetAgent::Protocol::CommonDefinitions::MessageType>(cmdId),
                payloadLength,
                (unsigned char*) &payloadbuffer[HEADER_LENGTH_SIZE
                                                + 2 * DELIMITATOR_SIZE + headerLength]);

        CMessage* abc = new CMessage(NTFY_NEW_MESSAGE_FROM_HOST_RECEIVED, msg);
        metaDataQueue->enqueueNotification(abc);

        resetCounters();
    }
}
void CCommanControlServer::onSocketReadable(
    const AutoPtr<ReadableNotification>& pNf) {

    if (_socket.available())
    {

        int count = _socket.receiveBytes(
                        (void*) (&payloadbuffer[bytesAlreadyReceived]),
                        payloadbufferSize - bytesAlreadyReceived);

        if ((count < 0))
        {
            resetCounters();

        }
        else
        {

            bytesAlreadyReceived += count;
            payloadbuffer[bytesAlreadyReceived] = '\0';
            app.logger().error(Poco::format("%u bytes received", bytesAlreadyReceived));

            extractHeaderLength();
            extractHeaderContent();
            dispatchMessage();

        }

    }
    else
    {
        delete this;
    }
}

void CCommanControlServer::onSocketWritable(
    const AutoPtr<WritableNotification>& pNf) {

}

void CCommanControlServer::onSocketShutdown(
    const AutoPtr<ShutdownNotification>& pNf) {
}

CCSocketAcceptor::CCSocketAcceptor(ServerSocket& socket, SocketReactor& reactor,
                                   Poco::NotificationQueue* queue) :
        SocketAcceptor<CCommanControlServer>(socket, reactor), metaDataQueue(
    queue) {

}

CCommanControlServer* CCSocketAcceptor::createServiceHandler(
    StreamSocket& socket) {
    CCommanControlServer *ret = new CCommanControlServer(socket, *reactor());
    ret->setQueue(metaDataQueue);
    return ret;
}

}
}


