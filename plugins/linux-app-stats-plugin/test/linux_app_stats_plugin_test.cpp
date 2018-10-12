/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include <cstdio>
#include <vector>
#include <map>
#include <iostream>

#include "Poco/File.h"
#include "Poco/Timestamp.h"
#include "Poco/Thread.h"
#include "Poco/Activity.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "linux_app_stats_plugin.h"
#include "TargetAgentPluginInterface.h"
#include "TargetAgentTsProviderInterface.h"
#include "linux_app_stats_plugin_mocks.hpp"

namespace LinuxAppStatsPlugin {
using namespace TargetAgent;
using namespace Log4jReaderMocks;
using Poco::Thread;

namespace Tests {

using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::InSequence;

class LinuxAppTest: public ::testing::Test {
protected:
    CLinuxAppStatsPlugin* threadStatsPlugin;

    LinuxAppTest() :
    threadStatsPlugin(NULL) {
        CMessageDispatcher* taDispatcherMock = new Log4jReaderMocks::TaDispatcher();
        PluginInterface::CTimestampProvider* tsProvider =
            new Log4jReaderMocks::CTimestampProviderMock();
        threadStatsPlugin = new CLinuxAppStatsPlugin(taDispatcherMock,
                            tsProvider);
    }

    virtual ~LinuxAppTest() {
        delete threadStatsPlugin;
    }

    void createThreadHeapStats() {
        CMessageDispatcher* taDispatcherMock = new Log4jReaderMocks::TaDispatcher();
        PluginInterface::CTimestampProvider* tsProvider =
            new Log4jReaderMocks::CTimestampProviderMock();
        threadStatsPlugin = new CLinuxAppStatsPlugin(taDispatcherMock,
                            tsProvider);
    }

    void onMessageReceivedThreadHeapStats(int payloadLength,
                                          const unsigned char* payloadBuffer) {
        threadStatsPlugin->onMessageReceived(payloadLength, payloadBuffer);
    }

    void MessageTypeThreadHeapStats() {
        threadStatsPlugin->MessageType();

    }

    void onHostConnectionEstablishedThreadHeapStats() {
        threadStatsPlugin->onConnectionEstablished();
    }

    void onHostConnectionLostThreadHeapStats() {
        threadStatsPlugin->onConnectionLost();
    }

    void startThreadHeapStats() {
        threadStatsPlugin->startPlugin();
    }

    void stopThreadHeapStats() {
        threadStatsPlugin->stopPlugin();
    }

};

class ActiveObject {
public:
    ActiveObject() :
    _activity(this, &ActiveObject::runActivity), numberOfTriggers(0) {

    }

    void start() {
        _activity.start();
    }
    void stop() {
        _activity.stop();
        _activity.wait();
    }

    unsigned int getNumberOfTriggers() {
        return numberOfTriggers;
    }
protected:
    void runActivity() {
        periodic.configureTimer(500);

        while (!_activity.isStopped())
        {
            periodic.waitTimerTrigger();
            numberOfTriggers++;
        }
    }

private:
    Poco::Activity<ActiveObject> _activity;
    unsigned int numberOfTriggers;
    CPeriodicTaskInfo periodic;
};

class ActiveSocketObject {
public:
    ActiveSocketObject() :
    _activity(this, &ActiveSocketObject::runActivity) {

    }

    void start() {
        if (mSock.initialize())
        {
            _activity.start();
        }
    }
    void stop() {
        _activity.stop();
        _activity.wait();
    }

protected:
    void runActivity() {

        while (!_activity.isStopped())
        {

            if (mSock.waitForEventOnSocket() && !_activity.isStopped())
            {

                mSock.handleNewConnection();

                int nrOfBytes;

                for (unsigned int idx = 0; idx < mSock.mNrOfConnections;
                     idx++)
                {

                    if (FD_ISSET(mSock.mSocketList[idx], &mSock.readfds))
                    {
                        if ((nrOfBytes = read(mSock.mSocketList[idx], buffer, 1))
                            == 0)
                        {

                        }
                        else
                        {

                        }
                    }
                }
            }

        }
    }

private:
    Poco::Activity<ActiveSocketObject> _activity;
    CSocketInfo mSock;
    unsigned char buffer[500];
}
;

TEST_F(LinuxAppTest, createLinuxAppTest) {
    threadStatsPlugin = NULL;
    LinuxAppTest::createThreadHeapStats();
    ASSERT_TRUE(threadStatsPlugin != 0);
}

TEST_F(LinuxAppTest, onMessageReceivedTest) {
    threadStatsPlugin = NULL;
    LinuxAppTest::createThreadHeapStats();
    std::shared_ptr<TaDispatcher> dispatcher;
    dispatcher.reset(new Log4jReaderMocks::TaDispatcher());
    std::map<std::string, std::string> pluginConfiguration;
    pluginConfiguration.insert(
        std::pair<std::string, std::string>("samplingRate", "100"));
    pluginConfiguration.insert(
        std::pair<std::string, std::string>("Query", "$06 Bonjour"));
    threadStatsPlugin->setConfig(pluginConfiguration);
    LinuxAppTest::startThreadHeapStats();
    threadStatsPlugin->onMessageReceived(60,
                                         (const unsigned char*) "configEntry {mKey: APP_STATISTICS_SET_RESOLUTION  mValue: 5}");
    sleep(1);
    LinuxAppTest::stopThreadHeapStats();
    ASSERT_TRUE(threadStatsPlugin != 0);
}

TEST_F(LinuxAppTest, startStopThreadHeapStats) {
    threadStatsPlugin = NULL;
    LinuxAppTest::createThreadHeapStats();
    std::shared_ptr<TaDispatcher> dispatcher;
    dispatcher.reset(new Log4jReaderMocks::TaDispatcher());
    std::map<std::string, std::string> pluginConfiguration;
    pluginConfiguration.insert(
        std::pair<std::string, std::string>("samplingRate", "100"));
    pluginConfiguration.insert(
        std::pair<std::string, std::string>("Query", "$06 Bonjour"));
    threadStatsPlugin->setConfig(pluginConfiguration);
    LinuxAppTest::startThreadHeapStats();
    sleep(1);
    LinuxAppTest::stopThreadHeapStats();
    ASSERT_TRUE(threadStatsPlugin != 0);
}

TEST_F(LinuxAppTest, numberoftriggers) {
    ActiveObject _activity;
    _activity.start();
    sleep(2);
    ASSERT_TRUE(_activity.getNumberOfTriggers() >= 3);
}

TEST_F(LinuxAppTest, connectionEstablished) {
    ActiveSocketObject _activity;
    _activity.stop();
    ASSERT_TRUE(1);
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
