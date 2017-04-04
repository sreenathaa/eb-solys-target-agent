/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef SOURCE_DIRECTORY__TARGET_AGENT_MODULES_MESSAGE_RECORDER_INC_MARSHALINGENGINE_HPP_
#define SOURCE_DIRECTORY__TARGET_AGENT_MODULES_MESSAGE_RECORDER_INC_MARSHALINGENGINE_HPP_

#include "target_agent_prot_frame.pb.h"
#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/BinaryReader.h"
#include "CMessageDispatcher.h"
#include "plugin_provider.h"
#include <fstream>

namespace TargetAgent {
namespace MessageRecorder {

class CSerializer {
public:
    CSerializer();
    CSerializer(const CSerializer& other);
    void openStream(const char* file);

    void closeStream(void);

    void resetCounters(void);

    bool isHeaderSizeValid(void) const {
        return (mHeader.ByteSize() < MAX_HEADER_SIZE);
    }
    bool isFileSizeLimitReached(void) const;

    void adjustMessageBuffer(Poco::UInt32 expectedSize,
                             unsigned int* actualSize, unsigned char** buffer);
    bool deserialzeMessage(Poco::BinaryReader& reader,PluginInterface::ProtocolMessage** msg);
    std::streampos getCurentPosition(){
        return mFileStream.tellp();
    }
    bool serializeMessage(const PluginInterface::ProtocolMessage* msg, bool& limitReached);
    void flushMessage(const PluginInterface::ProtocolMessage* msg);
    CSerializer& operator=(const CSerializer& other);
    ~CSerializer();
private:
    void writeMessageHeader(const PluginInterface::ProtocolMessage* msg);
private:
    std::ofstream mFileStream;
    static const int MAX_HEADER_SIZE = 0xFF;
    static char mHeaderBuffer[MAX_HEADER_SIZE + 1];
    unsigned int mCurrentPayloadSize;
    unsigned char *mPayloadBuffer;
    Protocol::Frame::Header mHeader;
    unsigned long long int mCrtAmountOfBytes;
};

}
;
}
;

#endif /* SOURCE_DIRECTORY__TARGET_AGENT_MODULES_MESSAGE_RECORDER_INC_MARSHALINGENGINE_HPP_ */
