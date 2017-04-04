/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "Poco/Process.h"
#include "Poco/PipeStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/StringTokenizer.h"
#include "Poco/String.h"
#include "Poco/NumberParser.h"
#include "Poco/NumberFormatter.h"


#include "dbus_data_pool.h"

using Poco::Process;
using Poco::ProcessHandle;
using Poco::StringTokenizer;
using Poco::cat;

namespace TargetAgentDbusMonitor {

unsigned int CDataPool::getDbusDeamonVersion(void) {
    /*first check the dbus-deamon version*/
    unsigned int retVal = 6;
    unsigned localVersion = 0;
    std::string cmd("dbus-daemon");
    std::vector<std::string> args;
    args.push_back("--version");
    Poco::Pipe outPipe;
    ProcessHandle ph = Process::launch(cmd, args, 0, &outPipe, 0);

    Poco::PipeInputStream istr(outPipe);
    char array[250];

    istr.getline(array, sizeof(array));
    std::string lineString(array);

    mLogger->information(lineString);

    Poco::StringTokenizer tokens(lineString, " ",
                                 StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);

    if (tokens.count() > 1)
    {
        std::string completeVersionInfo(tokens[tokens.count() - 1]);

        Poco::StringTokenizer verTokens(completeVersionInfo, ".",
                                        StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
        const std::string testStr (verTokens[verTokens.count() - 2]);
        if (3 == verTokens.count())
        {

            Poco::NumberParser::parseUnsigned(testStr);

        }
        else
        {
            mLogger->error("Invalid number of tokens");
        }

    }
    else
    {
        mLogger->error("Failed to parse version information");
    }

    if (4 == localVersion)
    {
        retVal = localVersion;
    }

    mLogger->warning(Poco::format("Select DBUS minor version %u", retVal));
    return retVal;
}

void CDataPool::closeDbusConnection(void) {
    if (mDBusconnection != NULL)
    {
        dbus_connection_close(mDBusconnection);
        dbus_connection_unref(mDBusconnection);
        mDBusconnection = NULL;
    }
}

void CDataPool::waitForMessagesAvailable() {
    Poco::Mutex::ScopedLock lock(mMsgQMutex);
    {
        while (!mDbusMessageAvailable)
            mMsgQCondition.wait(mMsgQMutex);
    }
}

void CDataPool::pushMessageToQueue(DbusMessageEntry* msg) {

    Poco::Mutex::ScopedLock lock(mMsgQMutex);
    {

        mDbusQueue.push_back(msg);
    }
}

void CDataPool::updateMessageAvailableFlag() {
    Poco::Mutex::ScopedLock lock(mMsgQMutex);
    {
        if (mDbusQueue.size() == 0)
        {
            mDbusMessageAvailable = false;
        }
    }
}

DbusMessageEntry* CDataPool::waitForNextMessageToSend() {
    waitForMessagesAvailable();
    return popMessageFromQueue();
}
DbusMessageEntry* CDataPool::popMessageFromQueue() {
    DbusMessageEntry* msg = NULL;
    Poco::Mutex::ScopedLock lock(mMsgQMutex);
    {
        if (mDbusQueue.size() > 0)
        {
            msg = mDbusQueue.front();
            mDbusQueue.pop_front();
        }
    }
    return msg;
}
void CDataPool::notifyMessagesAvailable() {
    Poco::Mutex::ScopedLock lock(mMsgQMutex);
    {
        if (!mDbusMessageAvailable)
        {
            mDbusMessageAvailable = true;
            mMsgQCondition.signal();
            Poco::Thread::yield(); /* yield consumer-thread to speed up sending. */
        }
    }
}

}
