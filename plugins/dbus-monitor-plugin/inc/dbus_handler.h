/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef DBUS_HANDLER_H_
#define DBUS_HANDLER_H_


#include "Poco/SharedPtr.h"


#include "dbus-monitor-plugin-message-sender.h"

namespace TargetAgentDbusMonitor
{

class CDataPool;
class CDBusProducer;
class CDBusConsumer;


class CDBusHandler
{
public:

    CDBusHandler(DbusMonitorMessageSender* sender);
    void start(void);
    void stop(void);
    ~CDBusHandler(){
    }

    Poco::SharedPtr<CDataPool> getDataPool(void){
        return mDataPool;
    }

private:
    Poco::SharedPtr<CDataPool>   mDataPool;
    Poco::SharedPtr<CDBusProducer> mProducer;
    Poco::SharedPtr<CDBusConsumer> mConsumer;


    CDBusHandler();
    CDBusHandler(const CDBusHandler& other);
    CDBusHandler& operator=(const CDBusHandler& other);
};

}

#endif /* DBUS_HANDLER_H_ */
