/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#include "../inc/PathGenerator.hpp"

#include "CLogger.hpp"
#include "CMediator.h"
#include "config_provider.h"
#include "CMessageDispatcher.h"
#include "plugin_provider.h"
#include "CLogger.hpp"

using Poco::Timestamp;
namespace TargetAgent {
namespace MessageRecorder {

CPathHandler::CPathHandler(void) :
        mStartTime(), mIsSplitingActive(
            Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->getFileSizeLimit()
    > 0) {
}

std::string addSeparatorIfNeeded(std::string targetDirPath)
{
    if (targetDirPath.length() > 0 && *targetDirPath.rbegin() != Poco::Path::separator())
    {
        targetDirPath += Poco::Path::separator();
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().information(Poco::format("Path Separator added %s", targetDirPath));
    }
    return targetDirPath;
}

std::string CPathHandler::createPath(unsigned int& mPartNumber) {
    const char separator = '-';
    std::stringstream fileName;
    std::string targetDirPath = Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->getTargetDirPath();

    targetDirPath = addSeparatorIfNeeded(targetDirPath);

    Poco::Path path(
        targetDirPath);

    if (path.tryParse(targetDirPath) && !createIntermediateDirs(path))
    {
        path.clear();
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().information(
            "Use the current directory");
    }

    fileName
    << Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->getRecorderFilePrefix()
    << "-";

    fileName
    << Poco::format("%02d.%02d.%04d", mStartTime.day(),
                    mStartTime.month(), mStartTime.year());
    fileName
    << Poco::format("%c%02d.%02d.%02d.%02d", separator, mStartTime.hour(),
                    mStartTime.minute(), mStartTime.second(),mStartTime.millisecond());

    if(mIsSplitingActive)
        fileName
        << Poco::format("%cpart%u",
                        separator, mPartNumber);
    path.setBaseName(fileName.str() + ".bin");
    mPartNumber++;
    return path.toString();
}

bool CPathHandler::createIntermediateDirs(Poco::Path& outputPath) {
    bool retVal = true;

    try
    {
        if (outputPath.depth() > 0)
        {
            /* contains a path other than the current folder*/
            Poco::File *dirAsFile = new Poco::File(outputPath);

            if (dirAsFile->exists())
            {
                Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().information(
                    Poco::format("Output path already exists %s",
                                 outputPath.toString()));
            }
            else if (Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->doCreateOutputPath())
            {
                dirAsFile->createDirectories();
                Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().information(
                    Poco::format(
                        "create not existing directory structure %s",
                        outputPath.toString()));
            }
            else
            {
                Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error(
                    Poco::format(
                        "Target Directory [%s] does not exist and create output path is not enabled",
                        outputPath.toString()));
                retVal = false;
            }
        }
        else
        {
            Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().information(
                Poco::format("Use current folder %s",
                             outputPath.toString()));
        }
    }
    catch (Poco::Exception& e)
    {
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().log(
            e);
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getRuntimeLogger().error(
            Poco::format("%s %s %d", std::string(e.what()),
                         std::string(__FILE__), __LINE__));

        retVal = false;

    }

    return retVal;

}

}
}
