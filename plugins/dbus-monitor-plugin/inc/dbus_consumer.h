/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef DBUS_CONSUMER_H_
#define DBUS_CONSUMER_H_

#include <list>
#include <string>
#include <dbus/dbus.h>

#include "Poco/SharedPtr.h"
#include "Poco/Activity.h"

namespace TargetAgentDbusMonitor {

class CDataPool;

class CDBusConsumer {
public:
    void setDataPool(Poco::SharedPtr<CDataPool> dataPool) {
        mDataPool = dataPool;
    }
    bool parseListNamesReply(DBusMessage* reply,
                             std::list<std::string>& namesList);
    bool dbusListNames(std::list<std::string>& names);
    bool dbusGetNameOwner(std::string name, std::string& owner);
    int getServicePID(const char* servicename);
    int getServiceUID(const char* servicename);
    void dbusConsumerLoop(void);
    bool populateDataPool(void);
    bool insertService(std::string& owner);
    void handleNameOwnerChangeEvent(DBusMessage* msg);

    void dbus2proto(const DBusMessage *msg, const dbus_uint32_t& serial,
                    DBusEvtTraceMessage* tracedMsg);

    void start();
    void stop();

    CDBusConsumer() :
            mSecondconnection(0), mDataPool(0), mDbusConsumerTask(this,
            &CDBusConsumer::dbusConsumerLoop) {
    }
    ;
    ~CDBusConsumer() {
    }

    void createDbusMessageHeader(DBusMessageHeader* hdr, DBusMessage *msg,
                                 const dbus_uint32_t& serial);
    void createTraceMessagePayload(DBusMessagePayload* payload,
                                   DBusMessage* msg);
    void parsePayloadItem(int param_type, DBusMessagePayloadItem* item,
                          DBusMessageIter* iter);
    bool populateSenderInfo(DBusMessageHeader* hdr, const char* owner);
    bool populateReceiverInfo(DBusMessageHeader* hdr, const char* owner);
    DBusParamType dbusType2ProtoType(int type);

    bool isNotUniqueConnID(std::string& name) {
        return (name.at(0) != ':');
    }

    bool getUniqueConnectionName(const std::string& name, std::string& owner);

private:
    CDBusConsumer(const CDBusConsumer& other);
    CDBusConsumer& operator=(const CDBusConsumer& other);

private:
    DBusConnection* mSecondconnection;
    Poco::SharedPtr<CDataPool> mDataPool;
    Poco::Activity<CDBusConsumer> mDbusConsumerTask;
};

}

#endif /* DBUS_CONSUMER_H_ */
