/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#include <Poco/Logger.h>
#include "CLogger.hpp"
#include "CMediator.h"
#include "Poco/AsyncChannel.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/AutoPtr.h"
#include "Poco/SplitterChannel.h"
#include "Poco/SimpleFileChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FileChannel.h"
#include "Poco/NullChannel.h"

#include <list>
#include <algorithm>

using Poco::AsyncChannel;
using Poco::ConsoleChannel;
using Poco::AutoPtr;
using Poco::SplitterChannel;
using Poco::SimpleFileChannel;
using Poco::FormattingChannel;
using Poco::PatternFormatter;
using Poco::FileChannel;
using Poco::Logger;
using Poco::NullChannel;

Poco::FastMutex TargetAgent::logger::CSocketChannel::_mutex;

namespace TargetAgent {
namespace logger {



CLogger::CLogger() :
        taLogger(0), commLogger(0), configLogger(0), recLogger(0), pluginLogger(
    0), runtimeLogger(0), stringToEnum(),pSocket() {

    stringToEnum.insert(
        std::pair<std::string, Poco::Message::Priority>("PRIO_FATAL",
                Poco::Message::PRIO_FATAL));
    stringToEnum.insert(
        std::pair<std::string, Poco::Message::Priority>("PRIO_CRITICAL",
                Poco::Message::PRIO_CRITICAL));
    stringToEnum.insert(
        std::pair<std::string, Poco::Message::Priority>("PRIO_ERROR",
                Poco::Message::PRIO_ERROR));
    stringToEnum.insert(
        std::pair<std::string, Poco::Message::Priority>("PRIO_WARNING",
                Poco::Message::PRIO_WARNING));
    stringToEnum.insert(
        std::pair<std::string, Poco::Message::Priority>("PRIO_NOTICE",
                Poco::Message::PRIO_NOTICE));
    stringToEnum.insert(
        std::pair<std::string, Poco::Message::Priority>("PRIO_INFORMATION",
                Poco::Message::PRIO_INFORMATION));
    stringToEnum.insert(
        std::pair<std::string, Poco::Message::Priority>("PRIO_DEBUG",
                Poco::Message::PRIO_DEBUG));
    stringToEnum.insert(
        std::pair<std::string, Poco::Message::Priority>("PRIO_TRACE",
                Poco::Message::PRIO_TRACE));

    bool isAtLeastOneChannel = false;

    /*create and configure the two channels*/
    AutoPtr<ConsoleChannel> pCons(new ConsoleChannel);
    //pCons->setProperty("enableColors", "true");
    AutoPtr<FileChannel> pFile(new FileChannel("ta.log"));

    pFile->setProperty("rotation", "10 M");
    pFile->setProperty("archive", "timestamp");

    /*add them to a splitter channel*/

    AutoPtr<SplitterChannel> pSplitter(new SplitterChannel);

    Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->getLogChannels().size();


    if (isChannelActive("Console"))
    {

        pSplitter->addChannel(pCons);
        isAtLeastOneChannel = true;
    }


    if (isChannelActive("Socket"))
    {
        checkReaderAvailability();
        pSocket.assign(new CSocketChannel());
        pSplitter->addChannel(pSocket);
        isAtLeastOneChannel = true;
        thr.start(*pSocket);
    }

    if (isChannelActive("File"))
    {

        pSplitter->addChannel(pFile);

        isAtLeastOneChannel = true;
    }

    if (isAtLeastOneChannel)
    {
        AutoPtr<AsyncChannel> pAsync(new AsyncChannel(pSplitter));
        //"%d-%m-%Y %H:%M:%S: %t"
        AutoPtr<PatternFormatter> pPF(new PatternFormatter);
        pPF->setProperty("pattern", "%Y-%m-%d %H:%M:%S %s: %t");
        AutoPtr<FormattingChannel> pFC(new FormattingChannel(pPF, pAsync));

        Poco::Logger::root().setChannel(pFC);

        std::map<std::string, Poco::Message::Priority>::const_iterator it;

        it =
            stringToEnum.find(
                Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->getLogLevel());
        if (it != stringToEnum.end())
        {
            Poco::Logger::root().setLevel(it->second);
            Logger::get("TargetAgent").warning(Poco::format("Set logging level %s",
                                               Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->getLogLevel()));
        }
        else
        {
            Logger::get("TargetAgent").error(Poco::format("Invalid Log level: %s",
                                             Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->getLogLevel()));
            Poco::Logger::root().setLevel(Poco::Message::PRIO_ERROR);
        }

    }
    else
    {
        /*flush all messages*/
        AutoPtr<ConsoleChannel> pChannel(new ConsoleChannel);
        Logger::root().setChannel(pChannel);
        Logger::get("Logger").warning(
            "You have chosen not to have any logging for target agent.");

        AutoPtr<NullChannel> pNull(new NullChannel);
        Poco::Logger::root().setChannel(pNull);

    }

    taLogger = &Logger::get("TargetAgent"); // inherits root channel
    commLogger = &Logger::get("TargetAgent.CommunicationHandler"); // inherits root channel
    configLogger = &Logger::get("TargetAgent.ConfigProvider"); // inherits root channel
    recLogger = &Logger::get("TargetAgent.MessageRecorder"); // inherits root channel
    pluginLogger = &Logger::get("TargetAgent.PluginProvider"); // inherits root channel
    runtimeLogger = &(Logger::get("TargetAgent.RuntimeComponent")); // inherits root channel
}

void CLogger::checkReaderAvailability(void){
    const std::list<Configuration::PluginConfigItem>& pluginConfig =
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->Plugins();

    std::size_t found;
    std::list<Configuration::PluginConfigItem>::const_iterator pluginConfigIterator =
        pluginConfig.begin();


    for (; pluginConfigIterator != pluginConfig.end(); pluginConfigIterator++)
    {
        found=pluginConfigIterator->Name().find("socket-reader-plugin");
        if (found!=std::string::npos)
        {
            Logger::get("Logger").warning(
                "No Socket Reader Plugin Configured although logging on socket is active");

        }

    }
}

bool CLogger::isChannelActive(const std::string& channelName){
    bool isActive = false;
    std::list<Configuration::ConfigChannel>::const_iterator search =

        std::find(
            Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->getLogChannels().begin(),
            Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->getLogChannels().end(),
            channelName);

    if (search
        != Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->getLogChannels().end()
        && search->getIsChannelActive())
        isActive = true;
    return isActive;
}

CLogger::~CLogger() {
    /*will be handled by Poco*/
}

}
;
}
;
