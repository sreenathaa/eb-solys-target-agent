/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef PLUGINS_LINUX_APP_STATS_PLUGIN_INC_SOCKET_DESCRIPTOR_HPP_
#define PLUGINS_LINUX_APP_STATS_PLUGIN_INC_SOCKET_DESCRIPTOR_HPP_

#define PORT 7778

#include <netinet/in.h>

#include "Poco/Mutex.h"
#include "Poco/Logger.h"

namespace LinuxAppStatsPlugin {

class CSocketInfo {
public:
    CSocketInfo();
    bool initialize();
    void deinitialize();
    void sendWakeupEvent(void);
    void handleNewConnection(void);
    void handleConnectionClosed(unsigned int& socketIdx);
    bool waitForEventOnSocket(void);
    void retrieveCurrentSocketList(int * socketListCopy, int* nrOfConnCopy);
    void closeSocket(int fd);
    unsigned int getNrOfConnection(void) const {
        return mNrOfConnections;
    }
public:
    int mMasterSocketDescriptor,mMaxSocketDescriptor;
    unsigned int mNrOfConnections, mMaxNrOfClients;
    struct sockaddr_in mAddress;
    fd_set readfds;
    static const unsigned int SOCKET_LIST_SIZE = 30;
    static const unsigned int POSITION_FREE = 0;
    int mSocketList[SOCKET_LIST_SIZE];
    Poco::Mutex mMutex;
    Poco::Logger* mLogger;
};

}

#endif /* PLUGINS_LINUX_APP_STATS_PLUGIN_INC_SOCKET_DESCRIPTOR_HPP_ */
