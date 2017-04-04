/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#ifndef SOCKET_READER_PLUGIN_H_
#define SOCKET_READER_PLUGIN_H_

#include <iostream>
#include <fstream>

#include "Poco/Logger.h"


#include "TargetAgentPluginInterface.h"
#include "target_agent_prot_socket_reader_plugin.pb.h"
#include "socket_reader_activity.h"

using namespace std;

using namespace TargetAgent;
using namespace PluginInterface;

extern "C" LIBRARY_API void* createPluginInstance(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider);

namespace TargetAgentLog4jPlugin
{

class CSocketReaderPlugin: public PluginInterface::TargetAgentPluginInterface {
public:

    CSocketReaderPlugin(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider);
    virtual ~CSocketReaderPlugin();


    void onMessageReceived(int payloadLength,
                           const unsigned char* payloadBuffer);

    Protocol::CommonDefinitions::MessageType MessageType();

    bool shutdownPlugin();
    void onConnectionEstablished();
    void onConnectionLost();
    bool startPlugin();
    bool stopPlugin();
    bool setConfig(const std::map<std::string, std::string>& pluginConfiguration);


private:
    const CMessageDispatcher * const mMsgSenderHDL;
    const CTimestampProvider * const  mTimestampProvider;
    Protocol::CommonDefinitions::MessageType messageTypeSocketReader;
    Poco::Logger* logger;
    std::list<ServerSocket*> svSockets;
    std::list<SocketReactor*> reactors;
    std::list<SocketAcceptor<CSocketReaderServer>*> acceptors;
    std::list<Thread*> threads;
    std::list<unsigned int> ports;

};

}

#endif /* SOCKET_READER_PLUGIN_H_ */
