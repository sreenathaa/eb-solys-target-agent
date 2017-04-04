/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef PLUGIN_LOADER_H_
#define PLUGIN_LOADER_H_

#include <list>
#include <stdlib.h>
#include <map>
#include <memory>

#include "TargetAgentDispatcherInterface.h"
#include "TargetAgentPluginInterface.h"
#include "plugin_notifier.h"
#include "CComController.h"
#include "config_provider.h"

#include "Poco/SharedLibrary.h"
#include "Poco/Mutex.h"



namespace TargetAgent
{
namespace PluginManagement
{

using namespace Protocol::CommonDefinitions;

class PluginProxy : public PluginInterface::CMessageDispatcher,
            public PluginManagement::ConnectionStatusNotifier
{
public:

    PluginProxy(void);
    virtual ~PluginProxy(void);

    void loadPlugins(void);
    void initializePluginProvider(void);
    void deinitializePluginProvider(void);
    void onHostConnectionEstablished(void);
    void onHostConnectionLost(void);
    void setPluginsConfig(void);
    void sendMessage(MessageType type,
                     int payloadLength,
                     unsigned char* payloadBuffer,
                     unsigned long long timestamp = 0,
                     const std::map<std::string, std::string>* msgContextInfo = 0) const;

    void onMessageReceived( const PluginInterface::ProtocolMessage* msg);

private:
    bool mConnectionEstablished;
    Communicator::ComController* mSender;
    std::map<PluginInterface::TargetAgentPluginInterface*,const Configuration::PluginConfigItem *> mLoadedPlugins;
    bool mIsPluginStarted;
    mutable Poco::Mutex mMsgMutex;

private:
    PluginInterface::TargetAgentPluginInterface* createPluginInstance(Poco::SharedLibrary& pluginLibrary);
    void storePluginInList( PluginInterface::TargetAgentPluginInterface* pluginInstance,const Configuration::PluginConfigItem*);
    PluginProxy (const PluginProxy& rhs);
    const PluginProxy& operator= (const PluginProxy& rhs);
    void startPlugins(void);
    void stopPlugins(void);

};

};
};

#endif /* PLUGIN_LOADER_H_ */
