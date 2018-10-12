/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef dbusCmdCtrl_PLUGIN_H_
#define dbusCmdCtrl_PLUGIN_H_

#include <iostream>
#include <fstream>

#include "Poco/Logger.h"
#include "Poco/SharedPtr.h"

#include "TargetAgentPluginInterface.h"
#include "target_agent_prot_dbusCmdCtrl_plugin.pb.h"
#include <google/protobuf/text_format.h>

#include <dbus/dbus.h>

using namespace std;

using namespace TargetAgent;
using namespace PluginInterface;
const unsigned int DBUS_MSG_TIMEOUT = 50;

extern "C" void* createPluginInstance(
        const CMessageDispatcher* const senderHandle,
        const CTimestampProvider* tsProvider);

namespace TargetAgentdbusCmdCtrlPlugin
{

class CdbusCmdCtrlPlugin: public PluginInterface::TargetAgentPluginInterface
{
public:

    CdbusCmdCtrlPlugin(const CMessageDispatcher* const senderHandle,
                       const CTimestampProvider* tsProvider);
    virtual ~CdbusCmdCtrlPlugin();

    void onMessageReceived(int payloadLength,
                           const unsigned char* payloadBuffer);

    Protocol::CommonDefinitions::MessageType MessageType();

    bool shutdownPlugin();
    void onConnectionEstablished();
    void onConnectionLost();
    bool startPlugin();
    bool stopPlugin();
    bool setConfig(
        const std::map<std::string, std::string>& pluginConfiguration);

private:
    const CMessageDispatcher * const mMsgSenderHDL;
    const CTimestampProvider * const mTimestampProvider;
    Protocol::CommonDefinitions::MessageType messageTypeSocketReader;
    Poco::Logger* logger;
    std::string dbusmsg_signature;
    DBusConnection* mCmndCtrlconnection;

private:
    void sendDbusMessage(DBusApplicationMessage CmndCtrlDbusMsg);
    void prototoDbusTranslation(DBusMessagePayloadItem*, DBusMessageIter*);
    bool appendPrimitiveDataTypetoDbus(DBusMessagePayloadItem*, DBusMessageIter*);
    bool appendComplexDataTypetoDbus(DBusMessagePayloadItem*, DBusMessageIter*);
    void appendDicttoDbusMsg(DBusMessagePayloadItem*, DBusMessageIter*);
    void appendVarianttoDbusMsg(DBusMessagePayloadItem*, DBusMessageIter*);
    void appendArraytoDbusMsg(DBusMessagePayloadItem* ,DBusMessageIter*);
    void appendStructtoDbusMsg(DBusMessagePayloadItem*, DBusMessageIter*);
    std::string getSignatureType(DBusMessagePayloadItem);
    void modifySignature();
};

}

#endif /* dbusCmdCtrl_PLUGIN_H_ */
