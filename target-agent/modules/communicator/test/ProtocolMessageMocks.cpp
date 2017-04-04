/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#include "gmock/gmock.h"
#include "Poco/Logger.h"

#include "protocol_message.h"

#include <string>

#include <Poco/Timestamp.h>

#include <protocol_message.h>
#include <target_agent_version.hpp>
#include "CMediator.h"

namespace TargetAgent {
namespace PluginInterface {

ProtocolMessage::ProtocolMessage(Protocol::CommonDefinitions::MessageType type,
                                 int payloadLength,
                                 unsigned char* payloadBuffer,
                                 bool urgent,
                                 unsigned long long timestamp,
                                 const std::map<std::string, std::string>* msgContextInfo) :
mType(type), mPayloadLength(payloadLength), mPayload(NULL), mIsUrgent(false),mIsValid(false), mTimestampInMS(),mVersionToken(TargetAgent::Version::raceTaProtocolVersion) {

}


bool ProtocolMessage::setupMessageBuffer(int payloadLength,
        void* payloadBuffer) {

    return false;
}

void ProtocolMessage::setTimestamp(unsigned long long timestamp) {

}

ProtocolMessage& ProtocolMessage::operator=(const ProtocolMessage& rhs) {
    return *this;
}

ProtocolMessage::ProtocolMessage(const ProtocolMessage& msg) :
        mType(msg.mType), mPayloadLength(msg.mPayloadLength), mPayload(NULL),mIsUrgent(false), mIsValid(
    false), mTimestampInMS(),mVersionToken(TargetAgent::Version::raceTaProtocolVersion),mMmsgContextInfo() {

}

ProtocolMessage::~ProtocolMessage() {

}

bool ProtocolMessage::isValid() const {
    return mIsValid;
}

bool ProtocolMessage::isUrgent() const {
    return mIsUrgent;
}

unsigned long long ProtocolMessage::getTimestamp() const {
}

int ProtocolMessage::getMessageLength() const {
    return 0;
}

Protocol::CommonDefinitions::MessageType ProtocolMessage::getMessageType() const {
    return Protocol::CommonDefinitions::MSG_TYPE_INVALID;
}

const unsigned char* ProtocolMessage::getPayloadData() const {
    return NULL;
}

}
;
}
;

