/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "socket_reader_activity.h"
#include "target_agent_prot_socket_reader_plugin.pb.h"
#include <Poco/Net/NetException.h>
#include <socket_reader_plugin.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Poco/Base64Encoder.h"
#include "Poco/Base64Decoder.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <google/protobuf/text_format.h>

using namespace std;
using namespace Poco::Net;
using Poco::Base64Encoder;
using Poco::Base64Decoder;
using namespace TargetAgent::Protocol::SocketReader;

namespace TargetAgentLog4jPlugin
{


CSocketReaderServer::CSocketReaderServer(StreamSocket& socket,
        SocketReactor& reactor):_socket(socket), _reactor(reactor), messageIdentifier(TargetAgent::Protocol::SocketReader::TARGET_AGENT_MESSAGE),
        bytesAlreadyReceived(0),payloadLength(0),expectedAmountOfBytes(HEADER_LENGTH_SIZE)
{
    _reactor.addEventHandler(_socket,
                             NObserver<CSocketReaderServer, ReadableNotification>(*this,
                                     &CSocketReaderServer::onSocketReadable));
    _reactor.addEventHandler(_socket,
                             NObserver<CSocketReaderServer, ShutdownNotification>(*this,
                                     &CSocketReaderServer::onSocketShutdown));

}



CSocketReaderServer::~CSocketReaderServer()
{

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
                                NObserver<CSocketReaderServer, ReadableNotification>(*this,
                                        &CSocketReaderServer::onSocketReadable));
    _reactor.removeEventHandler(_socket,
                                NObserver<CSocketReaderServer, ShutdownNotification>(*this,
                                        &CSocketReaderServer::onSocketShutdown));

}

void CSocketReaderServer::resetCounters()
{
    bytesAlreadyReceived = 0;
    payloadLength = 0;
    expectedAmountOfBytes = HEADER_LENGTH_SIZE;
}


void CSocketReaderServer::onSocketReadable(
    const AutoPtr<ReadableNotification>& pNf)
{
    unsigned int crtPostion = bytesAlreadyReceived;

    unsigned int crtRead  = expectedAmountOfBytes- bytesAlreadyReceived;

    /*buffer overflow*/
    if(expectedAmountOfBytes>=BUFFER_SIZE)
    {
        crtPostion = 0;
        crtRead = (expectedAmountOfBytes- bytesAlreadyReceived)>BUFFER_SIZE?BUFFER_SIZE: (expectedAmountOfBytes- bytesAlreadyReceived);
    }

    if (_socket.available())
    {

        int count = _socket.receiveBytes(
                        (void*) (&payloadbuffer[crtPostion]),
                        crtRead);

        if ((count < 0))
        {
            resetCounters();

        }
        else
        {

            bytesAlreadyReceived += count;
            payloadbuffer[crtPostion+count] = '\0';
            extractHeaderContent();
            dispatchMessage();

        }

    }
}
void CSocketReaderServer::dispatchMessage(void)
{

    if (bytesAlreadyReceived  && (bytesAlreadyReceived >= expectedAmountOfBytes))
    {

        TargetAgent::Protocol::SocketReader::SocketReaderMessage message;
        message.set_type(messageIdentifier);
        TargetAgent::Protocol::SocketReader::SocketReaderInnerMessage* inner =
            message.mutable_message();
        inner->set_portno(1);
        unsigned int headerOffset = payloadLength?HEADER_LENGTH_SIZE:0;
        inner->set_data(std::string(payloadbuffer+headerOffset));
        message.set_encoding(Encoding_PlainAscii);
        sendRawTMessage(&message);
        resetCounters();
    }
}

void CSocketReaderServer::sendRawTMessage(

    TargetAgent::Protocol::SocketReader::SocketReaderMessage* message)
{
    unsigned char* payload = new unsigned char[message->ByteSize()];

    if (message->SerializeToArray((void*) payload, message->ByteSize()))
    {
        mMsgSenderHDL->sendMessage(Protocol::CommonDefinitions::MSG_TYPE_SOCKET_READER_PLUGIN,
                                   message->ByteSize(), payload);

    }
    delete[] payload;

}
void CSocketReaderServer::extractHeaderContent(void)
{
    if ((bytesAlreadyReceived >= HEADER_LENGTH_SIZE) && !payloadLength)
    {
        TargetAgent::Protocol::SocketReader::Header header;
        if(google::protobuf::TextFormat::ParseFromString(
               std::string(payloadbuffer, HEADER_LENGTH_SIZE), &header))
        {
            payloadLength = header.payloadlength();
            expectedAmountOfBytes += payloadLength;
            messageIdentifier = header.messagetype();
        }
    }
}
void CSocketReaderServer::onSocketWritable(
    const AutoPtr<WritableNotification>& pNf)
{
}

void CSocketReaderServer::onSocketShutdown(
    const AutoPtr<ShutdownNotification>& pNf)
{
    delete this;
}

CMessageSender::CMessageSender(ServerSocket& socket, SocketReactor& reactor,
                               const TargetAgent::PluginInterface::CMessageDispatcher * const msgSenderHDL) :
        SocketAcceptor<CSocketReaderServer>(socket, reactor), mMsgSenderHDL(
    msgSenderHDL) {

}

CSocketReaderServer* CMessageSender::createServiceHandler(
    StreamSocket& socket) {

    CSocketReaderServer *ret = new CSocketReaderServer(socket, *reactor());
    ret->setMessageSender(mMsgSenderHDL);
    return ret;
}

;
}
