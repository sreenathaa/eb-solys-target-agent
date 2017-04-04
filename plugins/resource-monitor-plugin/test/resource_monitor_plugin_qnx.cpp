/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#include <cstdio>
#include <vector>
#include <map>
#include <iostream>

#include "Poco/File.h"
#include "Poco/Timestamp.h"
#include "Poco/Thread.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "TargetAgentPluginInterface.h"
#include "TargetAgentTsProviderInterface.h"

#include "procFsReader.h"

int devctl( int filedes,
            int dcmd,
            void * dev_data_ptr,
            size_t n_bytes,
            int * dev_info_ptr ){
    return 0;
}
namespace ResourceMonitor {
using namespace TargetAgent;


namespace Tests {

using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::InSequence;

class ResourceMonitorTest: public ::testing::Test {
protected:

    ResourceMonitorTest(){
    }

    virtual ~ResourceMonitorTest() {
    }
};


TEST_F(ResourceMonitorTest, procFSTestReadProcFSPos) {
    descriptor entryInfo = { 0 };
    sstat statInfo = { 0 };
    sstatm statmInfo = { 0 };

    entryInfo.dir = opendir(PROC_FS_ROOT);
    entryInfo.dp = NULL;
    entryInfo.pid = 0;

    bool readingValid = true;

    do
    {
        readingValid = readingValid
                       && readProcFs(&entryInfo, &statInfo, &statmInfo);
    }
    while (0 != entryInfo.dp);

    ASSERT_FALSE(readingValid);
}

}
;
}
;

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();

}
