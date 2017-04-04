/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#ifndef DBUS_MONITOR_MESSAGE_SENDER_
#define DBUS_MONITOR_MESSAGE_SENDER_

#include "target_agent_prot_dbus_monitor.pb.h"

namespace TargetAgentDbusMonitor {

class DbusMonitorMessageSender {
public:

    virtual ~DbusMonitorMessageSender() {
    }
    virtual void sendApplicationMessage(
        DBusApplicationMessage* evtTraceMessage) = 0;

};

}
;
#endif

