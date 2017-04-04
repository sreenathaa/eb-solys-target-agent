/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include <time.h>
#include <unistd.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "config_mock.hpp"
#include "TargetAgentTsProviderInterface.h"
#include "TargetAgentPluginInterface.h"
#include "CTimestampProvider.h"
#include "logger_mock.hpp"
namespace PluginManagementTests {

using namespace TargetAgent;
using namespace PluginManagement;

namespace Tests {

using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::InSequence;

TEST(TimestampProviderTest, createTimestampProviderTest) {

    PluginInterface::CTimestampProvider* tsProvider =
        (new TimestampProviderFactory())->createTimestamp(
            eTimeRefUpTime);
    ASSERT_TRUE(tsProvider != 0);
    delete tsProvider;
}

TEST(TimestampProviderTest, invalidTimestampProviderTest1) {
    PluginInterface::CTimestampProvider* tsProvider =
        (new TimestampProviderFactory())->createTimestamp(
            eTimeRefInvalid);
    ASSERT_TRUE(tsProvider == 0);
}

TEST(TimestampProviderTest, invalidTimestampProviderTest2) {

    PluginInterface::CTimestampProvider* tsProvider =
        (new TimestampProviderFactory())->createTimestamp(
            eTimeRefMax);
    ASSERT_TRUE(tsProvider == 0);
}

TEST(TimestampProviderTest, timestampAccuracyUpTime) {
    unsigned long long tsInMs = 0;
    struct timespec ts = {
                             0 };
    PluginInterface::CTimestampProvider* tsProvider =
        (new TimestampProviderFactory())->createTimestamp(
            eTimeRefUpTime);
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
    {
        tsInMs = (((uint32_t) ts.tv_sec * 1000
                   + (uint32_t) ts.tv_nsec / 1000000LL));
    }
    ASSERT_TRUE(tsProvider->createTimestamp() == tsInMs);
    delete tsProvider;
}

TEST(TimestampProviderTest, timestampAccuracySystemTime) {
    Poco::Timestamp ts;
    unsigned long long tsInMs = (ts.epochMicroseconds() / 1000);
    TimestampProviderFactory* factory = new TimestampProviderFactory();
    PluginInterface::CTimestampProvider* tsProvider =
        (factory)->createTimestamp(
            eTimeRefAbsoluteTime);

    ASSERT_TRUE(tsProvider->createTimestamp() == tsInMs);
    delete factory;
}

};
}
;

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();

}
