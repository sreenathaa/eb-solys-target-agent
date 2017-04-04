/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#include "Serializer.hpp"
#include "CLogger.hpp"
#include "CMediator.h"

using Poco::Timestamp;
namespace TargetAgent
{
namespace MessageRecorder
{

char CSerializer::mHeaderBuffer[] = { 0 };

CSerializer::CSerializer() :
        mCurrentPayloadSize(1), mPayloadBuffer(
            new unsigned char[mCurrentPayloadSize]), mHeader(), mCrtAmountOfBytes(
                0)
{
}

void CSerializer::openStream(const char* file)
{
    mFileStream.open(file, std::ios::out | std::ios::binary | std::ios::app);

    mFileStream.exceptions(std::ios_base::badbit | std::ios_base::failbit);
}

void CSerializer::closeStream(void)
{
    mFileStream.flush();
    mFileStream.close();
}

void CSerializer::resetCounters(void)
{
    mCrtAmountOfBytes = 0;
}

void CSerializer::writeMessageHeader(const PluginInterface::ProtocolMessage* msg)
{
    mHeader.Clear();
    mHeader.set_type(msg->getMessageType());
    mHeader.set_timestamp(msg->getTimestamp());
    mHeader.set_length(msg->getMessageLength());
    std::map<std::string,std::string> contextMap = msg->getContextInfo();

    for(std::map<std::string,std::string>::iterator it=contextMap.begin(); it!=contextMap.end(); ++it)
    {
        TargetAgent::Protocol::Frame::MetaData* metadata = mHeader.add_metadatainfo();
        metadata->set_key(it->first);
        metadata->set_value(it->second);
    }
    mHeader.set_versiontoken(msg->getVersionToken());
}

bool CSerializer::serializeMessage(
    const PluginInterface::ProtocolMessage* msg, bool& limitReached)
{
    bool retVal = false;
    if (mFileStream.is_open())
    {
        writeMessageHeader(msg);

        mHeader.SerializeToArray((void*) mHeaderBuffer, mHeader.ByteSize());

        if (isHeaderSizeValid())
        {
            mCrtAmountOfBytes += mHeader.ByteSize() + 1 + mHeader.length();
            if (isFileSizeLimitReached())
            {
                limitReached = true;
                Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error(
                    Poco::format(
                        "Limit of %u MB was reached: create new file",
                        Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->getFileSizeLimit()));
                resetCounters();
            }
            else
            {
                flushMessage(msg);
                retVal = true;
            }
        }
        else
            Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error(
                "Header size exceeds maximum size %d",mHeader.ByteSize());
    }
    else
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().information(
            "File writer closed");
    return retVal;
}

bool CSerializer::isFileSizeLimitReached(void) const
{
    return (Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->isRecorderOn()
            && Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->getFileSizeLimit()
            && (mCrtAmountOfBytes
                >= Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->getFileSizeLimit()
                * 1024 * 1024));
}

void CSerializer::adjustMessageBuffer(Poco::UInt32 expectedSize,
                                      unsigned int* actualSize, unsigned char** buffer)
{
    if (expectedSize > *actualSize)
    {
        delete[] (*buffer);
        *buffer = new unsigned char[expectedSize];
        *actualSize = expectedSize;
    }
}

void CSerializer::flushMessage(
    const PluginInterface::ProtocolMessage* msg)
{
    mFileStream << (unsigned char) mHeader.ByteSize();
    mFileStream.write(mHeaderBuffer, mHeader.ByteSize());
    mFileStream.write((char*) msg->getPayloadData(), mHeader.length());
    mFileStream.flush();
}

CSerializer::~CSerializer()
{
    delete[] mPayloadBuffer;

}

bool CSerializer::deserialzeMessage(Poco::BinaryReader& fileReader,
                                    PluginInterface::ProtocolMessage** msg)
{
    unsigned char headerLength = 0;
    fileReader >> headerLength;

    if (headerLength > 0)
    {
        fileReader.readRaw((char*) mHeaderBuffer, headerLength);
        if (mHeader.ParseFromArray((void*) (mHeaderBuffer), headerLength))
        {
            adjustMessageBuffer(mHeader.length(), &mCurrentPayloadSize,
                                &mPayloadBuffer);
            fileReader.readRaw((char*) mPayloadBuffer, mHeader.length());
            *msg = new PluginInterface::ProtocolMessage(mHeader.type(),
                    mHeader.length(), mPayloadBuffer, false,
                    mHeader.timestamp());
            return true;
        }
    }
    return false;
}
}
}
