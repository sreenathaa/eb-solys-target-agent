/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "Poco/Timestamp.h"
#include "CLogger.hpp"
#include "CMediator.h"
#include "PathGenerator.hpp"
#include "Serializer.hpp"
#include "OfflineRecorder.hpp"

namespace TargetAgent {
namespace MessageRecorder {

void COfflineRecorder::resumeBuffering(void) {
    lastPosition = std::make_pair(mSerializer->getCurentPosition(),
                                  mFileHandles.end() - 1);
}

Poco::File* COfflineRecorder::fileFactory(void) {
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRecorderLogger().error("create new file");
    return new Poco::File(mPathHandler->createPath(mPartNumber));
}

void COfflineRecorder::sendBufferedData(void) {

    mMessageMutex.lock();

    pauseRecording();

    CMessageRecorder::sendBufferedData();

    resumeRecording();

    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRecorderLogger().error(
        Poco::format("keep recorded file due to offline mode %s",
                     mFileHandles.at(mPartNumber - 1)->path()));

    mMessageMutex.unlock();
}

}
}
