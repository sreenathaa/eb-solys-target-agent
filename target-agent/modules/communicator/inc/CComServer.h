/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef COM_SERVER_H_
#define COM_SERVER_H_



#include "Poco/NotificationQueue.h"
#include "Poco/Activity.h"
#include "Poco/Thread.h"
#include "Poco/Mutex.h"
#include "Poco/Condition.h"
#include "Poco/Types.h"
#include "Poco/Net/StreamSocket.h"


namespace TargetAgent{
namespace Communicator{


class CComServer {

public:
    CComServer(Poco::NotificationQueue& notificationQueue);
    virtual ~CComServer();

    void start (Poco::UInt16 port);
    void stop();

    void signalNextServerRun();

protected:
    void runComServer();

private:
    void waitForCurrentConnectionExpires();
    void wakeUpComServerActivity();
    void notifyNewHostConnection ();

    CComServer(const CComServer& that);
    const CComServer&  operator=(const CComServer& rhs);

private:
    Poco::UInt16 port;
    Poco::Net::StreamSocket mSocket;
    Poco::NotificationQueue& notificationQueue;
    Poco::Mutex connectionMutex;
    Poco::Condition connectionCond;
    bool connectionPredicate;
    Poco::Activity<CComServer> comServerActivity;
};

};
};


#endif /* COM_SERVER_H_ */
