/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "Poco/Logger.h"
#include "Poco/File.h"
#include "Serializer.hpp"

namespace TargetAgentTests {

namespace MessageRecorderTests {

using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::InSequence;

TEST(SerializerTests, serializeMessage) {

    static unsigned char buffer[1024];
    bool limitReached = false;
    TargetAgent::Protocol::Ctrl::ProtHandlerCtrlMessage ctrlMsg;
    ctrlMsg.set_id(
        TargetAgent::Protocol::Ctrl::PROT_HNDLR_CTRL_EVT_VERSION_INFO_ID);

    TargetAgent::Protocol::Ctrl::ProtHandlerCtrlEvtVersionInfo* versionInfoMsg =
        ctrlMsg.mutable_versioninfo();
    versionInfoMsg->set_majorversion(
        TargetAgent::Protocol::Ctrl::PROT_HNDLR_CTRL_MAJOR_VERSION);
    versionInfoMsg->set_minorversion(
        TargetAgent::Protocol::Ctrl::PROT_HNDLR_CTRL_MINOR_VERSION);
    ctrlMsg.SerializeToArray((void*) buffer, ctrlMsg.ByteSize());

    TargetAgent::PluginInterface::ProtocolMessage *tmpObj =
        new TargetAgent::PluginInterface::ProtocolMessage(
            TargetAgent::Protocol::CommonDefinitions::MSG_TYPE_PROT_HNDLR_CONTROL,
            ctrlMsg.ByteSize(), buffer, (unsigned long long) 5);
    Poco::File storage("aTemporaryFile.temp");
    TargetAgent::MessageRecorder::CSerializer * serializer =
        new TargetAgent::MessageRecorder::CSerializer();

    serializer->openStream(storage.path().c_str());

    serializer->serializeMessage(tmpObj, limitReached);

    serializer->closeStream();

    std::ifstream fileReader("aTemporaryFile.temp",std::ios::in | std::ios::binary);

    Poco::BinaryReader reader(fileReader);

    serializer->deserialzeMessage(reader, &tmpObj);

    ASSERT_TRUE(tmpObj->getMessageType() == TargetAgent::Protocol::CommonDefinitions::MSG_TYPE_PROT_HNDLR_CONTROL);
}

TEST(SerializerTests, streamNotOpened) {

    static unsigned char buffer[1024];
    bool limitReached = false;
    TargetAgent::Protocol::Ctrl::ProtHandlerCtrlMessage ctrlMsg;
    ctrlMsg.set_id(
        TargetAgent::Protocol::Ctrl::PROT_HNDLR_CTRL_EVT_VERSION_INFO_ID);

    TargetAgent::Protocol::Ctrl::ProtHandlerCtrlEvtVersionInfo* versionInfoMsg =
        ctrlMsg.mutable_versioninfo();
    versionInfoMsg->set_majorversion(
        TargetAgent::Protocol::Ctrl::PROT_HNDLR_CTRL_MAJOR_VERSION);
    versionInfoMsg->set_minorversion(
        TargetAgent::Protocol::Ctrl::PROT_HNDLR_CTRL_MINOR_VERSION);
    ctrlMsg.SerializeToArray((void*) buffer, ctrlMsg.ByteSize());

    TargetAgent::PluginInterface::ProtocolMessage *tmpObj =
        new TargetAgent::PluginInterface::ProtocolMessage(
            TargetAgent::Protocol::CommonDefinitions::MSG_TYPE_PROT_HNDLR_CONTROL,
            ctrlMsg.ByteSize(), buffer, (unsigned long long) 5);
    TargetAgent::MessageRecorder::CSerializer * serializer =
        new TargetAgent::MessageRecorder::CSerializer();


    serializer->serializeMessage(tmpObj, limitReached);


    ASSERT_TRUE(false ==serializer->serializeMessage(tmpObj, limitReached));
}

TEST(SerializerTests, InvalidHeader) {

    static unsigned char buffer[3096];
    bool limitReached = false;
    TargetAgent::Protocol::Ctrl::ProtHandlerCtrlMessage ctrlMsg;
    ctrlMsg.SerializeToArray((void*) buffer, ctrlMsg.ByteSize());

    TargetAgent::PluginInterface::ProtocolMessage *tmpObj =
        new TargetAgent::PluginInterface::ProtocolMessage(
            TargetAgent::Protocol::CommonDefinitions::MSG_TYPE_PROT_HNDLR_CONTROL,
            500, buffer, (unsigned long long) 3096);
    Poco::File storage("aTemporaryFile.temp");
    TargetAgent::MessageRecorder::CSerializer * serializer =
        new TargetAgent::MessageRecorder::CSerializer();

    serializer->serializeMessage(tmpObj, limitReached);

    serializer->closeStream();

    ASSERT_TRUE(false ==serializer->serializeMessage(tmpObj, limitReached));
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
