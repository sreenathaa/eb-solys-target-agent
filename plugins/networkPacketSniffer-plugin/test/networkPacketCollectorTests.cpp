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
#include <tins/tins.h>
#include "networkPacketSnifferPlugin.hpp".h"
#include "TargetAgentPluginInterface.h"
#include "TargetAgentTsProviderInterface.h"

#include "networkPacketCollectorMocks.hpp"

namespace NwSnifferStatsPlugin {
using namespace TargetAgent;
using namespace Log4jReaderMocks;
using Poco::Thread;

namespace Tests {

using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::InSequence;

class NwSnifferTests: public ::testing::Test {
protected:
    CNwSnifferStatsPlugin* nwSnifferPlugin;

    NwSnifferTests() :
    nwSnifferPlugin(NULL) {
        CMessageDispatcher* taDispatcherMock = new Log4jReaderMocks::TaDispatcher();
        PluginInterface::CTimestampProvider* tsProvider =
            new Log4jReaderMocks::CTimestampProviderMock();
        nwSnifferPlugin = new CNe(taDispatcherMock,
                                  tsProvider);
    }

    virtual ~NwSnifferTests() {
        delete nwSnifferPlugin;
    }

    void createNwSniffer() {
        CMessageDispatcher* taDispatcherMock = new Log4jReaderMocks::TaDispatcher();
        PluginInterface::CTimestampProvider* tsProvider =
            new Log4jReaderMocks::CTimestampProviderMock();
        nwSnifferPlugin = new CnetworkPacketSnifferPlugin(taDispatcherMock,
                          tsProvider);
    }

    void onMessageReceivedThreadHeapStats(int payloadLength,
                                          const unsigned char* payloadBuffer) {
        nwSnifferPlugin->onMessageReceived(payloadLength, payloadBuffer);
    }

    void MessageTypeThreadHeapStats() {
        nwSnifferPlugin->MessageType();

    }

    void onHostConnectionEstablishedThreadHeapStats() {
        nwSnifferPlugin->onConnectionEstablished();
    }

    void onHostConnectionLostThreadHeapStats() {
        nwSnifferPlugin->onConnectionLost();
    }

    void startThreadHeapStats() {
        nwSnifferPlugin->startPlugin();
    }

    void stopThreadHeapStats() {
        nwSnifferPlugin->stopPlugin();
    }

};


TEST_F(NwSnifferTests, createNwSnifferTest) {
    nwSnifferPlugin = NULL;
    NwSnifferTests::createNwSniffer();
    ASSERT_TRUE(nwSnifferPlugin != 0);
}

TEST_F(NwSnifferTests, startStopThreadHeapStats) {
    nwSnifferPlugin = NULL;
    NwSnifferTests::createNwSniffer();
    std::shared_ptr<TaDispatcher> dispatcher;
    dispatcher.reset(new Log4jReaderMocks::TaDispatcher());
    std::map<std::string, std::string> pluginConfiguration;
    pluginConfiguration.insert(
        std::pair<std::string, std::string>("samplingRate", "100"));
    pluginConfiguration.insert(
        std::pair<std::string, std::string>("Query", "$06 Bonjour"));
    nwSnifferPlugin->setConfig(pluginConfiguration);
    NwSnifferTests::startThreadHeapStats();
    sleep(1);
    NwSnifferTests::stopThreadHeapStats();
    ASSERT_TRUE(nwSnifferPlugin != 0);
}

TEST_F(NwSnifferTests, createNwSnifferTest) {
    nwSnifferPlugin = NULL;
    NwSnifferTests::createNwSniffer();
    ASSERT_TRUE(nwSnifferPlugin != 0);
}
TEST(ConnectorTest, testMessageParsed) {
    std::vector<int> ports;
    std::string jsonString = "{ \"name\":\"A\", \"age\":1, \"city\":\"B\" }";
    TargetAgentnetworkPacketSnifferPlugin::CPacketSniffer* temp = new TargetAgentnetworkPacketSnifferPlugin::CPacketSniffer(0,"", ports);
    Tins::RawPDU raw = new Tins::RawPDU(jsonString,jsonString.length());
    Tins::TCP *wrapper = new Tins::TCP ();
    wrapper->inner_pdu(raw);
    ASSERT_TRUE(temp->handle(*wrapper));
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
