/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifdef __linux__
#include <time.h>
#include <unistd.h>
#elif _WIN32
// windows code goes here
#endif

#include "Poco/Timestamp.h"

#include "CTimestampProvider.h"
#include "CMediator.h"
#include "CLogger.hpp"


namespace TargetAgent {
namespace PluginManagement {

unsigned long long CUpTimeTsProvider::createTimestamp(void) const {
    unsigned long long tsInMs = 0;


#if defined(__linux__) || defined(__QNX__)

    struct timespec ts = {
                             0 };
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
    {
        tsInMs = (((uint32_t) ts.tv_sec * 1000
                   + (uint32_t) ts.tv_nsec / 1000000LL));
    }
#elif _WIN32
    tsInMs = GetTickCount();
#endif

    return tsInMs;
}

unsigned long long CAbsoluteTimeTsProvider::createTimestamp(void) const {
    Poco::Timestamp ts;
    return (unsigned long long) (ts.epochMicroseconds() / 1000);
}

PluginInterface::CTimestampProvider* TimestampProviderFactory::createTimestamp(
    ETimeRef timeUnit) {
    PluginInterface::CTimestampProvider * ptr = 0;
    switch (timeUnit)
    {
    case eTimeRefUpTime:
        ptr = new CUpTimeTsProvider();
        break;
    case eTimeRefAbsoluteTime:
        ptr = new CAbsoluteTimeTsProvider();
        break;
    default:
        Mediator::CTargetAgentRuntime::getUniqueInstance()->getLogger()->getPluginProviderLogger().error(
            "Invalid timestamp reference");
        break;
    }
    tsProviders.push_back(ptr);
    return ptr;
}

TimestampProviderFactory::~TimestampProviderFactory() {

    std::vector<PluginInterface::CTimestampProvider*>::iterator it;

    for (it = tsProviders.begin(); it != tsProviders.end(); it++)
    {
        delete *it;
    }
}
}
}

