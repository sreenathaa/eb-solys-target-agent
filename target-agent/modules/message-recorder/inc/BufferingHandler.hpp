/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/


#ifndef SOURCE_DIRECTORY__TARGET_AGENT_MODULES_MESSAGE_RECORDER_INC_BUFFERINGHANDLER_HPP_
#define SOURCE_DIRECTORY__TARGET_AGENT_MODULES_MESSAGE_RECORDER_INC_BUFFERINGHANDLER_HPP_


#include "MessageRecorder.hpp"

namespace TargetAgent {
namespace MessageRecorder {

class CBufferingHandler : public CMessageRecorder{
public:
    CBufferingHandler() {
        startRecording();
    }
    void resumeBuffering(void);
    void stopRecording(void);
    void sendBufferedData(void);
    CBufferingHandler(const CBufferingHandler& other) {
    }
    CBufferingHandler& operator=(const CBufferingHandler& other);
    ~CBufferingHandler() {
    }
public:
    virtual Poco::File* fileFactory(void);
};

}
}


#endif /* SOURCE_DIRECTORY__TARGET_AGENT_MODULES_MESSAGE_RECORDER_INC_BUFFERINGHANDLER_HPP_ */
