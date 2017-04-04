/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#ifndef DLT_MESSAGE_SENDER_
#define DLT_MESSAGE_SENDER_

//Include header file generated from plugin specific proto file
#include "target_agent_prot_dlt_monitor_plugin.pb.h"
#include "target_agent_prot_most_spy.pb.h"

namespace DltMonitorPlugin {
class DltClientMessageSender {
public:

    virtual ~DltClientMessageSender() {
    }
    virtual void sendDLTMessage(
        TargetAgent::Protocol::DLTLogInspector::DLTLogInspectorMessage* message)=0;
    virtual void sendMOSTMessage(
        TargetAgent::Protocol::MostSpy::MostSpyApplicationMessage* message)=0;
};

}
;
#endif

