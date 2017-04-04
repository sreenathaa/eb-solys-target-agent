/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef LOGGER_MOCK_HPP_
#define LOGGER_MOCK_HPP_

#include "gmock/gmock.h"
#include "Poco/Logger.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/AutoPtr.h"
#include "Poco/NullChannel.h"
using Poco::Logger;
using Poco::ConsoleChannel;
using Poco::AutoPtr;
using Poco::NullChannel;

namespace TargetAgent {
namespace ts_tests {

class CLogger {
public:

    MOCK_CONST_METHOD0(getGlobalLogger,Poco::Logger&());
    MOCK_CONST_METHOD0(getCommLogger,Poco::Logger&());
    MOCK_CONST_METHOD0(getConfigLogger,Poco::Logger&());
    MOCK_CONST_METHOD0(getRecorderLogger,Poco::Logger&());
    MOCK_CONST_METHOD0(getPluginProviderLogger,Poco::Logger&());
    MOCK_CONST_METHOD0(getRuntimeLogger,Poco::Logger&());

};

}
;
}
;
#endif /* LOGGER_MOCK_HPP_ */
