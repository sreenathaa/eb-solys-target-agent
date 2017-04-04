/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "Poco/SharedLibrary.h"
#include "Poco/Exception.h"


#include "plugin_provider.h"
#include "protocol_message.h"
#include "CMediator.h"
#include "config_provider.h"
#include "CLogger.hpp"


using namespace TargetAgent::PluginInterface;

namespace TargetAgent {
namespace PluginManagement {

typedef void* (*PluginFactoryMethod)(
    const CMessageDispatcher* const senderHandle,
    const CTimestampProvider* tsProvider);

PluginProxy::PluginProxy(void) :
        mConnectionEstablished(false), mSender(NULL), mLoadedPlugins(), mIsPluginStarted(
    false) {
}

PluginProxy::~PluginProxy(void) {
    mLoadedPlugins.clear();
}

void PluginProxy::initializePluginProvider(void) {

    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getPluginProviderLogger().trace(
        "Plugin Provider initialize begin");

    this->mSender =
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getComController();

    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getPluginProviderLogger().information(
        "Load plugins");
    loadPlugins();

    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getPluginProviderLogger().information(
        "Set static plugins configuration");
    setPluginsConfig();

    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getPluginProviderLogger().information(
        "Start plugins");

    startPlugins();

    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getPluginProviderLogger().trace(
        "Plugin Provider initialize end");
}

void PluginProxy::deinitializePluginProvider(void) {
    stopPlugins();
}

void PluginProxy::onHostConnectionEstablished() {

    mMsgMutex.lock();
    mConnectionEstablished = true;
    if (Mediator::CTargetAgentRuntime::getUniqueInstance()->getMessageRecorder()
        != NULL)
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getMessageRecorder()->sendBufferedData();
    }
    std::map<PluginInterface::TargetAgentPluginInterface*,
    const Configuration::PluginConfigItem *>::iterator it;
    for (it = mLoadedPlugins.begin(); it != mLoadedPlugins.end(); it++)
    {
        it->first->onConnectionEstablished();
    }
    mMsgMutex.unlock();
}

void PluginProxy::onHostConnectionLost(void) {

    mMsgMutex.lock();
    if (Mediator::CTargetAgentRuntime::getUniqueInstance()->getMessageRecorder()
        != NULL)
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getMessageRecorder()->resumeBuffering();
    }
    std::map<PluginInterface::TargetAgentPluginInterface*,
    const Configuration::PluginConfigItem *>::iterator it;
    for (it = mLoadedPlugins.begin(); it != mLoadedPlugins.end(); it++)
    {
        it->first->onConnectionLost();
    }
    mConnectionEstablished = false;

    mMsgMutex.unlock();
}


void PluginProxy::sendMessage(MessageType type,
                              int payloadLength,
                              unsigned char* payloadBuffer,
                              unsigned long long timestamp,
                              const std::map<std::string, std::string>* msgContextInfo) const {
    bool memoryInUse = false;
    /*convert to internal message*/
    mMsgMutex.lock();

    const PluginInterface::ProtocolMessage* msg =
        new PluginInterface::ProtocolMessage(type, payloadLength,
                                             payloadBuffer,false, timestamp, msgContextInfo);

    bool dispatchToRecorder =
        (!mConnectionEstablished
         || (mConnectionEstablished
             && Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->isRecorderOn())) ?
        true : false;

    if (dispatchToRecorder)
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getMessageRecorder()->recordMessage(
            msg, false);
    }

    bool dispatchToClient =
        (mConnectionEstablished && mSender != NULL) ? true : false;

    if (dispatchToClient)
    {
        mSender->sendMessage(msg);
        memoryInUse = true;
    }

    if (!memoryInUse)
    {
        delete msg;
        msg = NULL;
    }

    mMsgMutex.unlock();
}


void PluginProxy::loadPlugins(void) {
    const std::list<Configuration::PluginConfigItem>& pluginConfig =
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->Plugins();

    std::list<Configuration::PluginConfigItem>::const_iterator pluginConfigIterator =
        pluginConfig.begin();

    for (; pluginConfigIterator != pluginConfig.end(); pluginConfigIterator++)
    {
        Poco::SharedLibrary pluginLibrary;

        try
        {
            pluginLibrary.load(pluginConfigIterator->Path());

            Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getPluginProviderLogger().warning(
                Poco::format(
                    "Loaded plugin %s with the following library name %s",
                    pluginConfigIterator->Name(),
                    pluginConfigIterator->Path()));
        }
        catch (Poco::LibraryLoadException& e)
        {
            Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getPluginProviderLogger().log(
                e);
            continue;
        }

        if (pluginLibrary.isLoaded())
        {
            PluginInterface::TargetAgentPluginInterface* pluginInstance =
                createPluginInstance(pluginLibrary);

            if (pluginInstance != NULL)
            {
                storePluginInList(pluginInstance, &(*pluginConfigIterator));
            }
            else
            {
                pluginLibrary.unload();

            }
        }
    }
}

PluginInterface::TargetAgentPluginInterface* PluginProxy::createPluginInstance(
    Poco::SharedLibrary& pluginLibrary) {
    if (pluginLibrary.hasSymbol("createPluginInstance"))
    {
        PluginFactoryMethod createPluginInstance =
            (PluginFactoryMethod) pluginLibrary.getSymbol(
                "createPluginInstance");

        if (createPluginInstance != NULL)
        {
            void* rawPointerToPluginInstance =
                createPluginInstance(
                    Mediator::CTargetAgentRuntime::getUniqueInstance()->getPluginManager(),
                    (new PluginManagement::TimestampProviderFactory())->createTimestamp(
                        (Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->getTimeReference())));

            if (rawPointerToPluginInstance != NULL)
            {
                PluginInterface::TargetAgentPluginInterface* pluginInstance =
                    static_cast<PluginInterface::TargetAgentPluginInterface*>(rawPointerToPluginInstance);
                if (pluginInstance != NULL)
                {
                    return pluginInstance;
                }
                else
                {
                    free(rawPointerToPluginInstance);
                    return NULL;
                }
            }
        }
    }
    return NULL;
}

void PluginProxy::storePluginInList(
    PluginInterface::TargetAgentPluginInterface* pluginInstance,
    const Configuration::PluginConfigItem* config) {
    mLoadedPlugins.insert(
        std::pair<PluginInterface::TargetAgentPluginInterface*,
        const Configuration::PluginConfigItem*>(pluginInstance,
                                                config));
}

void PluginProxy::setPluginsConfig(void) {
    std::map<PluginInterface::TargetAgentPluginInterface*,
    const Configuration::PluginConfigItem *>::iterator it;
    for (it = mLoadedPlugins.begin(); it != mLoadedPlugins.end(); it++)
    {
        it->first->setConfig(it->second->getPluginProperties());
    }
}

void PluginProxy::startPlugins(void) {
    if (!mIsPluginStarted)
    {
        std::map<PluginInterface::TargetAgentPluginInterface*,
        const Configuration::PluginConfigItem *>::iterator it =
            mLoadedPlugins.begin();
        for (it = mLoadedPlugins.begin(); it != mLoadedPlugins.end(); it++)
        {
            it->first->startPlugin();
        }
        mIsPluginStarted = true;
    }
    else
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getPluginProviderLogger().error(
            "Plugin already started");
    }
}

void PluginProxy::stopPlugins(void) {
    if (mIsPluginStarted)
    {
        std::map<PluginInterface::TargetAgentPluginInterface*,
        const Configuration::PluginConfigItem *>::iterator it;
        for (it = mLoadedPlugins.begin(); it != mLoadedPlugins.end(); it++)
        {
            it->first->stopPlugin();
        }
        mIsPluginStarted = false;
    }
    else
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getPluginProviderLogger().error(
            "Try to stop plugin which is not running");
    }
}

void PluginProxy::onMessageReceived(
    const PluginInterface::ProtocolMessage* msg) {

    Protocol::CommonDefinitions::MessageType item = msg->getMessageType();
    typedef std::map<PluginInterface::TargetAgentPluginInterface*,
    const Configuration::PluginConfigItem *>::iterator it_type;
    for (it_type iterator = mLoadedPlugins.begin();
         iterator != mLoadedPlugins.end(); iterator++)
    {
        if (iterator->first->MessageType() == item)
        {
            iterator->first->onMessageReceived(msg->getMessageLength(),
                                               msg->getPayloadData());
            break;
        }
    }
}

}
;
}
;
