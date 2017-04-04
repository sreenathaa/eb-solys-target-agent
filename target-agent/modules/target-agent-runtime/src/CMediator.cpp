/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#include <CMediator.h>
#include <iostream>
#include <CLogger.hpp>
#include "OfflineRecorder.hpp"
#include "BufferingHandler.hpp"

namespace TargetAgent {
namespace Mediator {


CTargetAgentRuntime* CTargetAgentRuntime::getUniqueInstance()
{
    static Poco::SingletonHolder<CTargetAgentRuntime> sh;
    return sh.get();
}

void CTargetAgentRuntime::run() {

    mConfigProvider = new Configuration::ConfigProvider();

    mConfigProvider->initialize();

    tsProvider =
        (new PluginManagement::TimestampProviderFactory())->createTimestamp(
            mConfigProvider->getTimeReference());

    mLogger = new logger::CLogger();
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getGlobalLogger().information(
        "Race Target Agent Started");
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getConfigLogger().information(
        "===Network Configuration===");
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getConfigLogger().information(
        Poco::format("Port Number %hu",
                     Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->StreamingPort()));
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getConfigLogger().information(
        "===Network Configuration===");

    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getConfigLogger().information(
        "===Recorder Configuration===");
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getConfigLogger().information(
        Poco::format("doRecord %b",
                     Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->isRecorderOn()));
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getConfigLogger().information(
        Poco::format("RecorderFilePath %s",
                     Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->getRecorderFilePrefix()));
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getConfigLogger().information(
        "===Recorder Configuration===");

    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getConfigLogger().information(
        "===Logger Configuration===");
    std::list<Configuration::ConfigChannel> logChannels =
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->getLogChannels();

    std::list<Configuration::ConfigChannel>::const_iterator logChIt =
        logChannels.begin();

    for (; logChIt != logChannels.end(); logChIt++)
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getPluginProviderLogger().information(
            Poco::format("Log Channel Configuration Active %b Name %s",
                         logChIt->getIsChannelActive(), logChIt->getName()));

    }
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getConfigLogger().information(
        "===Logger Configuration===");

    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getConfigLogger().information(
        "===Per Plugin Configuration===");
    const std::list<Configuration::PluginConfigItem>& pluginConfig =
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->Plugins();

    std::list<Configuration::PluginConfigItem>::const_iterator pluginConfigIterator =
        pluginConfig.begin();

    for (; pluginConfigIterator != pluginConfig.end(); pluginConfigIterator++)
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getPluginProviderLogger().information(
            Poco::format(
                "Loaded plugin %s with the following library name %s",
                pluginConfigIterator->Name(),
                pluginConfigIterator->Path()));
        std::map<std::string, std::string>::const_iterator propsIt =
            pluginConfigIterator->getPluginProperties().begin();

        for (; propsIt != pluginConfigIterator->getPluginProperties().end();
             propsIt++)
        {
            Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getPluginProviderLogger().information(
                Poco::format("Property %s Value %s", propsIt->first,
                             propsIt->second));
        }

    }
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getConfigLogger().information(
        "===Per Plugin Configuration===");

    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().information(
        "Target Agent Main Loop");

    /*at this stage we can use the logging*/
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().information(
        "XML Configuration successfully loaded");

    mPluginManager = new PluginManagement::PluginProxy();

    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().information(
        "Plugin Proxy created");

    mComHandler = new Communicator::ComController();
    if (Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->isRecorderOn())
    {
        mMessageRecorder = new MessageRecorder::COfflineRecorder();
    }
    else
    {
        mMessageRecorder = new MessageRecorder::CBufferingHandler();
    }

    mPluginManager->initializePluginProvider();

    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().information(
        "Plugin Manager Initialized");

    mComHandler->start();

    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().information(
        "Communication Handler Started");


}

void CTargetAgentRuntime::shutdown() {

    mMessageRecorder->stopRecording();

    mPluginManager->deinitializePluginProvider();

    mComHandler->stop();

    delete mPluginManager;

    delete mLogger;
}

PluginManagement::PluginProxy* CTargetAgentRuntime::getPluginManager(
    void) const {
    return mPluginManager;
}

Configuration::ConfigProvider* CTargetAgentRuntime::getConfigProvider(void) {
    return mConfigProvider.get();
}

Communicator::ComController* CTargetAgentRuntime::getComController(void) {
    return mComHandler.get();
}

MessageRecorder::CMessageRecorder* CTargetAgentRuntime::getMessageRecorder(
    void) {
    return mMessageRecorder.get();
}

logger::CLogger* CTargetAgentRuntime::getLogger(void) const {
    return mLogger;
}

CTargetAgentRuntime::CTargetAgentRuntime() :
        mConfigProvider(), mPluginManager(), mComHandler(), mMessageRecorder(), mLogger(
    0) {
}

}
}
