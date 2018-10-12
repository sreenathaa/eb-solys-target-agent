/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef STATS_COLLECTOR_H_
#define STATS_COLLECTOR_H_

#include <netinet/in.h>

#include <iostream>

#include "Poco/Mutex.h"
#include "Poco/Activity.h"
#include "Poco/Logger.h"

#include "socket_descriptor.hpp"
#include "periodic_task.hpp"

namespace LinuxAppStatsPlugin {




class CLinuxAppStatsPlugin;

class CThreadStatsCollector {
public:
    CThreadStatsCollector(CLinuxAppStatsPlugin* sender, std::string query,
                          unsigned int samplingRate);
    ~CThreadStatsCollector(){
    }
    void start();
    void stop();
    void mainSenderLoop();
    void mainReceiverLoop();
    void setSamplingRate(unsigned int samplingRate){
        mSamplingRate = samplingRate;
    }
    void processMessage(char* buffer,int nrOfBytes, int socketDescriptor);
    void sendQuery(const std::string & query);
private:
    Poco::Activity<CThreadStatsCollector> mConnAcceptor;
    Poco::Activity<CThreadStatsCollector> mProtHandler;
    CLinuxAppStatsPlugin* mSender;
    TargetAgent::Protocol::LinuxAppStats::AppStatistics* protoMsg;
    Poco::Logger* logger;
    std::string mQuery;
    unsigned int mSamplingRate;
    CPeriodicTaskInfo mTimerInfo;
    CSocketInfo mSock;
    Poco::Mutex mSingleWritterMutex;
    static const unsigned int BUF_SIZE = 256;
    static const unsigned int START_FLAG_LENGTH = 1;
    static const unsigned int HEADER_LENGTH = 3;
    static char buffer[BUF_SIZE + 1];
};

}
;

#endif
