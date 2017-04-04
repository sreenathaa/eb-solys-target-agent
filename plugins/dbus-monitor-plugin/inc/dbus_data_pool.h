/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef DBUS_DATA_POOL_H_
#define DBUS_DATA_POOL_H_

/*exception to the rule for includes*/
#include "target_agent_prot_dbus_monitor.pb.h"

#include <map>
#include <list>
#include <deque>
#include <iostream>

#include <dbus/dbus.h>

#include "Poco/Logger.h"
#include "Poco/Notification.h"
#include "Poco/NotificationQueue.h"
#include "Poco/Condition.h"
#include <Poco/LocalDateTime.h>
#include <Poco/Thread.h>

#include "dbus-monitor-plugin-message-sender.h"

namespace TargetAgentDbusMonitor {
class DbusMessageEntry;

class DBusService{
public:
    std::list<std::string> aliaslist;
    unsigned int     pid;
    unsigned int           connectionUnixUser;
};


class CDataPool {


public:
    CDataPool() :
            mSender(0), mObservedBus(DBUS_BUS_SESSION), mDBusconnection(0),mProtoDbusInstance(
                DBUS_INSTANCE_SESSION_BUS), mWaitConnection(
                    true), mWaitPopulateDP(true), mDbusQueue(), mMsgQMutex(), mMsgQCondition(), mDbusMessageAvailable(
                false) {
        mLogger = &(Poco::Logger::get("TargetAgent.DbusPlugin"));
    }
    virtual ~CDataPool() {
    }

    void setMessageSender(const DbusMonitorMessageSender* sender) {
        mSender = sender;
    }
    DbusMonitorMessageSender* getMessageSender(void) const {
        return const_cast<DbusMonitorMessageSender*>(mSender);
    }

    void setObservedBus(const DBusBusType& bus) {
        mObservedBus = bus;
    }

    const DBusBusType& getObservedBus(void) {
        return mObservedBus;
    }

    Poco::Logger* getLogger(void) {
        return mLogger;
    }


    inline DBusInstanceType getProtoDbusInstanceType(void) {
        DBusInstanceType retVal;
        return retVal =
                   (mObservedBus == DBUS_BUS_SYSTEM) ?
                   DBUS_INSTANCE_SYSTEM_BUS : DBUS_INSTANCE_SESSION_BUS;
    }

    unsigned int getDbusDeamonVersion(void);

    std::map<std::string, DBusService>& getServicePool(void) {
        return mUniqueIDtoService;
    }
    std::map<std::string, std::string>& getNameToOwnerPool(void) {
        return mNametoOwner;
    }

    void setDbusConnection(DBusConnection* connection) {
        mDBusconnection = connection;
    }


    DBusConnection* getDbusConnection(void) const {
        return mDBusconnection;
    }

    void closeDbusConnection(void);

    void connectionEstablished() {
        mWaitConnection.set();
    }

    void waitConnectionEstablished() {
        mWaitConnection.wait();
    }

    void waitDpPopulated() {
        mWaitPopulateDP.wait();
    }

    void dpPopulated() {
        mWaitPopulateDP.set();
    }


    void waitForMessagesAvailable();

    void pushMessageToQueue(DbusMessageEntry* msg);

    void updateMessageAvailableFlag();

    DbusMessageEntry* waitForNextMessageToSend();

    DbusMessageEntry* popMessageFromQueue();

    void notifyMessagesAvailable();

private:

    CDataPool(const CDataPool& other);
    CDataPool& operator=(const CDataPool& other);
private:
    const DbusMonitorMessageSender* mSender;
    DBusBusType mObservedBus;
    Poco::Logger* mLogger;
    DBusConnection* mDBusconnection;
    DBusInstanceType mProtoDbusInstance;
    std::map<std::string,DBusService> mUniqueIDtoService;
    std::map<std::string,std::string> mNametoOwner;
    Poco::Event mWaitConnection;
    Poco::Event mWaitPopulateDP;
    std::deque<DbusMessageEntry*> mDbusQueue;
    Poco::Mutex mMsgQMutex;
    Poco::Condition mMsgQCondition;
    bool mDbusMessageAvailable;
};


class DbusMessageEntry {
public:
    DbusMessageEntry(DBusMessage *message) {

        mMsg = dbus_message_copy(message);

        switch (dbus_message_get_type(message))
        {

        case DBUS_MESSAGE_TYPE_SIGNAL:
        case DBUS_MESSAGE_TYPE_METHOD_CALL:
            {
                mSerial = dbus_message_get_serial (message);
            }
            break;
        case DBUS_MESSAGE_TYPE_METHOD_RETURN:
        case DBUS_MESSAGE_TYPE_ERROR:
            {
                mSerial = dbus_message_get_reply_serial(message);
            }
            break;
        default:
            break;
        }

        timestampInUnixEpochMS = (unsigned long long) (ts.epochMicroseconds()
                                 / 1000);
    }

    ~DbusMessageEntry(void) {
        dbus_message_unref(mMsg);
    }
    DBusMessage* getMessage(void) const {
        return mMsg;
    }

    dbus_uint32_t& getSerial(void) {
        return mSerial;
    }

private:
    DBusMessage * mMsg;
    dbus_uint32_t mSerial;
    Poco::Timestamp ts;
public:
    unsigned long long timestampInUnixEpochMS;

};

}

#endif /* DBUS_DATA_POOL_H_ */
