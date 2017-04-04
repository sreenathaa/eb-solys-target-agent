/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#include <iostream>
#include <unistd.h>

#include <Poco/Timestamp.h>
#include "Poco/Thread.h"

#include "dbus_data_pool.h"
#include "dbus_consumer.h"

const unsigned int DBUS_MSG_TIMEOUT = 50;
const int INVALID_PROCESS_ID = -1;
const int INVALID_USER_ID = -1;

using namespace std;

namespace TargetAgentDbusMonitor {

void CDBusConsumer::dbus2proto(const DBusMessage *msg,
                               const dbus_uint32_t& serial, DBusEvtTraceMessage* tracedMsg) {
    createDbusMessageHeader(tracedMsg->mutable_header(),
                            const_cast<DBusMessage *>(msg), serial);

    createTraceMessagePayload(tracedMsg->mutable_payload(),
                              const_cast<DBusMessage *>(msg));
    tracedMsg->set_instance(mDataPool->getProtoDbusInstanceType());
}

void CDBusConsumer::dbusConsumerLoop(void) {
    try
    {
        dbus_connection_set_exit_on_disconnect(mSecondconnection, FALSE);
        mDataPool->getLogger()->information(
            "exit on disconnect disabled for seconds dbus connection");

#ifdef _POSIX_THREADS

        if (0 != pthread_setname_np(pthread_self(), "PDBusConsumTree"))
        {
            mDataPool->getLogger()->error("Failed to set thread name for PDBusConsumTree");
        }
#endif

        while (!mDbusConsumerTask.isStopped())
        {
            mDataPool->waitConnectionEstablished();

            if (NULL != mDataPool->getDbusConnection())
            {

                mDataPool->getLogger()->information("Populate data pool");
                populateDataPool();
                mDataPool->getLogger()->warning("After DataPool is populated");
                mDataPool->dpPopulated();

                mDataPool->getLogger()->information(
                    "DataPool Populated trigger producer");

                while (!mDbusConsumerTask.isStopped())
                {
                    DbusMessageEntry* pMessageObject =
                        mDataPool->waitForNextMessageToSend();

                    if (pMessageObject)
                    {

                        handleNameOwnerChangeEvent(
                            pMessageObject->getMessage());

                        DBusApplicationMessage* wrapper =
                            DBusApplicationMessage::default_instance().New();
                        dbus2proto(pMessageObject->getMessage(),
                                   pMessageObject->getSerial(),
                                   wrapper->mutable_tracemessage());
                        mDataPool->getMessageSender()->sendApplicationMessage(
                            wrapper);
                        if (0 != wrapper)
                            delete wrapper;
                        if (0 != pMessageObject)
                            delete pMessageObject;
                    }
                    else
                    {
                        mDataPool->getLogger()->information(
                            "Null Dbus Message");
                    }

                    mDataPool->updateMessageAvailableFlag();
                }
            }
            else
            {
                mDataPool->getLogger()->information("Null Dbus Connection");
                mDbusConsumerTask.stop();
            }
        }
    }
    catch (Poco::Exception& e)
    {
        mDataPool->getLogger()->log(e);
    }
}

bool CDBusConsumer::insertService(std::string& name) {
    bool retVal = false;
    std::string owner;

    if (dbusGetNameOwner(name, owner))
    {

        mDataPool->getNameToOwnerPool().insert(
            std::pair<std::string, std::string>(name, owner));

        mDataPool->getLogger()->information(
            Poco::format("name %s owner: %s", name, owner));

        std::map<std::string, DBusService>::iterator serviceIt =
            mDataPool->getServicePool().find(owner);

        if (mDataPool->getServicePool().end() != serviceIt)
        {
            serviceIt->second.aliaslist.push_back(name);

            for (std::list<string>::iterator it =
                     serviceIt->
                     second.aliaslist.begin();
                 it != serviceIt->second.aliaslist.end();
                 ++it)
                mDataPool->getLogger()->information(
                    Poco::format("alias %s ", name, owner));

        }
        else
        {
            DBusService service;
            service.pid = getServicePID(owner.c_str());
            service.connectionUnixUser = getServiceUID(owner.c_str());
            service.aliaslist.push_back(name);
            mDataPool->getServicePool().insert(
                std::pair<std::string, DBusService>(owner, service));
            mDataPool->getLogger()->information(
                Poco::format("Owner %s added", owner));
        }
        retVal = true;
    }
    else
        mDataPool->getLogger()->warning("failed to get name owner");

    return retVal;
}

bool CDBusConsumer::populateDataPool(void) {
    bool retVal = false;

    std::list<std::string> dbusNames;

    if (dbusListNames(dbusNames))
    {
        for (std::list<std::string>::iterator it = dbusNames.begin();
             it != dbusNames.end(); it++)
        {
            insertService(*it);
        }
    }
    else
    {
        retVal = false;
        mDataPool->getLogger()->error(
            "not able to retrieve the dbus names list");
    }
    return retVal;
}

bool CDBusConsumer::parseListNamesReply(DBusMessage* reply,
                                        std::list<std::string>& namesList) {
    DBusMessageIter iterator;
    dbus_message_iter_init(reply, &iterator);
    mDataPool->getLogger()->information("parseListNamesReply");
    int type = dbus_message_iter_get_arg_type(&iterator);

    switch (type)
    {
    case DBUS_TYPE_ARRAY:
        {
            int arg_type;
            DBusMessageIter subiterator;

            dbus_message_iter_recurse(&iterator, &subiterator);

            while ((arg_type = dbus_message_iter_get_arg_type(&subiterator))
                   != DBUS_TYPE_INVALID)
            {

                switch (arg_type)
                {
                    char *str;
                case DBUS_TYPE_STRING:
                    {
                        dbus_message_iter_get_basic(&subiterator, &str);
                        std::string servicename(str);
                        namesList.push_back(servicename);
                    }
                    break;
                }
                dbus_message_iter_next(&subiterator);
            }

        }
        break;
    }
    return true;
}

bool CDBusConsumer::dbusListNames(std::list<std::string>& names) {

    bool retVal = true;

    std::string destination = "org.freedesktop.DBus";
    const string objectPath = "/org/freedesktop/DBus";
    string interface("org.freedesktop.DBus");
    string method("ListNames");

    DBusError error;

    DBusMessage* msg = dbus_message_new_method_call(destination.c_str(),
                       objectPath.c_str(), interface.c_str(), method.c_str());

    if (NULL != msg)
    {
        dbus_error_init(&error);

        DBusMessage* reply = dbus_connection_send_with_reply_and_block(
                                 mSecondconnection, msg, DBUS_MSG_TIMEOUT, &error);
        if (dbus_error_is_set(&error))
        {
            mDataPool->getLogger()->information(
                Poco::format("destination %s DBUS Error: %s",
                             std::string(error.name),
                             std::string(error.message)));
        }

        if (reply)
            parseListNamesReply(reply, names);

        dbus_message_unref(reply);
        dbus_message_unref(msg);
    }
    else
    {
        retVal = false;
        mDataPool->getLogger()->error(
            "dbus_message_new_method_call returned NULL");
    }

    return retVal;
}

bool CDBusConsumer::dbusGetNameOwner(std::string name, std::string& owner) {

    owner = name;
    bool retVal = true;

    if (isNotUniqueConnID(owner))
    {
        std::string destination = "org.freedesktop.DBus";
        const string objectPath = "/org/freedesktop/DBus";
        string interface("org.freedesktop.DBus");
        string method("GetNameOwner");

        DBusError error;
        DBusMessageIter iterator;
        DBusMessage* msg = dbus_message_new_method_call(destination.c_str(),
                           objectPath.c_str(), interface.c_str(), method.c_str());

        if (msg != NULL)
        {
            dbus_error_init(&error);
            dbus_message_iter_init_append(msg, &iterator);
            dbus_message_iter_append_basic(&iterator, DBUS_TYPE_STRING, &owner);

            DBusMessage* reply = dbus_connection_send_with_reply_and_block(
                                     mSecondconnection, msg, DBUS_MSG_TIMEOUT, &error);
            if (dbus_error_is_set(&error))
            {

                mDataPool->getLogger()->information(
                    Poco::format("destination %s DBUS Error: %s",
                                 std::string(error.name),
                                 std::string(error.message)));
            }
            if (reply)
            {
                dbus_message_iter_init(reply, &iterator);
                int type = dbus_message_iter_get_arg_type(&iterator);
                char *str;
                switch (type)
                {
                case DBUS_TYPE_STRING:
                    {
                        dbus_message_iter_get_basic(&iterator, &str);
                        owner = str;
                    }

                }
                dbus_message_unref(reply);
            }
            dbus_message_unref(msg);
        }
        else
            retVal = false;
    }

    return retVal;
}

int CDBusConsumer::getServicePID(const char* servicename) {
    int pid = INVALID_PROCESS_ID;
    std::string destination = "org.freedesktop.DBus";
    const string objectPath = "/org/freedesktop/DBus";
    string interface("org.freedesktop.DBus");
    string member("GetConnectionUnixProcessID");

    DBusError error;
    DBusMessageIter iterator;

    DBusMessage* msg = dbus_message_new_method_call(destination.c_str(),
                       objectPath.c_str(), interface.c_str(), member.c_str());
    if (msg != NULL)
    {
        dbus_error_init(&error);
        dbus_message_iter_init_append(msg, &iterator);
        dbus_message_iter_append_basic(&iterator, DBUS_TYPE_STRING,
                                       &servicename);

        DBusMessage* reply = dbus_connection_send_with_reply_and_block(
                                 mSecondconnection, msg, DBUS_MSG_TIMEOUT, &error);
        if (dbus_error_is_set(&error))
        {
            mDataPool->getLogger()->information(
                Poco::format("destination %s DBUS Error: %s",
                             std::string(error.name),
                             std::string(error.message)));
        }

        if (reply)
        {
            dbus_message_iter_init(reply, &iterator);
            int type = dbus_message_iter_get_arg_type(&iterator);
            switch (type)
            {
            case DBUS_TYPE_UINT32:
                {
                    dbus_message_iter_get_basic(&iterator, &pid);
                    break;
                }
            }
            dbus_message_unref(reply);
        }
        dbus_message_unref(msg);

    }
    mDataPool->getLogger()->information(Poco::format("pid %d DBUS", pid));
    return pid;
}

int CDBusConsumer::getServiceUID(const char* servicename) {
    unsigned int uid = INVALID_USER_ID;
    std::string destination = "org.freedesktop.DBus";
    const string path = "/org/freedesktop/DBus";
    string introspectInterface("org.freedesktop.DBus");
    string introspectMember("GetConnectionUnixUser");
    DBusError error;
    DBusMessageIter iterator;
    DBusMessage* msg = dbus_message_new_method_call(destination.c_str(),
                       path.c_str(), introspectInterface.c_str(),
                       introspectMember.c_str());
    if (msg != NULL)
    {
        dbus_error_init(&error);
        dbus_message_iter_init_append(msg, &iterator);
        dbus_message_iter_append_basic(&iterator, DBUS_TYPE_STRING,
                                       &servicename);
        /*reply message */
        DBusMessage* reply = dbus_connection_send_with_reply_and_block(
                                 mSecondconnection, msg, DBUS_MSG_TIMEOUT, &error);
        if (dbus_error_is_set(&error))
        {
            mDataPool->getLogger()->information(
                Poco::format("destination %s DBUS Error: %s",
                             std::string(error.name),
                             std::string(error.message)));
        }

        if (reply)
        {
            dbus_message_iter_init(reply, &iterator);
            int type = dbus_message_iter_get_arg_type(&iterator);
            switch (type)
            {
            case DBUS_TYPE_UINT32:
                {
                    dbus_message_iter_get_basic(&iterator, &uid);
                    break;
                }
            }
            dbus_message_unref(reply);
        }
        dbus_message_unref(msg);

    }
    mDataPool->getLogger()->information(Poco::format("user id %x ", uid));
    return uid;
}

void CDBusConsumer::handleNameOwnerChangeEvent(DBusMessage* msg) {

    if (dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_SIGNAL)
    {

        std::string sender(dbus_message_get_sender(msg));
        std::string interface(dbus_message_get_interface(msg));
        std::string member(dbus_message_get_member(msg));

        mDataPool->getLogger()->information(
            Poco::format("Name owner changed %s %s %s", sender, interface,
                         member));
        if (sender.compare("org.freedesktop.DBus") == 0
            && interface.compare("org.freedesktop.DBus") == 0
            && member.compare("NameOwnerChanged") == 0)
        {

            DBusMessageIter iter;
            dbus_message_iter_init(msg, &iter);

            const char *name;
            const char *old_owner_val;
            const char *new_owner_val;

            if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING)
            {
                dbus_message_iter_get_basic(&iter, &name);

                if (dbus_message_iter_next(
                        &iter) && dbus_message_iter_get_arg_type(&iter)
                    == DBUS_TYPE_STRING)
                {
                    dbus_message_iter_get_basic(&iter, &old_owner_val);

                    if (dbus_message_iter_next(
                            &iter) && dbus_message_iter_get_arg_type(&iter)
                        == DBUS_TYPE_STRING)
                    {
                        dbus_message_iter_get_basic(&iter, &new_owner_val);

                        if (*old_owner_val == 0x00 && *new_owner_val != 0x00)
                        {

                            std::string alreadythere(new_owner_val);
                            std::map<std::string, DBusService>::iterator deleteIt =
                                mDataPool->getServicePool().find(
                                    alreadythere);
                            if (mDataPool->getServicePool().end() != deleteIt)
                            {
                                mDataPool->getServicePool().erase(deleteIt);
                                mDataPool->getLogger()->information(
                                    Poco::format("Already there!!!!! %s",
                                                 alreadythere.c_str()));
                            }

                            DBusService service;
                            service.pid = getServicePID(new_owner_val);
                            service.connectionUnixUser = getServiceUID(
                                                             new_owner_val);
                            service.aliaslist.push_back(std::string(name));
                            mDataPool->getServicePool().insert(
                                std::pair<std::string, DBusService>(
                                    std::string(new_owner_val),
                                    service));

                            mDataPool->getNameToOwnerPool().insert(
                                std::pair<std::string, std::string>(
                                    std::string(name),
                                    std::string(new_owner_val)));

                            mDataPool->getLogger()->information(
                                Poco::format(
                                    "OnewChanged New Owner: old %s new %s",
                                    std::string(old_owner_val),
                                    std::string(new_owner_val)));

                        }
                        else if (*old_owner_val != 0x00
                                 && *new_owner_val == 0x00)
                        {
                            std::string oldName(old_owner_val);
                            std::map<std::string, DBusService>::iterator deleteIt =
                                mDataPool->getServicePool().find(oldName);
                            if (mDataPool->getServicePool().end() != deleteIt)
                                mDataPool->getServicePool().erase(deleteIt);

                            std::map<std::string, std::string>::const_iterator it;

                            /*ugly*/
                            for (it = mDataPool->getNameToOwnerPool().begin();
                                 it != mDataPool->getNameToOwnerPool().end();
                                )
                            {

                                if (it->second == oldName)
                                {
                                    mDataPool->getNameToOwnerPool().erase(it++);
                                }
                                else
                                {
                                    ++it;
                                }
                            }
                            mDataPool->getLogger()->information(
                                Poco::format(
                                    "OnewChanged Delete  Owner: old %s new %s",
                                    std::string(old_owner_val),
                                    std::string(new_owner_val)));

                        }
                        else
                        {
                            std::string oldName(old_owner_val);

                            std::map<std::string, DBusService>::iterator deleteIt =
                                mDataPool->getServicePool().find(oldName);
                            if (mDataPool->getServicePool().end() != deleteIt)
                                mDataPool->getServicePool().erase(deleteIt);

                            DBusService service;
                            service.pid = getServicePID(new_owner_val);
                            service.connectionUnixUser = getServiceUID(
                                                             new_owner_val);
                            service.aliaslist.push_back(name);
                            mDataPool->getServicePool().insert(
                                std::pair<std::string, DBusService>(
                                    std::string(new_owner_val),
                                    service));

                            std::map<std::string, std::string>::const_iterator it;

                            for (it = mDataPool->getNameToOwnerPool().begin();
                                 it != mDataPool->getNameToOwnerPool().end();
                                )
                            {

                                if (it->second == oldName)
                                {
                                    mDataPool->getNameToOwnerPool().erase(it++);
                                }
                                else
                                {
                                    ++it;
                                }
                            }
                            mDataPool->getNameToOwnerPool().insert(
                                std::pair<std::string, std::string>(
                                    std::string(name),
                                    std::string(new_owner_val)));

                            mDataPool->getLogger()->information(
                                Poco::format(
                                    "OnewChanged Update  Owner: old %s new %s",
                                    std::string(old_owner_val),
                                    std::string(new_owner_val)));
                        }
                    }
                }
            }
        }
    }

}

bool CDBusConsumer::getUniqueConnectionName(const std::string& name,
        std::string& owner) {

    bool retVal = false;

    std::map<std::string, std::string>::iterator serviceIt =
        mDataPool->getNameToOwnerPool().find(name);

    if (mDataPool->getNameToOwnerPool().end() != serviceIt)
    {
        mDataPool->getLogger()->information("name Found");
        owner = serviceIt->second;
        retVal = true;

    }
    else
    {
        mDataPool->getLogger()->error(
            Poco::format("Owner %s not found in data pool!", name));
    }
    return retVal;
}
bool CDBusConsumer::populateSenderInfo(DBusMessageHeader* hdr,
                                       const char* owner) {

    bool retVal = false;
    if (owner == NULL)
    {
        mDataPool->getLogger()->error("Owner parameter is null");
    }
    else
    {
        std::map<std::string, DBusService>::iterator serviceIt =
            mDataPool->getServicePool().find(owner);

        if (mDataPool->getServicePool().end() != serviceIt)
        {
            mDataPool->getLogger()->warning(
                Poco::format("Sender info found %u",
                             serviceIt->second.pid));
            hdr->set_sender_pid(serviceIt->second.pid);
            hdr->set_sender_user_id(serviceIt->second.connectionUnixUser);

            for (std::list<std::string>::iterator it =
                     serviceIt->
                     second.aliaslist.begin();
                 it != serviceIt->second.aliaslist.end();
                 it++) {

                std::string alias_name = *it;
                std::string owner_name = std::string(owner);
                if (alias_name.length() != 0
                    && alias_name.compare(owner_name) != 0)
                {
                    hdr->add_sender_alias_names(alias_name);
                }
            }

        }
        else
        {
            mDataPool->getLogger()->warning("Sender not found in data pool!");
        }
    }
    return retVal;
}

bool CDBusConsumer::populateReceiverInfo(DBusMessageHeader* hdr,
        const char* owner) {

    bool retVal = false;
    if (owner == NULL)
    {
        mDataPool->getLogger()->error("Owner parameter is null");

    }
    else
    {

        std::map<std::string, DBusService>::iterator serviceIt =
            mDataPool->getServicePool().find(owner);

        if (mDataPool->getServicePool().end() != serviceIt)
        {
            mDataPool->getLogger()->information(
                Poco::format("Sender info found %u",
                             serviceIt->second.pid));
            hdr->set_receiver_pid(serviceIt->second.pid);
            hdr->set_receiver_user_id(serviceIt->second.connectionUnixUser);

            for (std::list<std::string>::iterator it =
                     serviceIt->
                     second.aliaslist.begin();
                 it != serviceIt->second.aliaslist.end();
                 it++) {

                std::string alias_name = *it;
                std::string owner_name = std::string(owner);
                if (alias_name.length() != 0
                    && alias_name.compare(owner_name) != 0)
                {
                    hdr->add_receiver_alias_names(alias_name);
                }
            }

        }
        else
        {
            mDataPool->getLogger()->warning("Sender not found in data pool!");
        }
    }
    return retVal;
}

void CDBusConsumer::createDbusMessageHeader(DBusMessageHeader* hdr,
        DBusMessage *msg, const dbus_uint32_t& serial) {
    switch (dbus_message_get_type(msg))
    {

    case DBUS_MESSAGE_TYPE_METHOD_CALL:
        {

            hdr->set_type(DBUS_MSG_TYPE_METHOD_CALL);
            hdr->set_method_signature(dbus_message_get_signature(msg));
            hdr->set_sender(dbus_message_get_sender(msg));
            populateSenderInfo(hdr, dbus_message_get_sender(msg));
            hdr->set_receiver(dbus_message_get_destination(msg));
            std::string tmp;
            if (getUniqueConnectionName(
                    std::string(dbus_message_get_destination(msg)), tmp))
            {
                populateReceiverInfo(hdr, tmp.c_str());
            }

            mDataPool->getLogger()->information(
                Poco::format("Sender info found %s",
                             std::string(dbus_message_get_destination(msg))));
            hdr->set_path(dbus_message_get_path(msg));
            const char * interface = 0;
            interface = dbus_message_get_interface(msg);
            if (0 != interface)
            {
                hdr->set_interface(interface);
            }
            else
            {
                hdr->set_interface("");
            }
            hdr->set_member(dbus_message_get_member(msg));
            hdr->set_serial(serial);
        }
        break;

    case DBUS_MESSAGE_TYPE_METHOD_RETURN:
        {
            hdr->set_type(DBUS_MSG_TYPE_METHOD_RETURN);
            hdr->set_sender(dbus_message_get_sender(msg));
            populateSenderInfo(hdr, dbus_message_get_sender(msg));
            hdr->set_receiver(dbus_message_get_destination(msg));
            std::string tmp;
            if (getUniqueConnectionName(
                    std::string(dbus_message_get_destination(msg)), tmp))
            {
                populateReceiverInfo(hdr, tmp.c_str());
            }
            hdr->set_reply_serial(serial);
        }
        break;

    case DBUS_MESSAGE_TYPE_ERROR:
        {
            hdr->set_type(DBUS_MSG_TYPE_ERROR);
            hdr->set_sender(dbus_message_get_sender(msg));
            populateSenderInfo(hdr, dbus_message_get_sender(msg));
            hdr->set_receiver(dbus_message_get_destination(msg));
            std::string tmp;
            if (getUniqueConnectionName(
                    std::string(dbus_message_get_destination(msg)), tmp))
            {
                populateReceiverInfo(hdr, tmp.c_str());
            }
            hdr->set_reply_serial(serial);
            hdr->set_error_name(dbus_message_get_error_name(msg));
        }
        break;
    case DBUS_MESSAGE_TYPE_SIGNAL:
        {
            const char* receiver = dbus_message_get_destination(msg);

            if (receiver == NULL)
            {
                hdr->set_receiver("broadcast");
            }
            else
            {
                hdr->set_receiver(receiver);
                std::string tmp;
                if (getUniqueConnectionName(std::string(receiver), tmp))
                {
                    populateReceiverInfo(hdr, tmp.c_str());
                }
            }

            hdr->set_type(DBUS_MSG_TYPE_SIGNAL);
            hdr->set_sender(dbus_message_get_sender(msg));
            populateSenderInfo(hdr, dbus_message_get_sender(msg));
            hdr->set_path(dbus_message_get_path(msg));
            hdr->set_interface(dbus_message_get_interface(msg));
            hdr->set_member(dbus_message_get_member(msg));
            hdr->set_serial(serial);
        }
        break;
    default:
        hdr->set_type(DBUS_MSG_TYPE_INVALID);
        break;
    }
}

void CDBusConsumer::createTraceMessagePayload(DBusMessagePayload* payload,
        DBusMessage* msg) {
    DBusMessageIter iter;
    dbus_message_iter_init(msg, &iter);

    do
    {
        int type = dbus_message_iter_get_arg_type(&iter);

        if (type != DBUS_TYPE_INVALID)
        {
            DBusMessagePayloadItem* item = payload->add_param();
            parsePayloadItem(type, item, &iter);
        }
    }
    while (dbus_message_iter_next(&iter))
        ;
}

void CDBusConsumer::parsePayloadItem(int param_type,
                                     DBusMessagePayloadItem* item, DBusMessageIter* iter) {
    item->set_type(dbusType2ProtoType(param_type));

    switch (param_type)
    {
    case DBUS_TYPE_BYTE:
    case DBUS_TYPE_BOOLEAN:
    case DBUS_TYPE_INT16:
    case DBUS_TYPE_INT32:
    case DBUS_TYPE_INT64:
        {
            //::google::protobuf::int64 val;
            int val = 0;
            dbus_message_iter_get_basic(iter, &val);
            item->set_int_val((::google::protobuf::int64) val);
        }
        break;
    case DBUS_TYPE_UINT16:
    case DBUS_TYPE_UINT32:
    case DBUS_TYPE_UINT64:
        {
            //::google::protobuf::int64 val;
            unsigned int val = 0;
            dbus_message_iter_get_basic(iter, &val);
            item->set_uint_val((::google::protobuf::uint64) val);
        }
        break;
    case DBUS_TYPE_DOUBLE:
        {
            double val = 0;
            dbus_message_iter_get_basic(iter, &val);
            item->set_double_val(val);
        }
        break;
    case DBUS_TYPE_STRING:
    case DBUS_TYPE_SIGNATURE:
    case DBUS_TYPE_OBJECT_PATH:
        {
            const char *val;
            dbus_message_iter_get_basic(iter, &val);
            item->set_str_val(val);
        }
        break;

    case DBUS_TYPE_STRUCT:
    case DBUS_TYPE_ARRAY:
    case DBUS_TYPE_VARIANT:
    case DBUS_TYPE_DICT_ENTRY:
        {
            DBusMessageIter subiter;

            dbus_message_iter_recurse(iter, &subiter);

            do
            {
                int type = dbus_message_iter_get_arg_type(&subiter);

                if (type != DBUS_TYPE_INVALID)
                {
                    DBusMessagePayloadItem* sub_item =
                        item->mutable_composite_val()->add_param();
                    parsePayloadItem(type, sub_item, &subiter);
                }
            }
            while (dbus_message_iter_next(&subiter))
                ;
        }
        break;

    default:
        break;
    }
}

DBusParamType CDBusConsumer::dbusType2ProtoType(int type) {
    DBusParamType t;

    switch (type)
    {
    case DBUS_TYPE_BYTE:
        t = DBUS_MSG_PARAM_TYPE_BYTE;
        break;

    case DBUS_TYPE_BOOLEAN:
        t = DBUS_MSG_PARAM_TYPE_BOOLEAN;
        break;

    case DBUS_TYPE_INT16:
        t = DBUS_MSG_PARAM_TYPE_INT16;
        break;

    case DBUS_TYPE_UINT16:
        t = DBUS_MSG_PARAM_TYPE_UINT16;
        break;

    case DBUS_TYPE_INT32:
        t = DBUS_MSG_PARAM_TYPE_INT32;
        break;

    case DBUS_TYPE_UINT32:
        t = DBUS_MSG_PARAM_TYPE_UINT32;
        break;

    case DBUS_TYPE_INT64:
        t = DBUS_MSG_PARAM_TYPE_INT64;
        break;

    case DBUS_TYPE_UINT64:
        t = DBUS_MSG_PARAM_TYPE_UINT64;
        break;

    case DBUS_TYPE_DOUBLE:
        t = DBUS_MSG_PARAM_TYPE_DOUBLE;
        break;

    case DBUS_TYPE_STRING:
        t = DBUS_MSG_PARAM_TYPE_STRING;
        break;

    case DBUS_TYPE_SIGNATURE:
        t = DBUS_MSG_PARAM_TYPE_SIGNATURE;
        break;

    case DBUS_TYPE_OBJECT_PATH:
        t = DBUS_MSG_PARAM_TYPE_OBJ_PATH;
        break;

    case DBUS_TYPE_ARRAY:
        t = DBUS_MSG_PARAM_TYPE_ARRAY;
        break;

    case DBUS_TYPE_STRUCT:
        t = DBUS_MSG_PARAM_TYPE_STRUCT;
        break;

    case DBUS_TYPE_VARIANT:
        t = DBUS_MSG_PARAM_TYPE_VARIANT;
        break;

    case DBUS_TYPE_DICT_ENTRY:
        t = DBUS_MSG_PARAM_TYPE_DICT_ENTRY;
        break;

    default:
        t = DBUS_MSG_PARAM_TYPE_INVALID;
        break;
    }

    return t;
}

void CDBusConsumer::start() {
    DBusError error;
    dbus_error_init(&error);
    dbus_threads_init_default();

    mSecondconnection = dbus_bus_get_private(mDataPool->getObservedBus(), &error);

    if ((mSecondconnection != NULL) && !dbus_error_is_set(&error))
    {

        dbus_bus_get_unique_name(mSecondconnection);
    }
    mDbusConsumerTask.start();
}

void CDBusConsumer::stop() {
    mDbusConsumerTask.stop();
    mDbusConsumerTask.wait();
    if (mSecondconnection != NULL)
    {
        dbus_connection_close(mSecondconnection);
        dbus_connection_unref(mSecondconnection);
        mSecondconnection = NULL;
    }
    mDbusConsumerTask.stop();

}

}
