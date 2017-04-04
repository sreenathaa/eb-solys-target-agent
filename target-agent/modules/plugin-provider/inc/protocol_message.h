/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#ifndef PROTOCOL_MESSAGE_H_
#define PROTOCOL_MESSAGE_H_

#include <target_agent_prot_common_definitions.pb.h>

namespace TargetAgent
{
namespace PluginInterface
{

class ProtocolMessage
{
public:
    ProtocolMessage(
        Protocol::CommonDefinitions::MessageType type,
        int payloadLength,
        unsigned char* payloadBuffer,
        bool urgent = false,
        unsigned long long timestamp = 0,
        const std::map<std::string, std::string>* msgContextInfo = 0);

    ProtocolMessage(const ProtocolMessage& msg);
    virtual ~ProtocolMessage();
    ProtocolMessage& operator= (const ProtocolMessage& rhs);

    Protocol::CommonDefinitions::MessageType getMessageType() const;
    std::map<std::string,std::string> getContextInfo(void) const;
    int       getMessageLength() const;
    const unsigned char*  getPayloadData() const;
    bool       isValid() const;
    bool      isUrgent() const;
    unsigned long long   getTimestamp() const;
    const unsigned int& getVersionToken(void) const;

private:
    bool setupMessageBuffer(int payloadLength, void* payloadBuffer);
    void setTimestamp(unsigned long long timestamp);

private:
    Protocol::CommonDefinitions::MessageType mType;
    int mPayloadLength;
    unsigned char* mPayload;
    bool mIsValid;
    bool mIsUrgent;
    unsigned long long mTimestampInMS;
    const unsigned int mVersionToken;
    std::map<std::string, std::string> mMmsgContextInfo;
};

};
};
#endif /* PROTOCOL_MESSAGE_H_ */
