/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef CLOGGER_H_
#define CLOGGER_H_



#include <iostream>
#include <sstream>

#include "Poco/NotificationQueue.h"
#include "Poco/Thread.h"
#include "Poco/Runnable.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketConnector.h"
#include "Poco/Channel.h"

#include "CConnector.hpp"



namespace Poco
{
class Logger;
}

namespace TargetAgent
{
namespace logger
{



class CSocketChannel: public Poco::Channel, public Poco::Runnable
{
public:

    CSocketChannel():sa("localhost",1223),reactor(),eventQueue(),connector(0),thread()
    {
    }

    void run()
    {
#ifdef _POSIX_THREADS
        if (0 != pthread_setname_np(pthread_self(), "CLoggerSocket"))
        {
        }
#endif
        Poco::Thread::sleep(TIMEOUT_SERVER_STARTED);
        connector=new CSocketConnector(sa, reactor,&eventQueue);
        thread.start(reactor);
    }
    void log(const Poco::Message& msg)
    {
        Poco::FastMutex::ScopedLock lock(_mutex);
        if(eventQueue.size()>=MAX_NR_OF_ENTRIES)
            eventQueue.clear();
        eventQueue.enqueueNotification(new CLogNotification(msg));

    }


protected:
    ~CSocketChannel()
    {
    }

private:
    static const int MAX_NR_OF_ENTRIES = 100;
    static const unsigned int TIMEOUT_SERVER_STARTED = 2000;
    Poco::Net::SocketAddress sa;
    Poco::Net::SocketReactor reactor;
    Poco::NotificationQueue eventQueue;
    Poco::Net::SocketConnector<CLoggerServiceHandler>* connector;
    Poco::Thread thread;
    static Poco::FastMutex _mutex;
};


class CLogger
{
public:

    CLogger();

    ~CLogger();

    Poco::Logger& getGlobalLogger(void) const
    {
        return *commLogger;
    }
    ;
    Poco::Logger& getCommLogger(void) const
    {
        return *commLogger;
    }
    ;
    Poco::Logger& getConfigLogger(void) const
    {
        return *configLogger;
    }
    ;
    Poco::Logger& getRecorderLogger(void) const
    {
        return *recLogger;
    }
    ;
    Poco::Logger& getPluginProviderLogger(void) const
    {
        return *pluginLogger;
    }
    ;
    Poco::Logger& getRuntimeLogger(void) const
    {
        return *runtimeLogger;
    }
    ;
    void checkReaderAvailability(void);

private:
    CLogger(const CLogger&);
    CLogger& operator =(const CLogger&);
    bool isChannelActive(const std::string& channelName);
private:
    Poco::Logger* taLogger;
    Poco::Logger* commLogger;
    Poco::Logger* configLogger;
    Poco::Logger* recLogger;
    Poco::Logger* pluginLogger;
    Poco::Logger* runtimeLogger;
    std::map<std::string, Poco::Message::Priority> stringToEnum;
    Poco::Thread thr;
    Poco::AutoPtr<CSocketChannel> pSocket;
};

}
;
}
;

#endif /* CLOGGER_H_ */
