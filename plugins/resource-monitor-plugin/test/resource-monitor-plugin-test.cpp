/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#include <cstdio>
#include <vector>
#include <map>
#include <iostream>

#include "Poco/File.h"
#include "Poco/Timestamp.h"
#include "Poco/Thread.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "resource_monitor_plugin_mocks.hpp"

#include "TargetAgentPluginInterface.h"
#include "TargetAgentTsProviderInterface.h"

#include "resource-monitor-plugin.h"
#include "resource-monitor-scheduler.h"
#include "procFsReader.h"

namespace ResourceMonitor {
using namespace TargetAgent;
using namespace Log4jReaderMocks;

namespace Tests {

using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::InSequence;

class ResourceMonitorTest: public ::testing::Test {
protected:
    ResourceMonitorPlugin* resourceMonitorPlugin;
    ResourceMonitorScheduler* scheduler;

    ResourceMonitorTest() :
    resourceMonitorPlugin(NULL), scheduler() {
        CMessageDispatcher* taDispatcherMock =
            new Log4jReaderMocks::TaDispatcher();
        PluginInterface::CTimestampProvider* tsProvider =
            new Log4jReaderMocks::CTimestampProviderMock();
        resourceMonitorPlugin = new ResourceMonitorPlugin(taDispatcherMock,
                                tsProvider);
    }

    virtual ~ResourceMonitorTest() {
        delete resourceMonitorPlugin;
    }

    void createResourceMonitor() {
        CMessageDispatcher* taDispatcherMock =
            new Log4jReaderMocks::TaDispatcher();
        PluginInterface::CTimestampProvider* tsProvider =
            new Log4jReaderMocks::CTimestampProviderMock();
        resourceMonitorPlugin = new ResourceMonitorPlugin(taDispatcherMock,
                                tsProvider);
    }

    void onMessageReceivedResourceMonitor(int payloadLength,
                                          const unsigned char* payloadBuffer) {
        resourceMonitorPlugin->onMessageReceived(payloadLength, payloadBuffer);
    }

    void MessageTypeResourceMonitor() {
        resourceMonitorPlugin->MessageType();

    }

    void onHostConnectionEstablishedResourceMonitor() {
        resourceMonitorPlugin->onConnectionEstablished();
    }

    void onHostConnectionLostResourceMonitor() {
        resourceMonitorPlugin->onConnectionLost();
    }

    void startResourceMonitor() {
        resourceMonitorPlugin->startPlugin();
    }

    void stopResourceMonitor() {
        resourceMonitorPlugin->stopPlugin();
    }

};

TEST_F(ResourceMonitorTest, createResourceMonitorTest) {
    resourceMonitorPlugin = NULL;
    ResourceMonitorTest::createResourceMonitor();
    ASSERT_TRUE(resourceMonitorPlugin != 0);
}

TEST_F(ResourceMonitorTest, onMessageReceivedTest) {
    resourceMonitorPlugin = NULL;
    ResourceMonitorTest::createResourceMonitor();
    resourceMonitorPlugin->onMessageReceived(5, 0);
}

TEST_F(ResourceMonitorTest, startSchedulerTest) {
    scheduler = new ResourceMonitorScheduler((new Log4jReaderMocks::TaDispatcher()),(new Log4jReaderMocks::CTimestampProviderMock()));
    std::vector<int> observed;
    std::vector<std::string> unresolvedProcesses;
    observed.push_back(100);

    scheduler->setConfiguration(observed, 500,unresolvedProcesses);
    scheduler->startMonitoring();
    sleep(1);
    scheduler->stopMonitoring();

    ASSERT_TRUE(resourceMonitorPlugin != 0);
}

TEST_F(ResourceMonitorTest, setConfig) {
    resourceMonitorPlugin = NULL;
    ResourceMonitorTest::createResourceMonitor();
    std::map<std::string, std::string> conf;
    conf.insert(std::pair<std::string,std::string>("procName",""));
    conf.insert(std::pair<std::string,std::string>("procName","top"));
    conf.insert(std::pair<std::string,std::string>("samplingRate",""));
    conf.insert(std::pair<std::string,std::string>("samplingRate","100"));
    conf.insert(std::pair<std::string,std::string>("samplingRate","100000"));
    conf.insert(std::pair<std::string,std::string>("procID","1"));
    resourceMonitorPlugin->setConfig(conf);

    ASSERT_TRUE(resourceMonitorPlugin != 0);
}

TEST_F(ResourceMonitorTest, startStopResourceMonitor) {
    resourceMonitorPlugin = NULL;
    ResourceMonitorTest::createResourceMonitor();
    std::shared_ptr<TaDispatcher> dispatcher;
    dispatcher.reset(new Log4jReaderMocks::TaDispatcher());
    std::map<std::string, std::string> emptyPluginConfiguration;
    resourceMonitorPlugin->setConfig(emptyPluginConfiguration);
    ResourceMonitorTest::startResourceMonitor();
    sleep(1);
    ResourceMonitorTest::stopResourceMonitor();
    ASSERT_TRUE(resourceMonitorPlugin != 0);
}

TEST_F(ResourceMonitorTest, procFSTestReadMemoryPos) {
    sysMemInfo memInfo = { 0 };
    bool result = readMeminfo(&memInfo);
    ASSERT_TRUE(result);
}

TEST_F(ResourceMonitorTest, procFSTestReadProcIdNeg) {
    int procId = 0;
    unsigned int actualNumerOfPids = 0;
    bool result = getProcIDbyName("",1, &procId,&actualNumerOfPids);
    ASSERT_FALSE(result);
}

TEST_F(ResourceMonitorTest, procFSTestReadProcIdPos) {
    int procId = 0;
    int pidsArray[10];
    unsigned int occurences;
    bool result = getProcIDbyName("init",10, pidsArray, &occurences);
    ASSERT_TRUE(result);
}

TEST_F(ResourceMonitorTest, procFSTestReadProcFSPos) {
    descriptor entryInfo = { 0 };
    sstat statInfo = { 0 };
    sstatm statmInfo = { 0 };

    entryInfo.dir = opendir(PROC_FS_ROOT);
    entryInfo.dp = NULL;
    entryInfo.pid = 0;

    bool readingValid = true;

    do
    {
        readingValid = readingValid
                       && readProcFs(&entryInfo, &statInfo, &statmInfo);
    }
    while (0 != entryInfo.dp);

    ASSERT_FALSE(readingValid);
}

}
;
}
;

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();

}
