/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include <sstream>
#include <ctime>

#include "Poco/BinaryWriter.h"
#include "Poco/Format.h"
#include "Poco/Timestamp.h"
#include "Poco/TemporaryFile.h"

#include "../inc/MessageRecorder.hpp"
#include "../inc/PathGenerator.hpp"
#include "Serializer.hpp"
#include "CMediator.h"
#include "CLogger.hpp"

using Poco::Timestamp;
namespace TargetAgent {
namespace MessageRecorder {

CMessageRecorder::CMessageRecorder() :
        mPathHandler(new CPathHandler()), mSerializer(new CSerializer()), mPartNumber(
            0), mFileHandles(), lastPosition(
                std::make_pair(0, mFileHandles.begin())), /*mCurrentPosition(
                                                             0), mCurrentIterator(mFileHandles.end()),*/mSender(
            Mediator::CTargetAgentRuntime::getUniqueInstance()->getComController()), recorderConfig(
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()) {
    //mCrtMode = (recorderConfig->isRecorderOn()) ? RECORDING : BUFFERING;

}

void CMessageRecorder::recordMessage(
    const PluginInterface::ProtocolMessage* msg, bool doCleanup) {
    mMessageMutex.lock();
    try
    {
        if (!mFileHandles.back()->canWrite() || !mFileHandles.back()->exists())
        {
            Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error(
                "the file cannot be written");
        }
        else
        {
            bool limitReached = false;
            mSerializer->serializeMessage(msg, limitReached);
            if (limitReached)
            {
                stopRecording();
                startRecording();
                mSerializer->serializeMessage(msg, limitReached);
            }
        }
    }
    catch (std::ios_base::failure& exc)
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error(
            Poco::format("exception writing file %s", exc.what()));
    }
    catch (Poco::Exception &e)
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getCommLogger().log(
            e);
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error(
            "exception writing file");
    }
    if (doCleanup)
    {
        delete msg;
        msg = NULL;
    }
    mMessageMutex.unlock();
}

void CMessageRecorder::sendMessagesInOneFile(Poco::BinaryReader& fileReader) {
    while (fileReader.good())
    {
        PluginInterface::ProtocolMessage* msg;
        if (mSerializer->deserialzeMessage(fileReader, &msg))
        {
            mSender->sendMessage(msg);
        }
    }
}
void CMessageRecorder::sendBufferedData(void) {
    std::vector<Poco::File*>::iterator start = mFileHandles.begin();

    if (0 != lastPosition.first)
    {
        start = lastPosition.second;
    }
    Timestamp now;
    for (std::vector<Poco::File*>::iterator it = start;
         it != mFileHandles.end(); ++it)
    {
        std::ifstream fileReader((*it)->path().c_str(),
                                 std::ios::in | std::ios::binary);
        Poco::BinaryReader reader(fileReader);

        if (lastPosition.first && (it == lastPosition.second))
        {
            fileReader.seekg(lastPosition.first);
        }

        sendMessagesInOneFile(reader);
        fileReader.close();
    }

    Timestamp::TimeDiff diff = now.elapsed();

    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRecorderLogger().error(
        Poco::format("It Took %Ld [ms] to send the buffered data",
                     (diff / 1000)));
    Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRecorderLogger().error(
        Poco::format("Send File%s", mFileHandles.back()->path()));

}

void CMessageRecorder::startRecording(void) {
    mMessageMutex.lock();
    mFileHandles.push_back(fileFactory());
    resumeRecording();
    mMessageMutex.unlock();
}

void CMessageRecorder::resumeRecording(void) {
    mMessageMutex.lock();
    mSerializer->openStream(mFileHandles.back()->path().c_str());
    mMessageMutex.unlock();
}

void CMessageRecorder::stopRecording(void) {
    mMessageMutex.lock();
    pauseRecording();
    mSerializer->resetCounters();
    mMessageMutex.unlock();
}

void CMessageRecorder::pauseRecording(void) {
    mMessageMutex.lock();
    mSerializer->closeStream();
    mMessageMutex.unlock();
}

CMessageRecorder::~CMessageRecorder() {
    for (std::vector<Poco::File*>::iterator it = mFileHandles.begin();
         it != mFileHandles.end(); ++it)
    {
        (*it)->remove(false);
    }
    mFileHandles.clear();
    delete mPathHandler;
    delete mSerializer;
}

}
;
}
;
