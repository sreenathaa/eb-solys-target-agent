/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef DBUS_PRODUCER_H_
#define DBUS_PRODUCER_H_

#include <dbus/dbus.h>

#include "Poco/Activity.h"
#include "Poco/SharedPtr.h"

namespace TargetAgentDbusMonitor {
class CDataPool;

class CDBusProducer {
public:

    CDBusProducer(void);

    void setDataPool(Poco::SharedPtr<CDataPool> dataPool) {
        mDataPool = dataPool;
    }
    virtual ~CDBusProducer();

    void start() {
        dbusProducerTask.start();
    }
    void stop() {
        if (NULL != dbusconnection)
        {
            cleanUpDBusConnection();
            dbus_connection_close(dbusconnection);
            dbus_connection_unref(dbusconnection);
            dbusconnection = NULL;
        }
        dbusProducerTask.stop();
    }
    void handleDbusMessage(DBusMessage *msg);

    void dbusMonitorTask(void);

    void cleanUpDBusConnection(void);
    bool addMatchingRuleVersionIndependent(const char* type);
private:

    Poco::Activity<CDBusProducer> dbusProducerTask;
private:
    DBusConnection* dbusconnection;
    bool dbusMonitorAddMatchRule();
    Poco::SharedPtr<CDataPool> mDataPool;
    CDBusProducer(const CDBusProducer& other);
    CDBusProducer& operator=(const CDBusProducer& other);
};

}

#endif /* DBUS_PRODUCER_H_ */

