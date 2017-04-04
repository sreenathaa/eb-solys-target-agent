/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef CTIMESTAMPPROVIDER_HPP_
#define CTIMESTAMPPROVIDER_HPP_

#ifdef __linux__
#include <time.h>
#include <unistd.h>
#elif _WIN32
// windows code goes here
#endif

#include <iostream>
#include <vector>

#include "Poco/Timestamp.h"

#include "TargetAgentTsProviderInterface.h"

namespace TargetAgent {
namespace PluginManagement {

enum ETimeRef {
    eTimeRefInvalid   = 0,
    eTimeRefUpTime    = 1,
    eTimeRefAbsoluteTime  = 2,
    eTimeRefMax    = 3
};

class CUpTimeTsProvider: public TargetAgent::PluginInterface::CTimestampProvider {
public:
    CUpTimeTsProvider() {
    }
    unsigned long long createTimestamp(void) const;
private:

    CUpTimeTsProvider(const CUpTimeTsProvider& other);
    CTimestampProvider& operator=(const CUpTimeTsProvider& other);
    ~CUpTimeTsProvider() {
    }
};

class CAbsoluteTimeTsProvider: public TargetAgent::PluginInterface::CTimestampProvider {
public:
    CAbsoluteTimeTsProvider() {
    }

    unsigned long long createTimestamp(void) const;
private:
    CAbsoluteTimeTsProvider(const CAbsoluteTimeTsProvider& other);
    CAbsoluteTimeTsProvider& operator=(const CAbsoluteTimeTsProvider& other);
    ~CAbsoluteTimeTsProvider() {
    }
};

class TimestampProviderFactory {
public:
    TimestampProviderFactory() {
    }

    ~TimestampProviderFactory();

    PluginInterface::CTimestampProvider* createTimestamp(
        ETimeRef timeUnit);
private:

    std::vector<PluginInterface::CTimestampProvider*> tsProviders;
};

}
}

#endif /* CTIMESTAMPPROVIDER_HPP_ */
