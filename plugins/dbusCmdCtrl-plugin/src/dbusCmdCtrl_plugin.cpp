/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/



#include <iostream>
#include <google/protobuf/text_format.h>
#include "dbusCmdCtrl_plugin.hpp"
#include <math.h>

extern "C" void* createPluginInstance(
        const CMessageDispatcher* const senderHandle,
        const CTimestampProvider* tsProvider)
{
    return new TargetAgentdbusCmdCtrlPlugin::CdbusCmdCtrlPlugin(senderHandle,
            tsProvider);
}

namespace TargetAgentdbusCmdCtrlPlugin
{

using namespace TargetAgent;

CdbusCmdCtrlPlugin::CdbusCmdCtrlPlugin(
    const CMessageDispatcher* const senderHandle,
    const CTimestampProvider* tsProvider) :
        mMsgSenderHDL(senderHandle), mTimestampProvider(tsProvider), messageTypeSocketReader(
            Protocol::CommonDefinitions::MSG_TYPE_DBUSCMDCTRL_PLUGIN), logger(
                0)
{
    logger = &(Poco::Logger::get("TargetAgent.CdbusCmdCtrlPlugin")); // inherits configuration from Target Agent
    // Dbus
    DBusError error;
    dbus_error_init(&error);
    dbus_threads_init_default();

    mCmndCtrlconnection = dbus_bus_get_private(DBUS_BUS_SESSION, &error);
    if ((mCmndCtrlconnection != NULL) && !dbus_error_is_set(&error))
    {

        dbus_bus_get_unique_name(mCmndCtrlconnection);
    }

}

CdbusCmdCtrlPlugin::~CdbusCmdCtrlPlugin()
{
}

bool CdbusCmdCtrlPlugin::startPlugin()
{

    return true;
}


bool CdbusCmdCtrlPlugin::stopPlugin()
{
    return true;
}

bool CdbusCmdCtrlPlugin::setConfig(
    const std::map<std::string, std::string>& pluginConfiguration)
{
    logger->information("setConfig");
    return true;
}

void CdbusCmdCtrlPlugin::onMessageReceived(int payloadLength,
        const unsigned char* payloadBuffer)
{
    bool retVal;
    logger->information("Message Received by Plugin");
    DBusApplicationMessage CmndCtrlDbusMsg;
    //TargetAgent::Protocol::LoggingTests::LogConfiguration CmndCtrlDbusMsg;
    retVal = google::protobuf::TextFormat::ParseFromString(
                 std::string((char*) payloadBuffer, payloadLength), &CmndCtrlDbusMsg);

    if (retVal)
    {
        logger->information(
            Poco::format("parsed message %s", CmndCtrlDbusMsg.Utf8DebugString()));
        sendDbusMessage(CmndCtrlDbusMsg);
    }
    else
    {
        logger->error("Error in Parsing Protobuf Message");
    }
}

Protocol::CommonDefinitions::MessageType CdbusCmdCtrlPlugin::MessageType()
{
    return messageTypeSocketReader;
}

bool CdbusCmdCtrlPlugin::shutdownPlugin()
{
    return true;
}


void CdbusCmdCtrlPlugin::onConnectionEstablished(void)
{
}

void CdbusCmdCtrlPlugin::onConnectionLost()
{
}

void CdbusCmdCtrlPlugin::sendDbusMessage(
    DBusApplicationMessage CmndCtrlDbusMsg)
{
    std::string destination =
        CmndCtrlDbusMsg.tracemessage().header().receiver();
    std::string objectPath = CmndCtrlDbusMsg.tracemessage().header().path();
    std::string interface = CmndCtrlDbusMsg.tracemessage().header().interface();
    std::string method = CmndCtrlDbusMsg.tracemessage().header().member();
    dbusmsg_signature = CmndCtrlDbusMsg.tracemessage().header().method_signature();

    DBusError error;
    dbus_error_init(&error);
    DBusMessage* msg = dbus_message_new_method_call(destination.c_str(),
                       objectPath.c_str(), interface.c_str(), method.c_str());

    if (dbus_error_is_set(&error))
    {
        logger->error(
            Poco::format("destination %s DBUS Error: %s",
                         std::string(error.name), std::string(error.message)));
    }

    if (NULL != msg)
    {
        DBusMessageIter iterator;
        logger->information("Preparing to send Dbus Message");
        dbus_message_iter_init_append(msg, &iterator);

        for (int j = 0;
             j < CmndCtrlDbusMsg.tracemessage().payload().param_size();
             j++)
        {
            DBusMessagePayloadItem entry =
                CmndCtrlDbusMsg.tracemessage().payload().param(j);
            prototoDbusTranslation(&entry, &iterator);
        }

        logger->information(
            "Iterator has been filled with data, Sending Message and waiting for reply");
        DBusMessage* reply = dbus_connection_send_with_reply_and_block(
                                 mCmndCtrlconnection, msg, DBUS_MSG_TIMEOUT, &error);

        if (dbus_error_is_set(&error))
        {
            logger->error(
                Poco::format("destination %s DBUS Error: %s",
                             std::string(error.name),
                             std::string(error.message)));
        }

        logger->information("Done wit DBus Send");
        if(reply)
            dbus_message_unref(reply);
        if(msg)
            dbus_message_unref(msg);
    }
}


void CdbusCmdCtrlPlugin::prototoDbusTranslation(DBusMessagePayloadItem* entry,
        DBusMessageIter* iterator)
{

    modifySignature();
    if (!appendPrimitiveDataTypetoDbus(entry,iterator))
    {
        appendComplexDataTypetoDbus(entry,iterator);
    }
}

bool CdbusCmdCtrlPlugin::appendPrimitiveDataTypetoDbus(DBusMessagePayloadItem* entry,
        DBusMessageIter* iterator)
{

    bool DataAddedtoDbus = true;

    switch (entry->type())
    {
    case DBUS_MSG_PARAM_TYPE_BYTE:
        {
            logger->information("Adding Byte to the Dbus Iterator");
            uint32_t var = entry->int_val();
            dbus_message_iter_append_basic(iterator, DBUS_TYPE_BYTE,
                                           &var);
            break;
        }
    case DBUS_MSG_PARAM_TYPE_BOOLEAN:
        {
            logger->information("Adding Boolean to the Dbus Iterator");
            int32_t var = entry->int_val();
            dbus_message_iter_append_basic(iterator, DBUS_TYPE_BOOLEAN,
                                           &var);
            break;
        }
    case DBUS_MSG_PARAM_TYPE_INT16:
        {
            logger->information("Adding INT16 to the Dbus Iterator");
            int16_t var = entry->int_val();
            dbus_message_iter_append_basic(iterator, DBUS_TYPE_INT16,
                                           &var);
            break;
        }
    case DBUS_MSG_PARAM_TYPE_INT64:
        {
            logger->information("Adding INT64 to the Dbus Iterator");
            int64_t var = entry->int_val();
            dbus_message_iter_append_basic(iterator, DBUS_TYPE_INT64,
                                           &var);
            break;
        }
    case DBUS_MSG_PARAM_TYPE_INT32:
        {
            logger->information("Adding INT32 to the Dbus Iterator");
            int32_t var = entry->int_val();
            dbus_message_iter_append_basic(iterator, DBUS_TYPE_INT32,
                                           &var);
            break;
        }

    case DBUS_MSG_PARAM_TYPE_UINT16:
        {
            logger->information("Adding UINT16 to the Dbus Iterator");
            uint16_t var = entry->uint_val();
            dbus_message_iter_append_basic(iterator, DBUS_TYPE_UINT16,
                                           &var);
            break;
        }
    case DBUS_MSG_PARAM_TYPE_UINT32:
        {
            logger->information("Adding UINT32 to the Dbus Iterator");
            uint32_t var = entry->uint_val();
            dbus_message_iter_append_basic(iterator, DBUS_TYPE_UINT32,
                                           &var);
            break;
        }
    case DBUS_MSG_PARAM_TYPE_UINT64:
        {
            logger->information("Adding UINT64 to the Dbus Iterator");
            uint64_t var = entry->uint_val();
            dbus_message_iter_append_basic(iterator, DBUS_TYPE_UINT64,
                                           &var);
            break;
        }

    case DBUS_MSG_PARAM_TYPE_DOUBLE:
        {
            logger->information("Adding DOUBLE to the Dbus Iterator");
            double var = entry->double_val();
            dbus_message_iter_append_basic(iterator, DBUS_TYPE_DOUBLE,
                                           &var);
            break;
        }

    case DBUS_MSG_PARAM_TYPE_STRING:
        {
            logger->information("Adding STRING to the Dbus Iterator");
            dbus_message_iter_append_basic(iterator, DBUS_TYPE_STRING,
                                           &entry->str_val());
            break;
        }
    case DBUS_MSG_PARAM_TYPE_SIGNATURE:
        {
            logger->information("Adding SIGNATURE to the Dbus Iterator");
            dbus_message_iter_append_basic(iterator, DBUS_TYPE_SIGNATURE,
                                           &entry->str_val());
            break;
        }
    case DBUS_MSG_PARAM_TYPE_OBJ_PATH:
        {
            logger->information("Adding OBJ_PATH to the Dbus Iterator");
            dbus_message_iter_append_basic(iterator, DBUS_TYPE_OBJECT_PATH,
                                           &entry->str_val());
            break;
        }
    default:
        DataAddedtoDbus= false;
        break;
    }

    return DataAddedtoDbus;
}

bool CdbusCmdCtrlPlugin::appendComplexDataTypetoDbus(DBusMessagePayloadItem* entry,
        DBusMessageIter* iterator)
{

    bool DataAddedtoDbus = true;

    switch (entry->type())
    {
    case DBUS_MSG_PARAM_TYPE_STRUCT:
        {
            appendStructtoDbusMsg(entry,iterator);
            break;
        }
    case DBUS_MSG_PARAM_TYPE_ARRAY:
        {
            appendArraytoDbusMsg(entry,iterator);
            break;
        }
    case DBUS_MSG_PARAM_TYPE_VARIANT:
        {
            appendVarianttoDbusMsg(entry,iterator);
            break;
        }
    case DBUS_MSG_PARAM_TYPE_DICT_ENTRY:
        {
            appendDicttoDbusMsg(entry,iterator);
            break;
        }

    default:
        DataAddedtoDbus=false;
        break;
    }

    return DataAddedtoDbus;

}

void CdbusCmdCtrlPlugin::appendDicttoDbusMsg(DBusMessagePayloadItem* entry,
        DBusMessageIter* iterator)
{
    logger->information("Creating iterator for dbus_dict");
    DBusMessageIter dbus_dict;
    dbus_message_iter_open_container(iterator, DBUS_TYPE_DICT_ENTRY,
                                     NULL, &dbus_dict);
    for (int j = 0; j < entry->composite_val().param_size(); j++)
    {
        DBusMessagePayloadItem SubEntry = entry->composite_val().param(j);
        prototoDbusTranslation(&SubEntry, &dbus_dict);
    }
    logger->information("Closing container");
    dbus_message_iter_close_container(iterator,&dbus_dict);
}
void CdbusCmdCtrlPlugin::appendVarianttoDbusMsg(DBusMessagePayloadItem* entry,
        DBusMessageIter* iterator)
{
    logger->information("Creating iterator for dbus_variant");
    DBusMessageIter dbus_variant;
    const char* contained_signature= getSignatureType(entry->composite_val().param(0)).c_str();
    dbus_message_iter_open_container(iterator, DBUS_TYPE_VARIANT, contained_signature, &dbus_variant);
    for (int j = 0; j < entry->composite_val().param_size(); j++)
    {
        DBusMessagePayloadItem SubEntry = entry->composite_val().param(j);
        prototoDbusTranslation(&SubEntry, &dbus_variant);
    }
    logger->information("Closing container");
    dbus_message_iter_close_container(iterator,&dbus_variant);

}
void CdbusCmdCtrlPlugin::appendArraytoDbusMsg(DBusMessagePayloadItem* entry,
        DBusMessageIter* iterator)
{

    DBusMessageIter dbus_array;
    logger->information("Creating iterator for dbus_array");

    dbus_message_iter_open_container(iterator, DBUS_TYPE_ARRAY,dbusmsg_signature.c_str(), &dbus_array);
    for (int j = 0; j < entry->composite_val().param_size(); j++)
    {
        DBusMessagePayloadItem SubEntry = entry->composite_val().param(j);
        logger->information("parsing parameters");
        prototoDbusTranslation(&SubEntry, &dbus_array);
    }
    logger->information("Closing container");
    dbus_message_iter_close_container(iterator,&dbus_array);

}
void CdbusCmdCtrlPlugin::appendStructtoDbusMsg(DBusMessagePayloadItem* entry,
        DBusMessageIter* iterator)
{
    DBusMessageIter dbus_struct;
    logger->information("Creating iterator for dbus_struct");
    dbus_message_iter_open_container(iterator, DBUS_TYPE_STRUCT,
                                     NULL, &dbus_struct);
    for (int j = 0; j < entry->composite_val().param_size(); j++)
    {
        DBusMessagePayloadItem SubEntry = entry->composite_val().param(j);
        prototoDbusTranslation(&SubEntry, &dbus_struct);
    }
    logger->information("Closing container");
    dbus_message_iter_close_container(iterator,&dbus_struct);
}

std::string CdbusCmdCtrlPlugin::getSignatureType(DBusMessagePayloadItem entry)
{
    std::string retVal;
    switch (entry.type())
    {
    case DBUS_MSG_PARAM_TYPE_BYTE:
        {
            retVal= "y";
            break;
        }
    case DBUS_MSG_PARAM_TYPE_BOOLEAN:
        {
            retVal= "b";
            break;
        }
    case DBUS_MSG_PARAM_TYPE_INT16:
        {
            retVal= "n";
            break;
        }
    case DBUS_MSG_PARAM_TYPE_INT64:
        {
            retVal= "x";
            break;
        }
    case DBUS_MSG_PARAM_TYPE_INT32:
        {
            retVal= "i";
            break;
        }

    case DBUS_MSG_PARAM_TYPE_UINT16:
        {
            retVal= "q";
            break;
        }
    case DBUS_MSG_PARAM_TYPE_UINT32:
        {
            retVal= "u";
            break;
        }
    case DBUS_MSG_PARAM_TYPE_UINT64:
        {
            retVal= "t";
            break;
        }

    case DBUS_MSG_PARAM_TYPE_DOUBLE:
        {
            retVal= "d";
            break;
        }

    case DBUS_MSG_PARAM_TYPE_STRING:
        {
            retVal= "s";
            break;
        }
    case DBUS_MSG_PARAM_TYPE_SIGNATURE:
        {
            retVal= "g";
            break;
        }
    case DBUS_MSG_PARAM_TYPE_OBJ_PATH:
        {
            retVal= "o";
            break;
        }
    default:
        retVal= "";
    }

    return retVal;
}


void CdbusCmdCtrlPlugin::modifySignature()
{
    if (dbusmsg_signature.find("a") != std::string::npos)
    {
        dbusmsg_signature.erase(0,1);
    }
}

}


;
