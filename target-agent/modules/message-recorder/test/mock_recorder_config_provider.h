/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "gmock/gmock.h"
#include "config_provider.h"

namespace TargetAgent{
namespace MessageRecorder{
namespace Tests{

class MockRecorderConfigProvider : public Configuration::ConfigProvider
{
public:
    MOCK_CONST_METHOD0(getRecorderFilePath, const std::string());
    MOCK_CONST_METHOD0(isRecorderOn, const bool());
    MOCK_CONST_METHOD0(keepRecordedFile, const bool());
};

};
};
};
