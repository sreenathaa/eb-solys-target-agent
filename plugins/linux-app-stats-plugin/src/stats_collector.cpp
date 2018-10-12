/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/


#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Poco/Net/NetException.h"

#include "../inc/linux_app_stats_plugin.h"
#include "../inc/stats_collector.h"
#include "../inc/socket_descriptor.hpp"
#include "../inc/periodic_task.hpp"

using namespace std;
using namespace Poco::Net;

namespace LinuxAppStatsPlugin {

char CThreadStatsCollector::buffer[] = { 0 };

CThreadStatsCollector::CThreadStatsCollector(CLinuxAppStatsPlugin* sender,
        std::string query, unsigned int samplingRate) :
        mConnAcceptor(this, &CThreadStatsCollector::mainReceiverLoop), mProtHandler(
            this, &CThreadStatsCollector::mainSenderLoop), mSender(sender), protoMsg(
                TargetAgent::Protocol::LinuxAppStats::AppStatistics::default_instance().New()), logger(
                    &(Poco::Logger::get("TargetAgent.CThreadStatsPlugin"))), mQuery(
                query), mSamplingRate(samplingRate), mTimerInfo(), mSock(), mSingleWritterMutex() {
}

void CThreadStatsCollector::start() {

    if (mSock.initialize())
    {
        mConnAcceptor.start();
        mProtHandler.start();
    }
    else
        logger->error("failure on socket initialization");
}

void CThreadStatsCollector::mainReceiverLoop() {

#ifdef _POSIX_THREADS
    if (0 != pthread_setname_np(pthread_self(), "PAppStatsRecv"))
    {
        logger->error("failed to set the thread name for PAppStatsRecv");
    }
#endif

    logger->error(Poco::format("Waiting for connections on port %d", PORT));

    while (!mConnAcceptor.isStopped())
    {

        if (mSock.waitForEventOnSocket() && !mConnAcceptor.isStopped())
        {

            mSock.handleNewConnection();

            int nrOfBytes;

            for (unsigned int idx = 0; idx < mSock.mMaxNrOfClients; idx++)
            {

                if ((CSocketInfo::POSITION_FREE != mSock.mSocketList[idx])
                    && FD_ISSET(mSock.mSocketList[idx], &mSock.readfds))
                {
                    if ((nrOfBytes = read(mSock.mSocketList[idx], buffer,
                                          START_FLAG_LENGTH)) == CSocketInfo::POSITION_FREE)
                    {
                        mSock.handleConnectionClosed(idx);
                        logger->error(
                            Poco::format(
                                "client disconnected  ip %s , port %hu",
                                std::string(
                                    inet_ntoa(
                                        mSock.mAddress.sin_addr)),
                                ntohs(mSock.mAddress.sin_port)));
                    }
                    else
                    {
                        processMessage(buffer, nrOfBytes,
                                       mSock.mSocketList[idx]);
                    }
                }
            }
        }

    }
}

void CThreadStatsCollector::processMessage(char* buffer, int nrOfBytes,
        int socketDescriptor) {
    buffer[nrOfBytes] = '\0';

    if (buffer[0] == '$')
    {
        nrOfBytes = read(socketDescriptor, buffer, HEADER_LENGTH);
        if (nrOfBytes >= 0)
        {
            buffer[HEADER_LENGTH - 1] = '\0';
            int length = 0;
            sscanf(buffer, "%x", &length);
            nrOfBytes = read(socketDescriptor, buffer, length);
            if (nrOfBytes > 0)
            {
                buffer[nrOfBytes] = '\0';
            }

            protoMsg->Clear();
            protoMsg->set_trace(buffer, nrOfBytes);
            mSender->sendRawTMessage(protoMsg);

            logger->warning(Poco::format("%s", protoMsg->Utf8DebugString()));
        }

    }
}
void CThreadStatsCollector::sendQuery(const std::string & query) {
    int socketList[CSocketInfo::SOCKET_LIST_SIZE];
    int nrOfClients = 0;
    int idx = 0, nrOfBytes = 0;
    Poco::Mutex::ScopedLock lock(mSingleWritterMutex);
    mSock.retrieveCurrentSocketList(socketList, &nrOfClients);

    for (idx = 0; idx < nrOfClients; idx++)
    {

        if ((nrOfBytes = send(socketList[idx], query.c_str(), query.length(), 0))
            != (int) query.length())
        {
            logger->information(
                Poco::format("send query failed bytes send %d %s",
                             nrOfBytes, query));
        }
        else
            logger->information(Poco::format("send query %s", query));

    }
}
void CThreadStatsCollector::mainSenderLoop() {

#ifdef _POSIX_THREADS
    if (0 != pthread_setname_np(pthread_self(), "PAppStatsSender"))
    {
        logger->error("failed to set the thread name for PAppStatsSender");
    }
#endif
    if (mTimerInfo.configureTimer(mSamplingRate))
    {
        while (!mProtHandler.isStopped())
        {

            mTimerInfo.waitTimerTrigger();

            sendQuery(mQuery);
        }
    }
}

void CThreadStatsCollector::stop() {
    mConnAcceptor.stop();
    mSock.sendWakeupEvent();
    mConnAcceptor.wait();
    mProtHandler.stop();
    mTimerInfo.wakeupEvent();
    mProtHandler.wait();
    mSock.deinitialize();
    logger->error("CThreadStatsCollector stopped");
}
}
;
