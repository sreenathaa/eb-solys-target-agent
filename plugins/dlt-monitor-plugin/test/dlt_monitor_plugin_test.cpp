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
#include "dlt_mocks.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "dlt-monitor-plugin.h"
#include "dlt-client-activity.h"
#include "TargetAgentPluginInterface.h"

using namespace TargetAgent;
namespace DltMonitorPlugin {
namespace Tests {

using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::InSequence;

class DltMonitorPluginTest: public ::testing::Test {
protected:

    DltMonitorPlugin* dltMonitorPlugin;
    DltMessageHandler* dltMessageHandler;

    DltMonitorPluginTest() {

    }

    virtual ~DltMonitorPluginTest() {
        //cleaning up
        if(0 != dltMonitorPlugin )
            delete dltMonitorPlugin;
    }

    void createDltMonitorPlugin() {
        CMessageDispatcher* taDispatcherMock =
            new TaDispatcherMocker::TaDispatcher();
        PluginInterface::CTimestampProvider* tsProvider =
            new TaDispatcherMocker::CTimestampProviderMock();
        dltMonitorPlugin = new DltMonitorPlugin(taDispatcherMock, tsProvider);

    }

    void startDltMonitorPlugin() {
        dltMonitorPlugin->startPlugin();
    }

    void stopDltMonitorPlugin() {
        dltMonitorPlugin->stopPlugin();
    }

    void sendDltMonitorPluginToFile(
        TargetAgent::Protocol::DLTLogInspector::DLTLogInspectorMessage* message) {
        dltMonitorPlugin->sendDLTMessage(message);
    }

    void shutdownDltMonitorPlugin() {
        dltMonitorPlugin->shutdownPlugin();
    }

    void onHostConnectionEstablishedDltMonitorPlugin() {
        dltMonitorPlugin->onConnectionEstablished();
    }

    void onHostConnectionLostDltMonitorPlugin() {
        dltMonitorPlugin->onConnectionLost();
    }

};

TEST_F(DltMonitorPluginTest, createDltMonitorPluginTest) {
    dltMonitorPlugin = NULL;
    DltMonitorPluginTest::createDltMonitorPlugin();
    DltMonitorPluginTest::shutdownDltMonitorPlugin();
    ASSERT_TRUE(dltMonitorPlugin != NULL);

}

TEST_F(DltMonitorPluginTest, shutDownDltPluginTest) {
    dltMessageHandler = NULL;
    DltMonitorPluginTest::createDltMonitorPlugin();
    DltMonitorPluginTest::shutdownDltMonitorPlugin();
    ASSERT_EQ(0, dltMessageHandler);

}

TEST_F(DltMonitorPluginTest,onHostConnectionEstablishedTest ) {
    dltMonitorPlugin = NULL;
    DltMonitorPluginTest::createDltMonitorPlugin();
    DltMonitorPluginTest::onHostConnectionEstablishedDltMonitorPlugin();
    DltMonitorPluginTest::shutdownDltMonitorPlugin();
}

TEST_F(DltMonitorPluginTest, onHostConnectionLostTest) {
    dltMonitorPlugin = NULL;
    DltMonitorPluginTest::createDltMonitorPlugin();
    DltMonitorPluginTest::onHostConnectionEstablishedDltMonitorPlugin();
    DltMonitorPluginTest::onHostConnectionLostDltMonitorPlugin();
    DltMonitorPluginTest::shutdownDltMonitorPlugin();
}

/*must be checked in the dlt documentation the posibility to create the underlying data type for dlt messages: dltmessage in roder to create meaningfull tests*/


}
;
}
;

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

