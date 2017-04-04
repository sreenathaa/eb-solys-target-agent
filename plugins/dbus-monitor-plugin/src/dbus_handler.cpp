/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "dbus_data_pool.h"
#include "dbus_handler.h"
#include "dbus_consumer.h"
#include "dbus_producer.h"

#include <iostream>

namespace TargetAgentDbusMonitor {

CDBusHandler::CDBusHandler(DbusMonitorMessageSender* sender) {
    mDataPool = new CDataPool();
    mDataPool->setMessageSender(sender);

    mProducer = new CDBusProducer();
    mProducer->setDataPool(mDataPool);

    mConsumer = new CDBusConsumer();
    mConsumer->setDataPool(mDataPool);
}

void CDBusHandler::start(void) {
    mDataPool->getLogger()->information("CDbusHandler start");

    mProducer->start();

    mConsumer->start();
}

void CDBusHandler::stop(void) {
    mDataPool->getLogger()->information("Before consumer stop");
    mDataPool->notifyMessagesAvailable();
    mConsumer->stop();
    mDataPool->getLogger()->information("After consumer stop");
    mDataPool->getLogger()->information("Before Producer stop");
    mProducer->stop();
    mDataPool->getLogger()->information("After producer stop");

}

}
