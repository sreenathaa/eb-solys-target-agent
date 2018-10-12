/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifdef _WIN32
	#include <winsock2.h> 
	#include <ws2tcpip.h>
	#include <windows.h>
#endif
 
#include <iostream>
#include "Poco/NumberParser.h"
#include "networkPacketSnifferPlugin.hpp"


using Poco::NumberParser;

extern "C" LIBRARY_API void* createPluginInstance(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider) {
    return new TargetAgentnetworkPacketSnifferPlugin::CnetworkPacketSnifferPlugin(senderHandle, tsProvider);
}


namespace TargetAgentnetworkPacketSnifferPlugin {

using namespace TargetAgent;

CnetworkPacketSnifferPlugin::CnetworkPacketSnifferPlugin(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider) :mMsgSenderHDL(senderHandle),mTimestampProvider(tsProvider),
        messageTypeSocketReader(
            Protocol::CommonDefinitions::MSG_TYPE_NETWORKPACKETSNIFFER_PLUGIN), logger(
        0),sniffer(0) {
    logger = &(Poco::Logger::get("TargetAgent.CnetworkPacketSnifferPlugin")); // inherits configuration from Target Agent
}

CnetworkPacketSnifferPlugin::~CnetworkPacketSnifferPlugin() {
}

bool CnetworkPacketSnifferPlugin::setConfig(
    const std::map<std::string, std::string>& pluginConfiguration) {
    std::string networkInterface;
    std::vector<int> ports;
    std::string protocolType = "SOMEIP";
    std::string payloadType = "UDP";


    typedef std::map<std::string, std::string>::const_iterator it_type;

    for (it_type it = pluginConfiguration.begin(); it != pluginConfiguration.end(); ++it){
        logger->information(
            Poco::format("key %s value %s", it->first, it->second));
        if (std::string::npos != (it->first).find("NetworkInterface"))
        {
            networkInterface = it->second;
        }
        else
		if (std::string::npos != (it->first).find("PortNr"))
        {
			int port = atoi( it->second.c_str() );
            ports.push_back(port);
        }
		else if (std::string::npos != (it->first).find("PayloadType"))
		{
			payloadType= it->second;
		}
		else if (std::string::npos != (it->first).find("ProtocolType"))
		{
			protocolType= it->second;
		}

   }

    sniffer = new CPacketSniffer(mMsgSenderHDL,networkInterface,ports, payloadType, protocolType);
    return true;
}

void CnetworkPacketSnifferPlugin::onMessageReceived(int payloadLength,
        const unsigned char* payloadBuffer) {

}

Protocol::CommonDefinitions::MessageType CnetworkPacketSnifferPlugin::MessageType() {
    return messageTypeSocketReader;
}

void CnetworkPacketSnifferPlugin::onConnectionEstablished() {
}

void CnetworkPacketSnifferPlugin::onConnectionLost() {
    logger->information(
        "Connection Lost: trigger metadata dispatch, currently none");
}

bool CnetworkPacketSnifferPlugin::startPlugin() {
    logger->information("startPlugin");
    sniffer->start();
    return true;
}

bool CnetworkPacketSnifferPlugin::stopPlugin() {
    logger->information("stopPlugin");
    sniffer->stop();
    return true;
}

}
;
