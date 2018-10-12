/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/


#include <iostream>

#include "log4jPlugin.hpp"
#include "log4jReader.hpp"

extern "C" void* createPluginInstance(
        const CMessageDispatcher* const senderHandle,
        const CTimestampProvider* tsProvider) {
    return new TargetAgentlog4jPlugin::Clog4jPlugin(senderHandle, tsProvider);
}

namespace TargetAgentlog4jPlugin {

using namespace TargetAgent;

Clog4jPlugin::Clog4jPlugin(const CMessageDispatcher* const senderHandle,
                           const CTimestampProvider* tsProvider) :
        mMsgSenderHDL(senderHandle), mTimestampProvider(tsProvider), messageTypeSocketReader(
    Protocol::CommonDefinitions::MSG_TYPE_LOG4J_PLUGIN), logger(0) {
    logger = &(Poco::Logger::get("TargetAgent.Clog4jPlugin")); // inherits configuration from Target Agent
}

Clog4jPlugin::~Clog4jPlugin() {
}

bool Clog4jPlugin::setConfig(
    const std::map<std::string, std::string>& pluginConfiguration) {
    for (std::map<std::string, std::string>::const_iterator it =
             pluginConfiguration.begin(); it != pluginConfiguration.end();
         ++it)
    {
        logger->information(
            Poco::format("key %s value %s", it->first, it->second));
        if (std::string::npos != (it->first).find("log4jLogFile"))
        {

            std::string logFileName = it->second;
            logger->information(
                Poco::format("key %s value %s", it->first, logFileName));
            logReaders.push_back(new log4jReader(mMsgSenderHDL,mTimestampProvider,logFileName));
        }
    }
    logger->warning("setConfig");
    return true;
}

void Clog4jPlugin::onMessageReceived(int payloadLength,
                                     const unsigned char* payloadBuffer) {

}

Protocol::CommonDefinitions::MessageType Clog4jPlugin::MessageType() {
    return messageTypeSocketReader;
}

void Clog4jPlugin::onConnectionEstablished() {
}

void Clog4jPlugin::onConnectionLost() {
    logger->warning(
        "Connection Lost: trigger metadata dispatch, currently none");
}

bool Clog4jPlugin::startPlugin() {
    logger->warning("startPlugin");
    for (std::vector<log4jReader*>::const_iterator it = logReaders.begin();
         it != logReaders.end(); ++it)
    {
        (*it)->start();
    }
    return true;
}

bool Clog4jPlugin::stopPlugin() {
    logger->warning("stopPlugin");
    for (std::vector<log4jReader*>::const_iterator it = logReaders.begin();
         it != logReaders.end(); ++it)
    {
        (*it)->stop();
    }
    return true;
}

}
;
