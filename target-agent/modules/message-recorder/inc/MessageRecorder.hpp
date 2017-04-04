/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/


#ifndef MESSAGE_RECORDER_H_
#define MESSAGE_RECORDER_H_

#include <fstream>
#include <iostream>
#include <vector>
#include <utility>

#include "Poco/File.h"
#include "Poco/Event.h"
#include "Poco/Thread.h"
#include "Poco/Activity.h"
#include "Poco/LocalDateTime.h"
#include "Poco/BinaryReader.h"

#include "config_provider.h"
#include "CMessageDispatcher.h"
#include "plugin_provider.h"


namespace TargetAgent {
namespace MessageRecorder {

class CPathHandler;
class CSerializer;

class CMessageRecorder {

public:
    CMessageRecorder(void);
    virtual ~CMessageRecorder(void);
    virtual void recordMessage(const PluginInterface::ProtocolMessage* msg, bool doCleanup=true);
    virtual void sendBufferedData(void);
    virtual void resumeBuffering(void)=0;
    virtual void startRecording(void);
    virtual void stopRecording(void);

protected:
    CPathHandler* mPathHandler;
    CSerializer* mSerializer;
    unsigned int mPartNumber;
    std::vector<Poco::File*> mFileHandles;
    std::pair<std::streampos,std::vector<Poco::File*>::iterator> lastPosition;
    Poco::Mutex mMessageMutex;
    Communicator::ComController* mSender;
    const Configuration::ConfigProvider* recorderConfig;

public:
    virtual void resumeRecording(void);
    virtual void pauseRecording(void);
    virtual void sendMessagesInOneFile(Poco::BinaryReader& fileReader);
    virtual Poco::File* fileFactory(void) = 0;
};
}
;
}
;

#endif /* MESSAGE_RECORDER_H_ */
