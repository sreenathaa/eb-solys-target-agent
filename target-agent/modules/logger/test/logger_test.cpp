/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "Poco/Logger.h"


/*Luckily has the logger component no dependencies other the Poco::Logger*/

#include "config_mock.hpp"
#include "CLogger.hpp"

namespace TargetAgentTests {

namespace Logger {

using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::InSequence;
using namespace TargetAgent::Configuration;


TEST(LoggerTests, testFileActive) {
    ConfigChannel ch;
    ch.setName("File");
    ch.setIsChannelActive(true);
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->addLogChannel(ch);
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->setRecordedFile(true);
    TargetAgent::logger::CLogger *log = new TargetAgent::logger::CLogger();
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->clearChannel();
    ASSERT_TRUE(log != 0);
}
TEST(LoggerTests, testFileNa) {
    ConfigChannel ch;
    ch.setName("File");
    ch.setIsChannelActive(false);
    std::string level("PRIO_INFORMATION");
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->setLogLevel(level);
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->addLogChannel(ch);
    ch.setName("Socket");
    ch.setIsChannelActive(true);
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->addLogChannel(ch);
    TargetAgent::logger::CLogger *log = new TargetAgent::logger::CLogger();
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->clearChannel();
    ASSERT_TRUE(log != 0);
}

TEST(LoggerTests, testSocketActive) {
    ConfigChannel ch;
    ch.setName("File");
    ch.setIsChannelActive(false);
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->addLogChannel(ch);
    ch.setName("Socket");
    ch.setIsChannelActive(true);
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->addLogChannel(ch);
    TargetAgent::logger::CLogger *log = new TargetAgent::logger::CLogger();
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->clearChannel();
    ASSERT_TRUE(log != 0);
}

TEST(LoggerTests, testConsoleActive) {
    ConfigChannel ch;
    ch.setName("Console");
    ch.setIsChannelActive(true);
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->addLogChannel(ch);
    ch.setName("Socket");
    ch.setIsChannelActive(true);
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->addLogChannel(ch);
    TargetAgent::logger::CLogger *log = new TargetAgent::logger::CLogger();
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->clearChannel();
    ASSERT_TRUE(log != 0);
}

TEST(LoggerTests, testSocketReaderAvailable) {
    ConfigChannel ch;
    ch.setName("Console");
    ch.setIsChannelActive(true);
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->addLogChannel(ch);
    ch.setName("Socket");
    ch.setIsChannelActive(true);
    std::string pluginName("socket-reader-plugin");
    std::string pluginPath("");
    PluginConfigItem pluginConfigItem(pluginName,pluginPath);
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->addPlugin(pluginConfigItem);
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->addLogChannel(ch);
    TargetAgent::logger::CLogger *log = new TargetAgent::logger::CLogger();
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->clearChannel();
    delete log;
    ASSERT_TRUE(log != 0);
}


TEST(LoggerTests, noRecordFile) {
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->setRecordedFile(false);
    TargetAgent::logger::CLogger *log = new TargetAgent::logger::CLogger();
    ASSERT_TRUE(log != 0);
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
