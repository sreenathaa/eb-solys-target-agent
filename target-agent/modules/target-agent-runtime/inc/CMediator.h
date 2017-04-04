/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#ifndef CMEDIATOR_H_
#define CMEDIATOR_H_



#include "CComController.h"
#include "plugin_provider.h"
#include "config_provider.h"
#include "CTimestampProvider.h"
#include "MessageRecorder.hpp"
#include <iostream>
#include <memory>

#include "Poco/SharedPtr.h"

namespace TargetAgent {
namespace logger {
class CLogger;
}
}

namespace TargetAgent {

namespace Mediator {

class CTargetAgentRuntime {
public:
    CTargetAgentRuntime();
    static CTargetAgentRuntime* getUniqueInstance();

    void run();
    void shutdown();

    PluginManagement::PluginProxy* getPluginManager(void) const;


    Configuration::ConfigProvider*   getConfigProvider(void);
    Communicator::ComController*     getComController(void);
    MessageRecorder::CMessageRecorder*     getMessageRecorder(void);

    logger::CLogger* getLogger(void) const;

    Poco::SharedPtr<PluginInterface::CTimestampProvider> getTsProvider(void) const {
        return tsProvider;
    }
    virtual ~CTargetAgentRuntime() {
    }
private:

    CTargetAgentRuntime(const CTargetAgentRuntime& other);
    CTargetAgentRuntime& operator=(const CTargetAgentRuntime& other);

    static Poco::SharedPtr<CTargetAgentRuntime> single;


private:
    Poco::SharedPtr<Configuration::ConfigProvider> mConfigProvider;


    PluginManagement::PluginProxy* mPluginManager;

    Poco::SharedPtr<Communicator::ComController> mComHandler;
    Poco::SharedPtr<MessageRecorder::CMessageRecorder> mMessageRecorder;
    logger::CLogger* mLogger;
    Poco::SharedPtr<PluginInterface::CTimestampProvider> tsProvider;
};

}
}
#endif /* CMEDIATOR_H_ */

