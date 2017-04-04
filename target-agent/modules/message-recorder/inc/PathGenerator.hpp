/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef SOURCE_DIRECTORY__TARGET_AGENT_MODULES_MESSAGE_RECORDER_INC_PATHGENERATOR_HPP_
#define SOURCE_DIRECTORY__TARGET_AGENT_MODULES_MESSAGE_RECORDER_INC_PATHGENERATOR_HPP_

#include <iostream>
#include <sstream>

#include "target_agent_prot_frame.pb.h"
/*
#include "config_provider.h"
#include "CMessageDispatcher.h"
#include "plugin_provider.h"
#include "CLogger.hpp"
*/

#include "Poco/Path.h"
#include "Poco/LocalDateTime.h"

namespace TargetAgent {
namespace MessageRecorder {

class CPathHandler {
public:
    CPathHandler();
    CPathHandler(const CPathHandler& other) {
    }
    std::string createPath(unsigned int& mPartNumber);
    bool createIntermediateDirs(Poco::Path& outputPath);
    CPathHandler& operator=(const CPathHandler& other);
    ~CPathHandler() {
    }
private:
    Poco::LocalDateTime mStartTime;
    bool mIsSplitingActive;
};

}
;
}
;

#endif /* SOURCE_DIRECTORY__TARGET_AGENT_MODULES_MESSAGE_RECORDER_INC_PATHGENERATOR_HPP_ */
