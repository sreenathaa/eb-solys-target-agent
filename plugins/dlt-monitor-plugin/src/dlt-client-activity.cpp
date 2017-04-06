/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>

#include "dlt-client-activity.h"
#include "dlt-client-message-sender.h"
#include "dlt-monitor-plugin.h"

#include "target_agent_prot_dlt_monitor_plugin.pb.h"

#include "Poco/NumberParser.h"

using namespace std;
using namespace TargetAgent;

extern "C" {

    int dlt_receive_message_callback(DltMessage *message, void *data) {

        DltMonitorPlugin::DltMessageHandler *contextInfo;

        contextInfo = (DltMonitorPlugin::DltMessageHandler*) data;

        if (contextInfo == 0)
        {
            return -1;
        }

        if ((message == 0) || (data == 0))
        {
            contextInfo->logger->debug(
                "dlt_receive_message_callback message or data are pointing to NULL");
            return -1;
        }

        if (dlt_message_filter_check(message, &(contextInfo->filter),
                                     DLT_NOT_VERBOSE) == 1)
        {

            if (DLT_IS_HTYP_WEID(message->standardheader->htyp))
            {
                dlt_set_storageheader(message->storageheader,
                                      message->headerextra.ecu);
            }
            else
            {
                dlt_set_storageheader(message->storageheader, DLT_RECEIVE_ECU_ID);
            }

            if (DLT_IS_HTYP_WTMS(message->standardheader->htyp)
                && DLT_IS_HTYP_UEH(message->standardheader->htyp)
                && !contextInfo->parseLine(message))
            {
                contextInfo->logger->debug(
                    "dlt_receive_message_callback line parsing failed");
            }
        }
        return 0;
    }

    static void *dlt_consumer(void *context) {

        DltMonitorPlugin::DltMessageHandler *ctx;

        ctx = (DltMonitorPlugin::DltMessageHandler*) context;

#ifdef _POSIX_THREADS

        if (0 != pthread_setname_np(pthread_self(), "DltPluginMain"))
        {
            ctx->logger->error("Failed to set thread name");
        }
#endif
        const char* fileName = "filter.txt";

        dlt_client_init(&(ctx->dltclient), DLT_NOT_VERBOSE);
        dlt_file_init(&(ctx->file), DLT_NOT_VERBOSE);
        dlt_filter_init(&(ctx->filter), DLT_NOT_VERBOSE);

        if (dlt_filter_load(&(ctx->filter), fileName, DLT_NOT_VERBOSE) < 0)
        {
            dlt_file_free(&(ctx->file), DLT_NOT_VERBOSE);
            ctx->logger->error("Load filter failed");
            return 0;
        }

        dlt_file_set_filter(&(ctx->file), &(ctx->filter), DLT_NOT_VERBOSE);
        dlt_client_register_message_callback(&dlt_receive_message_callback);
        (ctx->dltclient.servIP) = "localhost";

        if (dlt_client_connect(&(ctx->dltclient), DLT_NOT_VERBOSE) != -1)
        {
            dlt_client_main_loop(&(ctx->dltclient), ctx, DLT_NOT_VERBOSE);

            dlt_client_cleanup(&(ctx->dltclient), DLT_NOT_VERBOSE);
        }

        dlt_file_free(&(ctx->file), DLT_NOT_VERBOSE);

        dlt_filter_free(&(ctx->filter), DLT_NOT_VERBOSE);

        return NULL;

    }

    static bool isMostMessage(DltMessage* message) {
        return (0 == strncmp(message->extendedheader->apid, "MOST", 4))
               && ((0 == strncmp(message->extendedheader->ctid, "IN", 2))
                   || (0 == strncmp(message->extendedheader->ctid, "OUT", 3)));
    }

}

namespace DltMonitorPlugin {

DltMessageHandler::DltMessageHandler(DltClientMessageSender& dispatcher) :
mDispatcher(dispatcher), dltclient(), t(), logger(0), dltMessage(0) {
    logger = &(Poco::Logger::get("TargetAgent.DltMonitorPlugin")); // inherits configuration from Target Agent

    dlWrappertmessage.set_id(
        Protocol::DLTLogInspector::LOG_ANALYZER_MESSAGE_TRACE);
    dltMessage = dlWrappertmessage.mutable_trace_message();
}

bool DltMessageHandler::start() {
    bool retVal = true;

    pthread_create(&t, NULL, &dlt_consumer, this);

    return retVal;
}

bool DltMessageHandler::stop() {
    bool retVal = true;

    if (pthread_cancel(t))
    {

        logger->error("Error joining thread");
        retVal = false;
    }
    else
        retVal = true;
    return retVal;
}
bool DltMessageHandler::isMostMessageValid(const std::string& message) {
    return ((MOST_HEADER_SIZE < message.size()) && isdigit(message[0]));
}

bool DltMessageHandler::parseMostMessage(DltMessage *message, char* payload) {
    bool retVal = true;
    static std::string payloadAsString;
    payloadAsString.assign(payload, strlen(payload));

    if (isMostMessageValid(payloadAsString))
    {
        payloadAsString.erase(
            std::remove(payloadAsString.begin(), payloadAsString.end(),
                        ' '), payloadAsString.end());

        string fktblockid;
        string instid;
        string optype;
        string fktid;
        string remainingPayload;

        fktblockid = payloadAsString.substr(FBLOCK_ID_BEGIN, CHARS_PRO_BYTE);
        instid = payloadAsString.substr(INST_ID_BEGIN, CHARS_PRO_BYTE);
        optype = payloadAsString.substr(OPTYPE_BEGIN, CHARS_PRO_BYTE);
        fktid = payloadAsString.substr((FKT_ID_BEGIN + CHARS_PRO_BYTE),
                                       CHARS_PRO_BYTE);
        fktid += payloadAsString.substr(FKT_ID_BEGIN, CHARS_PRO_BYTE);
        remainingPayload = payloadAsString.substr(MOST_HEADER_SIZE,
                           payloadAsString.length() - MOST_HEADER_SIZE);

        Protocol::MostSpy::MostSpyApplicationMessage mostWrapper;
        mostWrapper.set_messageid(Protocol::MostSpy::MOST_SPY_OP_MOST_MESSAGE);
        mostWrapper.set_operationtype(
            Protocol::MostSpy::MOST_SPY_OPTYPE_NOTIFY);
        mostWrapper.set_errorcodes(Protocol::MostSpy::MOST_SPY_RESULT_INVALID);

        TargetAgent::Protocol::MostSpy::MostSpyMostMessage* mostMsg =
            mostWrapper.mutable_mostmessage();
        mostMsg->set_fblockid(Poco::NumberParser::parseHex(fktblockid));
        mostMsg->set_instid(Poco::NumberParser::parseHex(instid));
        mostMsg->set_optype(Poco::NumberParser::parseHex(optype));
        mostMsg->set_fktid(Poco::NumberParser::parseHex(fktid));
        stringstream timeStampAsString;
        timeStampAsString << (message->headerextra.tmsp);
        mostMsg->set_timestamp(timeStampAsString.str());
        mostMsg->set_payload(remainingPayload);
        mDispatcher.sendMOSTMessage(&mostWrapper);

    }
    else
    {
        logger->warning(
            Poco::format("unable to parse most message %s",
                         payloadAsString));
        retVal = false;
    }

    return retVal;

}

void DltMessageHandler::storeMessageInfo(DltMessage *msg) {

    unsigned int messageType = DLT_GET_MSIN_MSTP(msg->extendedheader->msin);
    unsigned int messageSubType = DLT_GET_MSIN_MTIN(msg->extendedheader->msin);

    if (Protocol::DLTLogInspector::DLTMessageType_IsValid(messageType))
    {

        dltMessage->set_messagetype(
            static_cast<Protocol::DLTLogInspector::DLTMessageType>(messageType));

        switch (messageType)
        {
        case DLT_TYPE_LOG:
            if (Protocol::DLTLogInspector::MsgLogInfo_IsValid(messageSubType))
            {
                dltMessage->set_loginfo(
                    static_cast<Protocol::DLTLogInspector::MsgLogInfo>(messageSubType));
            }
            break;
        case DLT_TYPE_APP_TRACE:
            if (Protocol::DLTLogInspector::MsgTraceInfo_IsValid(
                    messageSubType))
            {
                dltMessage->set_traceinfo(
                    static_cast<Protocol::DLTLogInspector::MsgTraceInfo>(messageSubType));
            }

            break;
        case DLT_TYPE_NW_TRACE:
            if (Protocol::DLTLogInspector::MsgBusInfo_IsValid(messageSubType))
            {
                dltMessage->set_businfo(
                    static_cast<Protocol::DLTLogInspector::MsgBusInfo>(messageSubType));
            }

            break;
        case DLT_TYPE_CONTROL:
            if (Protocol::DLTLogInspector::MsgControlInfo_IsValid(
                    messageSubType))
            {
                dltMessage->set_controlinfo(
                    static_cast<Protocol::DLTLogInspector::MsgControlInfo>(messageSubType));
            }
            break;

        default:
            {

            }

        }
    }
}

void DltMessageHandler::parseDltMessage(DltMessage *message, char* payload) {
    dltMessage->clear_data();
    dltMessage->clear_loginfo();
    dltMessage->clear_traceinfo();
    dltMessage->clear_businfo();
    dltMessage->clear_controlinfo();

    int len = (
                  (strlen(message->extendedheader->apid) <= DLT_ID_SIZE) ?
                  strlen(message->extendedheader->apid) : DLT_ID_SIZE);
    dltMessage->set_channel(message->extendedheader->apid, len);
    stringstream timeStampAsString;
    timeStampAsString << (message->headerextra.tmsp);
    dltMessage->set_timestamp(timeStampAsString.str());
    len = ((strlen(message->extendedheader->ctid) <= DLT_ID_SIZE) ?
           strlen(message->extendedheader->ctid) : DLT_ID_SIZE);
    dltMessage->set_context(message->extendedheader->ctid, len);
    dltMessage->set_data(payload, strlen(payload));

    storeMessageInfo(message);

    mDispatcher.sendDLTMessage(&dlWrappertmessage);

}

bool DltMessageHandler::parseLine(DltMessage *message) {

    bool retVal = true;
    static char payload[DLT_RECEIVE_TEXTBUFSIZE];

    dlt_message_payload(message, payload, DLT_RECEIVE_TEXTBUFSIZE,
                        DLT_OUTPUT_ASCII, DLT_NOT_VERBOSE);

    try
    {

        if (isMostMessage(message))
        {
            parseMostMessage(message, payload);
        }
        else
        {
            parseDltMessage(message, payload);
        }

    }
    catch (Poco::Exception& exc)
    {
        logger->log(exc);
        retVal = false;
    }

    return retVal;
}

}

