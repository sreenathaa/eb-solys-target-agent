/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#include "protocol_message.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "config_mock.hpp"

namespace PluginManagementTests {

using namespace TargetAgent;
using namespace PluginInterface;


namespace Tests {

TEST(ProtocolMessageTest, messageCopyNullPayload) {
    unsigned long long ts = 5;
    TargetAgent::PluginInterface::ProtocolMessage msg1(Protocol::CommonDefinitions::MessageType::MSG_TYPE_DBUS, 0, 0,ts);
    msg1.getMessageLength();
    msg1.getPayloadData();
    TargetAgent::PluginInterface::ProtocolMessage msg2(msg1);
    ASSERT_TRUE(msg1.getMessageType()==Protocol::CommonDefinitions::MessageType::MSG_TYPE_DBUS);
}

TEST(ProtocolMessageTest, messageCopyHavePayload) {
    unsigned char testString[4]={0xef,0xaa,0x03,0x05};
    int size = 4;
    unsigned long long ts = 5;
    TargetAgent::PluginInterface::ProtocolMessage msg1(Protocol::CommonDefinitions::MessageType::MSG_TYPE_DBUS, size, testString,ts);
    msg1.getMessageLength();
    msg1.getPayloadData();
    TargetAgent::PluginInterface::ProtocolMessage msg2(msg1);
    ASSERT_TRUE(msg1.getMessageType()==Protocol::CommonDefinitions::MessageType::MSG_TYPE_DBUS);
}

TEST(ProtocolMessageTest, messageCopyHavePayloadTs ) {
    unsigned char testString[4]={0xef,0xaa,0x03,0x05};
    int size = 4;
    unsigned long long ts = 5;
    TargetAgent::PluginInterface::ProtocolMessage msg1(Protocol::CommonDefinitions::MessageType::MSG_TYPE_DBUS, size, testString,ts);
    TargetAgent::PluginInterface::ProtocolMessage msg2(msg1);
    ASSERT_TRUE(msg1.getMessageType()==Protocol::CommonDefinitions::MessageType::MSG_TYPE_DBUS);
}

TEST(ProtocolMessageTest, messageEquals) {
    int size = 4;
    unsigned long long ts = 5;
    TargetAgent::PluginInterface::ProtocolMessage msg1(Protocol::CommonDefinitions::MessageType::MSG_TYPE_DBUS, 0, 0,ts);
    TargetAgent::PluginInterface::ProtocolMessage msg2(Protocol::CommonDefinitions::MessageType::MSG_TYPE_RESOURCE_MONITOR, 0, 0,ts);
    msg2 = msg1;
    ASSERT_TRUE(msg1.getMessageType()==Protocol::CommonDefinitions::MessageType::MSG_TYPE_DBUS);
}

TEST(ProtocolMessageTest, messageEqualsHavePayload) {
    unsigned char testString[4]={0xef,0xaa,0x03,0x05};
    int size = 4;
    unsigned long long ts = 5;
    TargetAgent::PluginInterface::ProtocolMessage msg1(Protocol::CommonDefinitions::MessageType::MSG_TYPE_DBUS, size, testString,ts);
    TargetAgent::PluginInterface::ProtocolMessage msg2(Protocol::CommonDefinitions::MessageType::MSG_TYPE_RESOURCE_MONITOR, size, testString,ts);
    msg2 = msg1;
    ASSERT_TRUE(msg1.getMessageType()==Protocol::CommonDefinitions::MessageType::MSG_TYPE_DBUS);
}

TEST(ProtocolMessageTest, messageEqualsHavePayloadTs ) {
    unsigned char testString[4]={0xef,0xaa,0x03,0x05};
    int size = 4;
    unsigned long long ts = 5;
    TargetAgent::PluginInterface::ProtocolMessage msg1(Protocol::CommonDefinitions::MessageType::MSG_TYPE_DBUS, size, testString,ts);
    TargetAgent::PluginInterface::ProtocolMessage msg2(Protocol::CommonDefinitions::MessageType::MSG_TYPE_RESOURCE_MONITOR, 4, testString,ts);
    msg2 = msg1;
    ASSERT_TRUE(msg1.getMessageType()==Protocol::CommonDefinitions::MessageType::MSG_TYPE_DBUS);
}

TEST(ProtocolMessageTest, selfAssignment ) {
    unsigned char testString[4]={0xef,0xaa,0x03,0x05};
    int size = 4;
    unsigned long long ts = 5;
    TargetAgent::PluginInterface::ProtocolMessage msg1(Protocol::CommonDefinitions::MessageType::MSG_TYPE_DBUS, size, testString,ts);
    //TargetAgent::PluginInterface::ProtocolMessage msg2(Protocol::CommonDefinitions::MessageType::MSG_TYPE_RESOURCE_MONITOR, 4, testString,ts);
    msg1 = msg1;
    ASSERT_TRUE(msg1.getMessageType()==Protocol::CommonDefinitions::MessageType::MSG_TYPE_DBUS);
}
};
}
;

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();

}
