/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include <string>

#include <Poco/Timestamp.h>

#include <protocol_message.h>
#include <target_agent_version.hpp>
#include "CMediator.h"


namespace TargetAgent{
namespace PluginInterface
{

ProtocolMessage::ProtocolMessage(
    Protocol::CommonDefinitions::MessageType type,
    int payloadLength,
    unsigned char* payloadBuffer,
    bool urgent,
    unsigned long long timestamp,
    const std::map<std::string, std::string>* msgContextInfo)
        : mType(type),
        mPayloadLength(payloadLength),
        mPayload(NULL),
        mIsValid(false),
        mIsUrgent(urgent),
        mVersionToken(TargetAgent::Version::raceTaProtocolVersion),
        mMmsgContextInfo()
{
    if(0 != msgContextInfo)
    {
        this->mMmsgContextInfo = *msgContextInfo;
    }

    if(setupMessageBuffer(payloadLength, (void*)payloadBuffer))
    {
        mIsValid = true;
    }
    setTimestamp(timestamp);
}



bool ProtocolMessage::setupMessageBuffer(int payloadLength, void* payloadBuffer)
{
    if(payloadLength > 0)
    {
        mPayload = new (std::nothrow) unsigned char[payloadLength];
        if(mPayload != NULL)
        {
            memcpy ((void*)mPayload, (void*)payloadBuffer, payloadLength);
            return true;
        }
    }
    else
    {
        mPayload = NULL;
        return true;
    }
    return false;
}


void ProtocolMessage::setTimestamp(unsigned long long timestamp)
{
    if(timestamp == 0)
    {
        mTimestampInMS = Mediator::CTargetAgentRuntime::getUniqueInstance()->getTsProvider()->createTimestamp();
    }
    else
    {
        mTimestampInMS = timestamp;
    }
}


ProtocolMessage& ProtocolMessage::operator= (const ProtocolMessage& rhs)
{
    if(this == &rhs)
        return *this;

    this->mType  = rhs.mType;
    this->mPayloadLength  = rhs.mPayloadLength;
    this->mIsValid = false;
    this->mMmsgContextInfo = mMmsgContextInfo;

    if(mPayloadLength > 0)
    {
        mPayload = new (std::nothrow) unsigned char[mPayloadLength];
        if(mPayload != NULL)
        {
            memcpy ((void*)mPayload, (void*)rhs.mPayload, mPayloadLength);
            mIsValid=true;
        }
    }
    else
    {
        mPayload = NULL;
        mIsValid = true;
    }

    return *this;
}


ProtocolMessage::ProtocolMessage(const ProtocolMessage& msg)
        : mType(msg.mType),
        mPayloadLength(msg.mPayloadLength),
        mPayload(NULL),
        mIsValid(false),mVersionToken(TargetAgent::Version::raceTaProtocolVersion),mMmsgContextInfo(msg.getContextInfo())
{

    this->mIsUrgent = msg.isUrgent();
    this->mTimestampInMS = msg.getTimestamp();

    if(mPayloadLength > 0)
    {
        mPayload = new (std::nothrow) unsigned char[mPayloadLength];
        if(mPayload != NULL)
        {
            memcpy ((void*)mPayload, (void*)msg.mPayload, mPayloadLength);
            mIsValid=true;
        }
    }
    else
    {
        mPayload = NULL;
        mIsValid = true;
    }
}


ProtocolMessage::~ProtocolMessage()
{
    if(mPayload != NULL)
    {
        delete[] mPayload;
    }
}


bool ProtocolMessage::isValid() const
{
    return mIsValid;
}


bool ProtocolMessage::isUrgent() const
{
    return mIsUrgent;
}


unsigned long long ProtocolMessage::getTimestamp() const
{
    return mTimestampInMS;
}

const unsigned int& ProtocolMessage::getVersionToken(void) const
{
    return mVersionToken;
}

std::map<std::string,std::string> ProtocolMessage::getContextInfo(void) const {
    return mMmsgContextInfo;
};

int ProtocolMessage::getMessageLength() const
{
    if(mIsValid)
    {
        return mPayloadLength;
    }

    return 0;
}


Protocol::CommonDefinitions::MessageType ProtocolMessage::getMessageType() const
{
    if(mIsValid)
    {
        return mType;
    }

    return Protocol::CommonDefinitions::MSG_TYPE_INVALID;
}


const unsigned char* ProtocolMessage::getPayloadData() const
{
    if(mIsValid)
    {
        return mPayload;
    }

    return NULL;
}

};
};

