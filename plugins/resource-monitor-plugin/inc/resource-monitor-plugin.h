/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
/*! \file resource-monitor-plugin.h
\brief * Resource Monitor Plugin Class: data collector for system resources information at different levels:
 * system wise
 * process level
 * thread level
 
known limitations:
   *-
*/

#ifndef RESOURCE_MONITOR_PLUGIN_H_
#define RESOURCE_MONITOR_PLUGIN_H_

#include "TargetAgentPluginInterface.h"

/**
 * function has to be exported by every single plugin
 */

using namespace TargetAgent::PluginInterface;

extern "C" void* createPluginInstance(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider);

namespace ResourceMonitor {
class ResourceMonitorScheduler;
}

namespace ResourceMonitor {

/*using namespace TargetAgent;*/

/**
 * Resource Monitor Plugin Class: data collector for system resources information at different levels:
 * system wise
 * process level
 * thread level
 */

class ResourceMonitorPlugin: public TargetAgent::PluginInterface::TargetAgentPluginInterface {
public:
    /**
     * ResourceMonitorPlugin constructor.
     * no params
     */
    ResourceMonitorPlugin(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider);

    /**
     * ResourceMonitorPlugin destructor.
     */
    virtual ~ResourceMonitorPlugin(void);

    /* *
      * Trigger Plugin shutdown.
      * @param none
      * @see -
      * @return true:  startup phase of the plugin was successfully trigger
      *     false: error occurred during the startup phase
     
     bool initialize(
       const TargetAgent::PluginInterface::CMessageDispatcher* senderHandle, const TargetAgent::PluginInterface::CTimestampProvider* tsProvider = 0);*/

    /**
     * Trigger Plugin startup. It is mandatory to send here whatever is considered metadata in the context of the plugin.
     * @param none
     * @see -
     * @return true:  startup phase of the plugin was successfully trigger
     *     false: error occurred during the startup phase
     */
    bool startPlugin(void);

    /**
     * Trigger Plugin shutdown.
     * @param none
     * @see -
     * @return true:  shutdown phase of the plugin was successfully trigger
     *     false: error occurred during the shutdown phase
     */
    bool stopPlugin(void);

    /**
     * Set plugin configuration: as described in the plugin section of the configuration xml
     * @param map containing string,string pairs
     * @see -
     * @return true:  configuration was successfully applied
     *     false: error while setting the configuration
     */
    bool setConfig(
        const std::map<std::string, std::string>& pluginConfiguration);

    /**
     * Triggered when a client sends a meesage to that specific plugin instance
     * @param payloadLength length of the message in bytes
     * @param pointer to the message, valid until the function exists
     * @see -
     * @return true:  configuration was successfully applied
     *     false: error while setting the configuration
     */
    void onMessageReceived(int payloadLength,
                           const unsigned char* payloadBuffer /*do we need any timestamp here?*/);

    /**
     * Function fired when a client is connected
     * @param none
     * @see -
     * @return none
     */
    void onConnectionEstablished(void);

    /**
     * Function fired when a client disconnects. It is mandatory to send here whatever is considered metadata in the context of the plugin
     * @param none
     * @see -
     * @return none
     */
    void onConnectionLost(void);

    /**
     * Retrieve the message Type which the plugin is able to send
     * @param none
     * @see -
     * @return Protocol::CommonDefinitions::MessageType
     *     On error MSG_TYPE_INVALID shall be returned
     */
    TargetAgent::Protocol::CommonDefinitions::MessageType MessageType(
        void) ;

private:
    const CMessageDispatcher * const mMsgSenderHDL;
    const CTimestampProvider * const  mTimestampProvider;
    TargetAgent::Protocol::CommonDefinitions::MessageType messageType;
    ResourceMonitor::ResourceMonitorScheduler* scheduler;
    Poco::Logger* logger;
    static const unsigned int MIN_SAMPLING_RATE;
    static const unsigned int MAX_SAMPLING_RATE;
    static const unsigned int DEFAULT_SAMPLING_RATE;
private:
    /**
     * helper functions
     */
    bool storeSamplingRate(const std::string& samplingAsText,unsigned int& samplingAsValue) const;
    bool storeProcById(const std::string& procID,std::vector<int>& observed) const;
    bool storeProcByName(const std::string& procName,std::vector<int>& observed) const;

};

}
;

#endif /* RESOURCE_MONITOR_PLUGIN_H_ */
