/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include <sstream>
#include <string>
#include <fstream>

#include "Poco/Util/ServerApplication.h"
#include "Poco/NumberFormatter.h"

#include "target_agent_version.hpp"
#include "CConnector.hpp"


using Poco::Net::StreamSocket;
using Poco::Net::SocketConnector;
using Poco::Net::SocketReactor;
using Poco::Util::Application;
using Poco::Observer;
using Poco::Net::WritableNotification;
using Poco::Net::TimeoutNotification;
using Poco::Net::ErrorNotification;
using Poco::AutoPtr;
using Poco::Net::ShutdownNotification;
using Poco::Net::SocketAddress;
using Poco::Thread;
using Poco::Notification;
using Poco::NumberFormatter;

using namespace std;

namespace TargetAgent
{
namespace logger
{


CLoggerServiceHandler::CLoggerServiceHandler(StreamSocket& socket, SocketReactor& reactor):
        _socket(socket),
        _reactor(reactor),
        _pBuffer(new char[BUFFER_SIZE]),metaDataQueue(0)
{
    _reactor.addEventHandler(_socket, Poco::Observer<CLoggerServiceHandler, WritableNotification>(*this, &CLoggerServiceHandler::onWritable));
    _reactor.addEventHandler(_socket, Poco::Observer<CLoggerServiceHandler, ErrorNotification>(*this, &CLoggerServiceHandler::onError));
}

CLoggerServiceHandler::~CLoggerServiceHandler()
{
    _reactor.removeEventHandler(_socket, Poco::Observer<CLoggerServiceHandler, WritableNotification>(*this, &CLoggerServiceHandler::onWritable));
    _reactor.removeEventHandler(_socket, Poco::Observer<CLoggerServiceHandler, ErrorNotification>(*this, &CLoggerServiceHandler::onError));
    delete [] _pBuffer;
}

void CLoggerServiceHandler::onError(ErrorNotification* pNotification)
{
}

void CLoggerServiceHandler::setQueue(Poco::NotificationQueue* queue)
{
    metaDataQueue = queue;
    sendXMLConfiguration();
}

void CLoggerServiceHandler::onTimeout(TimeoutNotification* pNf)
{
}

void CLoggerServiceHandler::sendXMLConfiguration(void)
{

    std::ifstream infile("conf.xml");

    std::string line;

    while (std::getline(infile, line))
    {
        buildMessage(line.c_str(),line.length());
    }

    sendVersionInformation();
}

void CLoggerServiceHandler::sendVersionInformation(){
    std::string taVer("Target Agent Version: "+std::string(TargetAgent::Version::targetAgentVersion));
    std::string protVer("SOLYS->TA Protocol Version: ");
    protVer.append(Poco::NumberFormatter::format(TargetAgent::Version::raceTaProtocolVersion));
    buildMessage(taVer.c_str(),taVer.length());
    buildMessage(protVer.c_str(),protVer.length());
}

void CLoggerServiceHandler::buildMessage(const char* message, unsigned int  messageLength)
{
    //header {  payloadLength:    5  messageType: TARGET_AGENT_MESSAGE }

    std::string headerInProtoTextFormat;
    headerInProtoTextFormat += "payloadLength: ";
    headerInProtoTextFormat += Poco::format("%4u", messageLength);
    headerInProtoTextFormat+=" messageType: TARGET_AGENT_MESSAGE";
    writeNBytesToSocket(headerInProtoTextFormat.c_str(), headerInProtoTextFormat.length());
    writeNBytesToSocket(message, messageLength);
}

void CLoggerServiceHandler::onWritable(WritableNotification* pNf)
{

    AutoPtr<Notification> notification(metaDataQueue->waitDequeueNotification());
    while (notification)
    {
        CLogNotification* pWorkNf =
            dynamic_cast<CLogNotification*>(notification.get());
        if (pWorkNf)
        {
            buildMessage(pWorkNf->data().getText().c_str(),pWorkNf->data().getText().length());
        }
        notification = metaDataQueue->waitDequeueNotification();
    }
}

bool CLoggerServiceHandler::writeNBytesToSocket(const char* bytes, unsigned int bytesToWrite)
{
    unsigned int tmp = 0;
    try
    {
        while (bytesToWrite > tmp)
        {
            tmp += _socket.sendBytes(bytes + tmp, bytesToWrite - tmp);
        }
        // _socket.sendBytes('\0', 1);
    }
    catch (Poco::Exception& error)
    {
    }
    return true;
}
void CLoggerServiceHandler::onShutdown(const AutoPtr<ShutdownNotification>& pNf)
{
    delete this;
}



CSocketConnector::CSocketConnector(SocketAddress& sa, SocketReactor& reactor,  Poco::NotificationQueue* queue):
        SocketConnector<CLoggerServiceHandler>(sa, reactor), metaDataQueue(queue)
{
    reactor.addEventHandler(socket(), Observer<CSocketConnector, WritableNotification>(*this, &CSocketConnector::onReadable));
}

CLoggerServiceHandler* CSocketConnector::createServiceHandler()
{
    CLoggerServiceHandler *ret = new CLoggerServiceHandler(dynamic_cast<Poco::Net::StreamSocket& >(socket()), *reactor());
    ret->setQueue(metaDataQueue);
    return ret;
}

void CSocketConnector::onReadable(WritableNotification* r)
{
}

void CSocketConnector::onError(int errorCode)
{
}

}
}
