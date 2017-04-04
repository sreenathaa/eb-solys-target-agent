/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#ifndef DLT_MONITOR_ACTIVITY
#define DLT_MONITOR_ACTIVITY

#include <iostream>
#include <string>

#include "Poco/Activity.h"
#include "Poco/Logger.h"

#include "dlt-client-message-sender.h"
#include "dlt_client.h"

#define DLT_RECEIVE_TEXTBUFSIZE 10024
#define DLT_RECEIVE_ECU_ID "RECV"
#define DLT_NOT_VERBOSE 0
#define DLT_NOT_SERIAL 0

namespace DltMonitorPlugin {

class DltMessageHandler {
public:

    DltMessageHandler(DltClientMessageSender& dispatcher);

    bool parseLine(DltMessage *message);
    bool start();
    bool stop();


private:
    bool isMostMessageValid(const std::string& message);
    void storeMessageInfo(DltMessage *message);
    bool parseMostMessage(DltMessage *message,char* payload);
    void parseDltMessage(DltMessage *message,char* payload);

    DltClientMessageSender& mDispatcher;
public:
    DltClient dltclient;
    DltFile file;
    DltFilter filter;
    pthread_t t;
    Poco::Logger* logger;
    static const unsigned int FBLOCK_ID_BEGIN=10;
    static const unsigned int INST_ID_BEGIN=12;
    static const unsigned int OPTYPE_BEGIN=14;
    static const unsigned int FKT_ID_BEGIN=16;
    static const unsigned int CHARS_PRO_BYTE=2;
    static const unsigned int MOST_HEADER_SIZE=20;
private:
    TargetAgent::Protocol::DLTLogInspector::DLTLogInspectorMessage       dlWrappertmessage;
    TargetAgent::Protocol::DLTLogInspector::DLTLogInspectorTraceMessage* dltMessage;
};

}
;

extern "C" {
    int dlt_receive_message_callback(DltMessage *message, void *data);
}

#endif
