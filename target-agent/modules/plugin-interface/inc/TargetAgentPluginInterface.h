/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef ABSTRACT_PLUGIN_H_
#define ABSTRACT_PLUGIN_H_

#if defined(_WIN32)
#define LIBRARY_API __declspec(dllexport)
#else
#define LIBRARY_API
#endif

#include "TargetAgentDispatcherInterface.h"
#include "TargetAgentTsProviderInterface.h"

#include <map>
#include <memory>
#include <iostream>

namespace TargetAgent {
namespace PluginInterface {

/**
 * Target Agent Plugin Interface.
 */

class LIBRARY_API TargetAgentPluginInterface {
public:
    /**
     * Trigger Plugin startup. It is mandatory to send here whatever is considered metadata in the context of the plugin.
     * @param none
     * @see -
     * @return true:  startup phase of the plugin was successfully trigger
     *     false: error occurred during the startup phase
     */
    virtual bool startPlugin(void) = 0;

    /**
     * Trigger Plugin shutdown.
     * @param none
     * @see -
     * @return true:  shutdown phase of the plugin was successfully trigger
     *     false: error occurred during the shutdown phase
     */
    virtual bool stopPlugin(void) = 0;

    /**
     * Set plugin configuration: as described in the plugin section of the configuration xml
     * @param map containing string,string pairs
     * @see -
     * @return true:  configuration was successfully applied
     *     false: error while setting the configuration
     */
    virtual bool setConfig(
        const std::map<std::string, std::string>& pluginConfiguration)=0;

    /**
     * Triggered when a client sends a meesage to that specific plugin instance
     * @param payloadLength length of the message in bytes
     * @param pointer to the message, valid until the function exists
     * @see -
     * @return true:  configuration was successfully applied
     *     false: error while setting the configuration
     */
    virtual void onMessageReceived(int payloadLength,
                                   const unsigned char* payloadBuffer /*do we need any timestamp here?*/) = 0;

    /**
     * Function fired when a client is connected
     * @param none
     * @see -
     * @return none
     */
    virtual void onConnectionEstablished(void) = 0;

    /**
     * Function fired when a client disconnects. It is mandatory to send here whatever is considered metadata in the context of the plugin
     * @param none
     * @see -
     * @return none
     */
    virtual void onConnectionLost(void) = 0;

    /**
     * Retrieve the message Type which the plugin is able to send
     * @param none
     * @see -
     * @return Protocol::CommonDefinitions::MessageType
     *     On error MSG_TYPE_INVALID shall be returned
     */
    virtual Protocol::CommonDefinitions::MessageType MessageType(void) = 0;

    virtual ~TargetAgentPluginInterface(void) {
    }
};

}
;
}
;

#endif //ABSTRACT_PLUGIN_H_
