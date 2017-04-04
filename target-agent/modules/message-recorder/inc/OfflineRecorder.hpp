/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef SOURCE_DIRECTORY__TARGET_AGENT_MODULES_MESSAGE_RECORDER_INC_OFFLINERECORDER_CPP_
#define SOURCE_DIRECTORY__TARGET_AGENT_MODULES_MESSAGE_RECORDER_INC_OFFLINERECORDER_CPP_

#include "MessageRecorder.hpp"

namespace TargetAgent {
namespace MessageRecorder {

class COfflineRecorder : public CMessageRecorder{
public:
    COfflineRecorder() {
        startRecording();
    }
    void resumeBuffering(void);
    void sendBufferedData(void);

    COfflineRecorder(const COfflineRecorder& other) {
    }
    COfflineRecorder& operator=(const COfflineRecorder& other);
    ~COfflineRecorder() {
    }
public:
    virtual Poco::File* fileFactory(void);
};

}
}

#endif /* SOURCE_DIRECTORY__TARGET_AGENT_MODULES_MESSAGE_RECORDER_INC_OFFLINERECORDER_CPP_ */
