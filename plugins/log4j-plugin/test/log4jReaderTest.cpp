/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "Poco/Timestamp.h"
#include "Poco/Thread.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "TargetAgentPluginInterface.h"
#include "TargetAgentTsProviderInterface.h"
#include "log4jReaderMocks.hpp"
#include "../inc/log4jPlugin.hpp"


namespace TargetAgentlog4jPlugin {
using namespace TargetAgent;
using namespace Log4jReaderMocks;

namespace Tests {

using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::InSequence;

class Clog4jPluginTest: public ::testing::Test {
protected:
    TargetAgentlog4jPlugin::Clog4jPlugin* log4jPlugin;

    Clog4jPluginTest() :
    log4jPlugin(NULL) {
        CMessageDispatcher* taDispatcherMock =
            new Log4jReaderMocks::TaDispatcher();
        PluginInterface::CTimestampProvider* tsProvider = new Log4jReaderMocks::CTimestampProviderMock();
        log4jPlugin = new TargetAgentlog4jPlugin::Clog4jPlugin(0,0);
    }

    virtual ~Clog4jPluginTest() {
        delete log4jPlugin;
    }

    void createLog4j() {
        CMessageDispatcher* taDispatcherMock =
            new Log4jReaderMocks::TaDispatcher();
        PluginInterface::CTimestampProvider* tsProvider =
            new Log4jReaderMocks::CTimestampProviderMock();
        log4jPlugin = new TargetAgentlog4jPlugin::Clog4jPlugin(taDispatcherMock, tsProvider);
    }

    void onMessageReceivedLog4j(int payloadLength,
                                const unsigned char* payloadBuffer) {
        log4jPlugin->onMessageReceived(payloadLength, payloadBuffer);
    }

    void MessageTypeLog4j() {
        log4jPlugin->MessageType();

    }

    void onHostConnectionEstablishedLog4j() {
        log4jPlugin->onConnectionEstablished();
    }

    void onHostConnectionLostLog4j() {
        log4jPlugin->onConnectionLost();
    }

    void startLog4j() {
        log4jPlugin->startPlugin();
    }

    void stopLog4j() {
        log4jPlugin->stopPlugin();
    }

};

TEST_F(Clog4jPluginTest, createLog4jTest) {
    log4jPlugin = NULL;
    Clog4jPluginTest::createLog4j();
    ASSERT_TRUE(log4jPlugin != 0);
}

TEST_F(Clog4jPluginTest, onMessageReceivedTest) {
    log4jPlugin = NULL;
    Clog4jPluginTest::createLog4j();
    log4jPlugin->onMessageReceived(5, 0);
}

TEST_F(Clog4jPluginTest, startPlugin) {
    log4jPlugin = NULL;
    Clog4jPluginTest::createLog4j();
    log4jPlugin->startPlugin();
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
