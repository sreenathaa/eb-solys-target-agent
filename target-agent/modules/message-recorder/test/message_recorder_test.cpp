/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include <cstdio>
#include <vector>
#include <map>
#include "Poco/File.h"
#include "Poco/Timestamp.h"
#include "Poco/Thread.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mock_recorder_config_provider.h"
#include "protocol_message.h"
#include "dummy_receiver.h"
#include "test_message_sender_plugin.h"
//#include "config_mock.hpp"

#include "logger_mock.hpp"
#include "message_recorder.h"

namespace TargetAgent{
namespace MessageRecorder{
namespace Tests{

using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::InSequence;

struct TestData
{
    std::string payload;
    int numberOfMessagesToSend;
    PluginInterface::ProtocolMessage* testMessage;
    long sleepBetweenMessageInMilliseconds;
    TestData(std::string payload, int numberOfMessagesToSend, long sleepBetweenMessageInMilliseconds)
            : payload(payload),
            numberOfMessagesToSend(numberOfMessagesToSend),
            sleepBetweenMessageInMilliseconds(sleepBetweenMessageInMilliseconds)
    {
    }
};

class MessageRecorderTest : public ::testing::Test
{
protected:
    bool keepRecordedFilesAtTearDown;
    const MockRecorderConfigProvider* recorderConfigMock;
    MessageRecorder* messageRecorder;
    DummyReceiver* messageReceiver;

    MessageRecorderTest()
    {
        // Setup recorderConfig
        keepRecordedFilesAtTearDown = false;
        recorderConfigMock = new MockRecorderConfigProvider();
        EXPECT_CALL(
            *recorderConfigMock, getRecorderFilePath())
        .Times(AnyNumber())
        .WillRepeatedly(Return("C:\\work\\recordedMessagesTest.bin"));
        EXPECT_CALL(*recorderConfigMock, isRecorderOn()).WillRepeatedly(Return(true));
        EXPECT_CALL(*recorderConfigMock, keepRecordedFile()).WillRepeatedly(Return(true));
        messageReceiver = new DummyReceiver();
    }

    virtual ~MessageRecorderTest()
    {
        // Clean up. Don't change except you know what you are doing!
        // Standard false to clean up all created files
        EXPECT_CALL(*recorderConfigMock, keepRecordedFile()).WillRepeatedly(Return(keepRecordedFilesAtTearDown));
        delete messageRecorder;
        delete recorderConfigMock;
    }

    void createMessageRecorder()
    {
        messageRecorder = new MessageRecorder();
    }

    void deleteRecordedFile()
    {
        //messageRecorder->deleteRecordedFiles();
    }

    void saveMessageToFile(PluginInterface::ProtocolMessage* dummyProtocolMessage)
    {
        messageRecorder->sendMessage(dummyProtocolMessage);
    }

    void retrieveMessageFromFile()
    {
        messageRecorder->prepareToSendRecordedMessages();
    }

    Poco::File* getFirstMessageRecorderFile()
    {
        return messageRecorder->messageRecoderFiles.at(0);
    }

    TestData* createTestData(std::string pattern, unsigned int repeatPattern, unsigned int numberOfMessagesSend, long sleepBetweenMessageInMilliseconds)
    {
        std::string payload;
        for(; repeatPattern > 0; repeatPattern--)
            payload.append(pattern);
        TestData* data = new TestData(payload, numberOfMessagesSend, sleepBetweenMessageInMilliseconds);
        return data;
    }

    PluginInterface::ProtocolMessage* createTestMessage(Protocol::CommonDefinitions::MessageType type,int payloadLength, unsigned char* payload, bool urgent, unsigned long long timestamp)
    {
        if(timestamp == 0)
            std::cout << "Warning timestamp = 0 -> message takes actual time. Test will fail!";
        return new PluginInterface::ProtocolMessage(type, payloadLength, payload, urgent, timestamp);
    }

    int find(std::string pattern, std::vector<TestData*> patternList)
    {
        for(int position = 0; position < patternList.size(); position++)
        {
            TestData* data = patternList.at(position);
            if(data->payload.compare(pattern) == 0)
                return position;
        }
        return -1;
    }
};

TEST_F(MessageRecorderTest, CreateMessageRecorder)
{
    MessageRecorderTest::createMessageRecorder();

    ASSERT_TRUE(MessageRecorderTest::getFirstMessageRecorderFile()->exists());
}

TEST_F(MessageRecorderTest, deleteRecorderFile)
{
    MessageRecorderTest::createMessageRecorder();
    MessageRecorderTest::deleteRecordedFile();

    if(recorderConfigMock->keepRecordedFile())
        ASSERT_TRUE(MessageRecorderTest::getFirstMessageRecorderFile()->exists());
    else
        ASSERT_FALSE(MessageRecorderTest::getFirstMessageRecorderFile()->exists());
}

//TEST_F(MessageRecorderTest, SaveMessageToFile)
//{
// unsigned char dummyPayload[256];
// for (int i = 0; i < 256; i++) {
//  dummyPayload[i] = i;
// }
// Poco::Timestamp ts;
// PluginInterface::ProtocolMessage* dummyProtocolMessage = MessageRecorderTest::createTestMessage(Protocol::CommonDefinitions::MSG_TYPE_3G_PLUS_JVMTI_MONITOR,
//                         sizeof(dummyPayload),
//                         dummyPayload,
//                         false,
//                         (unsigned long long)(ts.epochMicroseconds()/1000));
// MessageRecorderTest::createMessageRecorder();
// Poco::UInt64 fileSizeBeforeMessageWriten = MessageRecorderTest::getFirstMessageRecorderFile()->getSize();

// MessageRecorderTest::saveMessageToFile(dummyProtocolMessage);
// ASSERT_LT(fileSizeBeforeMessageWriten, MessageRecorderTest::getFirstMessageRecorderFile()->getSize());
//}

//TEST_F(MessageRecorderTest, RetrieveMessageFromFile) {
// unsigned char dummyPayload[256];
// for (int i = 0; i < 256; i++) {
//  dummyPayload[i] = i;
// }
// Poco::Timestamp ts;
// PluginInterface::ProtocolMessage* dummyProtocolMessage = MessageRecorderTest::createTestMessage(Protocol::CommonDefinitions::MSG_TYPE_3G_PLUS_JVMTI_MONITOR,
//                         sizeof(dummyPayload),
//                         dummyPayload,
//                         false,
//                         (unsigned long long)(ts.epochMicroseconds()/1000));
// PluginInterface::ProtocolMessage* dummyProtocolMessageCopy = new PluginInterface::ProtocolMessage(*dummyProtocolMessage);
// MessageRecorderTest::createMessageRecorder();
// MessageRecorderTest::saveMessageToFile(dummyProtocolMessage);

// MessageRecorderTest::retrieveMessageFromFile();

// const PluginInterface::ProtocolMessage* retreivedMessage = NULL;
// while((retreivedMessage = messageReceiver->getNextMessage()) != NULL)
// {
//  ASSERT_EQ(dummyProtocolMessageCopy->getMessageType(), retreivedMessage->getMessageType());
//  ASSERT_EQ(dummyProtocolMessageCopy->getMessageLength(), retreivedMessage->getMessageLength());
//  // Compare payload with dummyPayload character by character
//  for (unsigned int position = 0; position < retreivedMessage->getMessageLength(); position++)
//  {
//   ASSERT_EQ(dummyPayload[position], *(retreivedMessage->getPayloadData() + position));
//  }
//  ASSERT_EQ(dummyProtocolMessageCopy->getTimestamp(), retreivedMessage->getTimestamp());
//  ASSERT_EQ(dummyProtocolMessageCopy->isValid(), retreivedMessage->isValid());
//  ASSERT_EQ(dummyProtocolMessageCopy->isUrgent(), retreivedMessage->isUrgent());
//  delete retreivedMessage;
// }

// delete dummyProtocolMessageCopy;
// delete messageReceiver;
//}

}
; // Tests
}
; // MessageRecorder
}
; // TargetAgent

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
