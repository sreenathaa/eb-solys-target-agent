/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

/*
 * socket_descriptor.cpp
 *
 *  Created on: 26.11.2015
 *      Author: vagrant
 */
#define PRETTY_PRINT_ERRNO(FKT) mLogger->error(Poco::format("%s failed with the following error: %s",std::string(#FKT),std::string(strerror(errno))))

#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "socket_descriptor.hpp"

namespace LinuxAppStatsPlugin {

CSocketInfo::CSocketInfo() :
        mMasterSocketDescriptor(0), mMaxSocketDescriptor(0), mNrOfConnections(
            0), mMaxNrOfClients(30), mMutex(), mLogger(
        &(Poco::Logger::get("TargetAgent.CThreadStatsPlugin"))) {
}
bool CSocketInfo::initialize() {
    bool retVal = true;
    int opt = 1;
    Poco::Logger* logger =
        &(Poco::Logger::get("TargetAgent.CThreadStatsPlugin"));

    for (unsigned int idx = 0; idx < mMaxNrOfClients; idx++)
    {
        mSocketList[idx] = 0;
    }

    if ((mMasterSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        PRETTY_PRINT_ERRNO(socket);
        retVal = false;
    }

    if (setsockopt(mMasterSocketDescriptor, SOL_SOCKET, SO_REUSEADDR,
                   (char *) &opt, sizeof(opt)) < 0)
    {
        PRETTY_PRINT_ERRNO(setsockopt);
        retVal = false;
    }

    mAddress.sin_family = AF_INET;
    mAddress.sin_addr.s_addr = INADDR_ANY;
    mAddress.sin_port = htons(PORT);

    if (bind(mMasterSocketDescriptor, (struct sockaddr *) &mAddress,
             sizeof(mAddress)) < 0)
    {
        PRETTY_PRINT_ERRNO(bind);
        retVal = false;
    }

    if (listen(mMasterSocketDescriptor, 3) < 0)
    {
        logger->error(Poco::format("Listener on port %d \n", (int) PORT));
        retVal = false;
    }
    return retVal;
}

void CSocketInfo::retrieveCurrentSocketList(int * socketListCopy,
        int* nrOfConnCopy) {
    Poco::Mutex::ScopedLock lock(mMutex);
    memcpy(socketListCopy, &mSocketList, SOCKET_LIST_SIZE * sizeof(int));
    *nrOfConnCopy = mNrOfConnections;
}

void CSocketInfo::handleConnectionClosed(unsigned int& socketIdx) {
    Poco::Mutex::ScopedLock lock(mMutex);
    int addrlen = sizeof(mAddress);
    getpeername(mSocketList[socketIdx], (struct sockaddr*) &mAddress,
                (socklen_t*) &addrlen);

    close(mSocketList[socketIdx]);
    mSocketList[socketIdx] = POSITION_FREE;
    --mNrOfConnections;
}

void CSocketInfo::handleNewConnection(void) {
    int addrlen;
    addrlen = sizeof(mAddress);

    if (FD_ISSET(mMasterSocketDescriptor, &readfds))
    {
        int newSocketDescriptor;
        if ((newSocketDescriptor = accept(mMasterSocketDescriptor,
                                          (struct sockaddr *) &mAddress, (socklen_t*) &addrlen)) < 0)
        {
            PRETTY_PRINT_ERRNO(accept);
        }
        else
        {
            Poco::Mutex::ScopedLock lock(mMutex);
            mLogger->error(
                Poco::format(
                    "new connection established socket fd is %d , ip is : %s , port : %hu",
                    newSocketDescriptor,
                    std::string(inet_ntoa(mAddress.sin_addr)),
                    ntohs(mAddress.sin_port)));

            for (unsigned int idx = 0; idx < mMaxNrOfClients; idx++)
            {
                if (mSocketList[idx] == POSITION_FREE)
                {
                    mSocketList[idx] = newSocketDescriptor;
                    mNrOfConnections++;
                    mLogger->error(
                        Poco::format("pushed to the socket list %d", idx));
                    break;
                }
            }
        }
    }
}
void CSocketInfo::sendWakeupEvent(void) {
    int sock;
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        PRETTY_PRINT_ERRNO(socket);
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(7778);

    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0)
    {
        PRETTY_PRINT_ERRNO(connect);
    }
}
bool CSocketInfo::waitForEventOnSocket(void) {
    bool retVal = true;
    int event;
    FD_ZERO(&readfds);
    FD_SET(mMasterSocketDescriptor, &readfds);
    mMaxSocketDescriptor = mMasterSocketDescriptor;

    for (unsigned int idx = 0; idx < mMaxNrOfClients; idx++)
    {
        if (mSocketList[idx] > 0)
            FD_SET(mSocketList[idx], &readfds);

        if (mSocketList[idx] > mMaxSocketDescriptor)
            mMaxSocketDescriptor = mSocketList[idx];
    }

    event = select(mMaxSocketDescriptor + 1, &readfds, NULL, NULL,
                   NULL);

    if ((event < 0) && (errno != EINTR))
    {
        PRETTY_PRINT_ERRNO(select);
        retVal = false;
    }
    return retVal;
}

void CSocketInfo::closeSocket(int fd) {
    if (fd >= 0)
    {
        shutdown(fd, SHUT_RDWR);
        close(fd);
    }
}
void CSocketInfo::deinitialize() {
    for (unsigned int idx = 0; idx < mMaxNrOfClients; idx++)
    {
        closeSocket(mSocketList[idx]);
    }

    closeSocket(mMasterSocketDescriptor);
}
}
