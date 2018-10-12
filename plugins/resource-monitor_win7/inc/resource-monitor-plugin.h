/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/


#ifndef RESOURCE_MONITOR_PLUGIN_H_
#define RESOURCE_MONITOR_PLUGIN_H_


#include "Poco/NumberParser.h"

#include "TargetAgentTsProviderInterface.h"
#include "TargetAgentPluginInterface.h"
#include "target_agent_prot_resource_monitor.pb.h"



using namespace TargetAgent::PluginInterface;

extern "C" LIBRARY_API void* createPluginInstance(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider);

namespace ResourceMonitor {

using namespace TargetAgent;
class ResourceMonitorScheduler;


class ResourceMonitorPlugin: public PluginInterface::TargetAgentPluginInterface
{
public:
    ResourceMonitorPlugin(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider);
    virtual ~ResourceMonitorPlugin();

    /**
     * Initializes the messageSenderHDL and scheduler.
     * @param resourceInfoMessage The message that will be sent
     * @return -
     */
    void sendResourceMonitorMessage(const ResourceInfo* resourceInfoMessage );

    /**
     * Empty function.
     * @param payloadLength
     * @param payloadBuffer
     * @return -
     */
    void onMessageReceived(INT payloadLength, const UCHAR* payloadBuffer);

    /**
     * Empty function.
     * @param -
     * @return A constant that represents the type of message
     */
    Protocol::CommonDefinitions::MessageType MessageType();

    /**
     * Empty function.
     * @param -
     * @return -
     */
    void onConnectionEstablished();

    /**
     * Empty function.
     * @param -
     * @return -
     */
    void onConnectionLost();

    /**
     * Reads and parses the config xml file.
     * @param -
     * @return TRUE all the time.
     */
    bool startPlugin();

    /**
     * Stops the scheduler.
     * @param -
     * @return TRUE all the time.
     */
    bool stopPlugin();

    /**
     * Sets the state of the plugin with the specified configuration.
     * @param pluginConfiguration contains the sampling rate and processes that will be added to the ignore list.
     * @return TRUE all the time.
     */
    bool setConfig(const std::map<std::string, std::string>& pluginConfiguration);


private:
    const CMessageDispatcher* const mMsgSenderHDL;
    const CTimestampProvider* const  mTimestampProvider;
    ResourceMonitorScheduler* scheduler;
    Protocol::CommonDefinitions::MessageType messageType;
    UINT samplingRate;
    Poco::Logger* logger;
    static const unsigned int DEFAULT_RATE = 1000;
    static const unsigned int MIN_RATE = 50;
    static const unsigned int MAX_RATE = 5000;


};

};

#endif /* RESOURCE_MONITOR_PLUGIN_H_ */
