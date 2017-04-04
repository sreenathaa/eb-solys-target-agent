/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "Poco/TemporaryFile.h"
#include "Poco/Timestamp.h"
#include "CLogger.hpp"
#include "CMediator.h"
#include "PathGenerator.hpp"
#include "Serializer.hpp"
#include "BufferingHandler.hpp"

namespace TargetAgent {
namespace MessageRecorder {


void CBufferingHandler::resumeBuffering(void) {
    startRecording();
}

Poco::File* CBufferingHandler::fileFactory(void) {
    Poco::File* retFile = NULL;

    retFile = new Poco::TemporaryFile();
    Poco::TemporaryFile::registerForDeletion(retFile->path());
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRecorderLogger().error(
        Poco::format("create temporary file %s", retFile->path()));

    return retFile;
}

void CBufferingHandler::stopRecording(void) {
    mMessageMutex.lock();
    CMessageRecorder::stopRecording();
    mFileHandles.back()->remove(false);
    mMessageMutex.unlock();
}
void CBufferingHandler::sendBufferedData(void) {

    mMessageMutex.lock();

    CMessageRecorder::stopRecording();

    CMessageRecorder::sendBufferedData();

    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRecorderLogger().warning(
        Poco::format("Delete Buffer File %s", mFileHandles.back()->path()));
    mFileHandles.back()->remove(false);
    mFileHandles.erase(mFileHandles.end() - 1);

    mMessageMutex.unlock();
}
}
}
