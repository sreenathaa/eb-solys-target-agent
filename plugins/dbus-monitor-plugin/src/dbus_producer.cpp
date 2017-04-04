/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include <unistd.h>

#include "dbus_data_pool.h"
#include "dbus_producer.h"

#define READ_DELAY 2000

#define EAVESDROPPING_RULE "eavesdrop=true"

static DBusHandlerResult monitor_filter_func(DBusConnection *connection,
        DBusMessage *message, void *user_data) {
    TargetAgentDbusMonitor::CDBusProducer* dbusProducer =
        static_cast<TargetAgentDbusMonitor::CDBusProducer*>(user_data);
    dbusProducer->handleDbusMessage(message);
    return DBUS_HANDLER_RESULT_HANDLED;
}

namespace TargetAgentDbusMonitor {

CDBusProducer::CDBusProducer() :
dbusProducerTask(this, &CDBusProducer::dbusMonitorTask) {
}

CDBusProducer::~CDBusProducer() {
    mDataPool->closeDbusConnection();
}

void CDBusProducer::dbusMonitorTask(void) {

#ifdef _POSIX_THREADS
    if (0 != pthread_setname_np(pthread_self(), "PDBusProducer"))
    {
        mDataPool->getLogger()->error("Failed to set thread name for PDBusProducer");
    }
#endif

    DBusError error;
    dbus_error_init(&error);

    mDataPool->getLogger()->information(
        Poco::format("Use the following bus %d",
                     (int) mDataPool->getObservedBus()));

    dbus_threads_init_default();

    dbusconnection = dbus_bus_get_private(mDataPool->getObservedBus(), &error);

    if ((dbusconnection != NULL) && !dbus_error_is_set(&error))
    {
        dbus_connection_set_exit_on_disconnect(dbusconnection, FALSE);
        mDataPool->getLogger()->information(
            "exit on disconnect disabled for first dbus connection");

        mDataPool->setDbusConnection(dbusconnection);
        mDataPool->connectionEstablished();
        mDataPool->waitDpPopulated();
        mDataPool->getLogger()->information("Consumer triggered!");
        if (dbusMonitorAddMatchRule())
        {
            DBusHandleMessageFunction filter_func = monitor_filter_func;
            if (dbus_connection_add_filter(dbusconnection, filter_func, this,
                                           NULL))
            {
                while (!dbusProducerTask.isStopped())
                {
                    dbus_connection_read_write_dispatch(dbusconnection, -1);
                    mDataPool->getLogger()->information("Next run...");
                }
            }
        }
    }
    else
    {
        mDataPool->getLogger()->error("Failed to connect to bus");
        mDataPool->connectionEstablished();
        dbusconnection = NULL;
    }

}

bool CDBusProducer::dbusMonitorAddMatchRule() {
    DBusError error;
    dbus_error_init(&error);
    DBusConnection* dbusconnection = mDataPool->getDbusConnection();

    /*todo remove unconditional jump statement*/
    if (mDataPool->getDbusDeamonVersion() == 4)
    {
        dbus_bus_add_match(dbusconnection, "type='signal'", &error);
        if (dbus_error_is_set(&error))
            goto error_case;
        dbus_bus_add_match(dbusconnection, "type='method_call'", &error);
        if (dbus_error_is_set(&error))
            goto error_case;
        dbus_bus_add_match(dbusconnection, "type='method_return'", &error);
        if (dbus_error_is_set(&error))
            goto error_case;
        dbus_bus_add_match(dbusconnection, "type='error'", &error);
        if (dbus_error_is_set(&error))
            goto error_case;
        return true;
    }
    else
    {

        dbus_bus_add_match(dbusconnection, EAVESDROPPING_RULE ",type='signal'",
                           &error);
        if (dbus_error_is_set(&error))
            goto error_case;

        dbus_bus_add_match(dbusconnection,
                           EAVESDROPPING_RULE ",type='method_call'", &error);
        if (dbus_error_is_set(&error))
            goto error_case;

        dbus_bus_add_match(dbusconnection,
                           EAVESDROPPING_RULE ",type='method_return'", &error);
        if (dbus_error_is_set(&error))
            goto error_case;

        dbus_bus_add_match(dbusconnection, EAVESDROPPING_RULE ",type='error'",
                           &error);
        if (dbus_error_is_set(&error))
            goto error_case;
        return true;
    }

error_case:
    return false;

}

void CDBusProducer::handleDbusMessage(DBusMessage *msg) {
    if (msg != NULL)
    {
        mDataPool->getLogger()->information("Push message to DBUS Queue");
        mDataPool->pushMessageToQueue(new DbusMessageEntry(msg));
        mDataPool->notifyMessagesAvailable();
    }
    else
    {
        mDataPool->getLogger()->error("Null message");
    }

}

void CDBusProducer::cleanUpDBusConnection(void) {
    DBusHandleMessageFunction filter_func = monitor_filter_func;
    dbus_connection_remove_filter(mDataPool->getDbusConnection(), filter_func,
                                  (void*) this);
}

}

