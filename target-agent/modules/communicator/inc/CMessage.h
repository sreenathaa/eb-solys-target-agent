/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef COM_BASE_NOTIFICATION_H_
#define COM_BASE_NOTIFICATION_H_

#include "Poco/Notification.h"
#include "Poco/Net/StreamSocket.h"
#include "protocol_message.h"

using namespace Poco::Net;

namespace TargetAgent {
namespace Communicator {

typedef enum CommunicatorNotificationType {
    NTFY_INVALID,
    NTFY_NEW_HOST_CONNECTION_ESTABLISHED,
    NTFY_NEW_MESSAGE_FROM_HOST_RECEIVED,
    NTFY_SOCKET_CONNECTION_CLOSED_BY_HOST,
    NTFY_CONNECTION_WATCHDOG_TIMEOUT
} CommunicatorNotificationType;

class CMessage: public Poco::Notification {

public:
    CMessage(const CommunicatorNotificationType& type,const PluginInterface::ProtocolMessage* payload): mType(type), mPayload(payload){
    }


    ~CMessage(){
    }

    inline const CommunicatorNotificationType& getType() const {
        return mType;
    }

    inline const PluginInterface::ProtocolMessage* getMessage() const {
        return mPayload;
    }
private:
    CommunicatorNotificationType mType;

    const PluginInterface::ProtocolMessage* mPayload;
};

}
;
}
;

#endif /* COM_BASE_NOTIFICATION_H_ */
