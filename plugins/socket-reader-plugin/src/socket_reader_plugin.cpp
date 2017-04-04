/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#include <iostream>

#include "Poco/NumberParser.h"

#include "socket_reader_plugin.h"
#include <google/protobuf/text_format.h>


extern "C" LIBRARY_API void* createPluginInstance(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider)
{
    return new TargetAgentLog4jPlugin::CSocketReaderPlugin(senderHandle, tsProvider);
}

namespace TargetAgentLog4jPlugin
{

using namespace TargetAgent;

CSocketReaderPlugin::CSocketReaderPlugin(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider) :
        mMsgSenderHDL(senderHandle),mTimestampProvider(tsProvider),
        messageTypeSocketReader(
            Protocol::CommonDefinitions::MSG_TYPE_SOCKET_READER_PLUGIN), logger(
                0)
{
    logger = &(Poco::Logger::get("TargetAgent.CSocketReaderPlugin")); // inherits configuration from Target Agent
    ports.push_back(1223);
}

CSocketReaderPlugin::~CSocketReaderPlugin()
{
}

bool CSocketReaderPlugin::setConfig(
    const std::map<std::string, std::string>& pluginConfiguration)
{
    for (std::map<std::string, std::string>::const_iterator it=pluginConfiguration.begin(); it!=pluginConfiguration.end(); ++it)
    {
        logger->information(
            Poco::format("key %s value %s", it->first, it->second));
        if (std::string::npos != (it->first).find("portNumber"))
        {

            unsigned int portNumer = Poco::NumberParser::parseUnsigned(
                                         it->second);
            logger->error(
                Poco::format("key %s value %u", it->first, portNumer));
            ports.push_back(portNumer);
        }
    }

    return true;
}

void CSocketReaderPlugin::onMessageReceived(int payloadLength,
        const unsigned char* payloadBuffer)
{
}

Protocol::CommonDefinitions::MessageType CSocketReaderPlugin::MessageType()
{
    return messageTypeSocketReader;
}


bool CSocketReaderPlugin::shutdownPlugin()
{
    return true;
}

void CSocketReaderPlugin::onConnectionEstablished()
{
}

void CSocketReaderPlugin::onConnectionLost()
{
    logger->warning(
        "Connection Lost: trigger metadata dispatch, currently none");
}

bool CSocketReaderPlugin::startPlugin()
{
    for (std::list<unsigned int>::iterator it = ports.begin(); it != ports.end(); it++)
    {
        ServerSocket *internalCommunication = new ServerSocket(*it);
        svSockets.push_back(internalCommunication);
        SocketReactor *reactor = new SocketReactor();
        reactors.push_back(reactor);
        SocketAcceptor<CSocketReaderServer>* acceptor = new CMessageSender(*internalCommunication, *reactor,mMsgSenderHDL);
        acceptors.push_back(acceptor);
        Thread *thr = new Thread();
        threads.push_back(thr);
        thr->start(*reactor);
    }

    return true;
}

bool CSocketReaderPlugin::stopPlugin()
{
    logger->information("CSocketReaderPlugin::stopPlugin begin");

    while(!svSockets.empty())
    {
        reactors.front()->stop();
        threads.front()->join();


        delete svSockets.front();
        svSockets.pop_front();
        delete reactors.front();
        reactors.pop_front();
        delete acceptors.front();
        acceptors.pop_front();

        delete threads.front();
        threads.pop_front();
    }

    logger->information("CSocketReaderPlugin::stopPlugin end");
    return true;
}

}
;
