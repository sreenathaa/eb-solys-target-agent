/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "Poco/Logger.h"
#include "Poco/File.h"
#include "config_mock.hpp"
#include "PathGenerator.hpp"
#include "Poco/RegularExpression.h"


using namespace TargetAgent::MessageRecorder;

namespace TargetAgentTests {

namespace MessageRecorderTests {

using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::InSequence;

TEST(PathGeneratorTests, generatePath) {
    unsigned int partNumber = 0;
    std::string logLevel("PRIO_ERROR");
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->setLogLevel(logLevel);
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->setTargetDirPath(Poco::Path::current());
    CPathHandler *gen = new CPathHandler();
    std::string fileName = gen->createPath(partNumber);
    Poco::RegularExpression re1("targetAgent[-0-9]{3}\.[0-9]{2}\.[0-9]{4}[-0-9]{3}\.[0-9]{2}\.[0-9]{2}\.*\.bin");
    std::cout<<fileName<<std::endl;
    bool match = re1.match(fileName);

    ASSERT_FALSE(match);
}

TEST(PathGeneratorTests, createDirsEmptyPath) {
    CPathHandler *gen = new CPathHandler();
    Poco::Path path;

    ASSERT_TRUE(true == gen->createIntermediateDirs(path));
}

TEST(PathGeneratorTests, createDirsValidPath) {
    unsigned int partNumber = 0;
    std::string logLevel("PRIO_INFORMATION");
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->setLogLevel(logLevel);
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->setTargetDirPath(Poco::Path::current().c_str());
    CPathHandler *gen = new CPathHandler();
    std::string fileName = gen->createPath(partNumber);
    Poco::Path path = Poco::Path::current();

    ASSERT_TRUE(true == gen->createIntermediateDirs(path));
}

TEST(PathGeneratorTests, createDirsNA) {
    CPathHandler *gen = new CPathHandler();
    Poco::Path path = Poco::Path::current().append("test");

    ASSERT_TRUE(true == gen->createIntermediateDirs(path));
}

TEST(PathGeneratorTests, pathNa) {
    unsigned int partNumber = 0;
    CPathHandler *gen = new CPathHandler();
    std::string fileName = gen->createPath(partNumber);


    ASSERT_TRUE(fileName!="");
}

TEST(PathGeneratorTests, pathNoCreateOuptut) {
    unsigned int partNumber = 0;
    std::string logLevel("PRIO_INFORMATION");
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->setLogLevel(logLevel);
    TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->getConfigProvider()->setTargetDirPath(Poco::Path::current().c_str());
    CPathHandler *gen = new CPathHandler();
    std::string fileName = gen->createPath(partNumber);
    Poco::Path path = Poco::Path::current();

    ASSERT_TRUE(true == gen->createIntermediateDirs(path));
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
