/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/


#ifndef JSONAPI_PLUGIN_H_
#define JSONAPI_PLUGIN_H_

#include <iostream>
#include <fstream>

#include "Poco/Activity.h"
#include <Poco/Thread.h>
#include "Poco/Logger.h"


#include "TargetAgentPluginInterface.h"

#include "target_agent_prot_jsonapi_plugin.pb.h"

using namespace std;

using namespace TargetAgent;
using namespace PluginInterface;
using namespace Poco;

extern "C" void* createPluginInstance(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider);


namespace TargetAgentjsonapiPlugin
{

/**
 * Reads Json API data from a linux fifo. 
 */
class CjsonapiPlugin: public PluginInterface::TargetAgentPluginInterface {
public:

	CjsonapiPlugin(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider);

    /**
	 * Trigger Plugin startup. It is mandatory to send here whatever is considered metadata in the context of the plugin.
	 * @param none
	 * @see -
	 * @return true:	 startup phase of the plugin was successfully trigger
	 * 		  false: error occurred during the startup phase
	 */
	virtual bool startPlugin(void);

	/**
	 * Trigger Plugin shutdown.
	 * @param none
	 * @see -
	 * @return true:	 shutdown phase of the plugin was successfully trigger
	 * 		  false: error occurred during the shutdown phase
	 */
	virtual bool stopPlugin(void);

	/**
	 * Set plugin configuration: as described in the plugin section of the configuration xml
	 * @param map containing string,string pairs
	 * @see -
	 * @return true:	 configuration was successfully applied
	 * 		  false: error while setting the configuration
	 */
	virtual bool setConfig(
			const std::map<std::string, std::string>& pluginConfiguration);

	/**
	 * Triggered when a client sends a meesage to that specific plugin instance
	 * @param payloadLength length of the message in bytes
	 * @param pointer to the message, valid until the function exists
	 * @see -
	 * @return true:	 configuration was successfully applied
	 * 		  false: error while setting the configuration
	 */
	virtual void onMessageReceived(int payloadLength,
			const unsigned char* payloadBuffer /*do we need any timestamp here?*/);

	/**
	 * Function fired when a client is connected
	 * @param none
	 * @see -
	 * @return none
	 */
	virtual void onConnectionEstablished(void);

	/**
	 * Function fired when a client disconnects. It is mandatory to send here whatever is considered metadata in the context of the plugin
	 * @param none
	 * @see -
	 * @return none
	 */
	virtual void onConnectionLost(void);

	/**
	 * Retrieve the message Type which the plugin is able to send
	 * @param none
	 * @see -
	 * @return Protocol::CommonDefinitions::MessageType
	 * 		  On error MSG_TYPE_INVALID shall be returned
	 */
	virtual Protocol::CommonDefinitions::MessageType MessageType(void);

	virtual ~CjsonapiPlugin();
protected:
	void run();

private:
	virtual void dispatch(JsonAPIMsg msg);

	const CMessageDispatcher * const mMsgSenderHDL;
	const CTimestampProvider * const  mTimestampProvider;
	Protocol::CommonDefinitions::MessageType messageTypeSocketReader;
	Poco::Logger* logger;
	Activity<CjsonapiPlugin> activity;
	int fd;
};

}

#endif /* JSONAPI_PLUGIN_H_ */
