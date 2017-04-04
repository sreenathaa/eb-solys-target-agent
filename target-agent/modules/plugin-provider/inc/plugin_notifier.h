/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef CONNECTION_STATUS_NOTIFIER_H_
#define CONNECTION_STATUS_NOTIFIER_H_


#if defined(_WIN32)
 #define LIBRARY_API __declspec(dllexport)
#else
 #define LIBRARY_API
#endif
#include <protocol_message.h>


namespace TargetAgent{
namespace PluginManagement{

class LIBRARY_API ConnectionStatusNotifier{
public:
    virtual ~ConnectionStatusNotifier(){
    }
    virtual void onHostConnectionEstablished() = 0;
    virtual void onHostConnectionLost() = 0;
    virtual void onMessageReceived( const PluginInterface::ProtocolMessage* msg)= 0;

};

};
};


#endif /* PROTOCOL_MESSAGE_RECEIVER_H_ */
