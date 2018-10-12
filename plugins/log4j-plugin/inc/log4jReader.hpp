/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef PLUGINS_LOG4J_PLUGIN_INC_LOG4JREADER_HPP_
#define PLUGINS_LOG4J_PLUGIN_INC_LOG4JREADER_HPP_

#include "Poco/Activity.h"
#include "Poco/Logger.h"
#include "target_agent_prot_log4j_plugin.pb.h"

namespace TargetAgent {
namespace PluginInterface {
class CMessageDispatcher;
class CTimestampProvider;
}
}


namespace TargetAgentlog4jPlugin {

class log4jReader {
public:
    log4jReader(
        const TargetAgent::PluginInterface::CMessageDispatcher * const mMsgSenderHDL,const TargetAgent::PluginInterface::CTimestampProvider * const  timestampProvider, const std::string logFileName);

    void start() {
        _activity.start();
    }

    void stop() {
        _activity.stop();
        _activity.wait();
    }

    virtual ~log4jReader();
protected:
    void runActivity();
private:
    const TargetAgent::PluginInterface::CMessageDispatcher * const mMsgSenderHDL;
    const TargetAgent::PluginInterface::CTimestampProvider * const  mTimestampProvider;
    Poco::Activity<log4jReader> _activity;
    const std::string mLogFileName;
    Poco::Logger* logger;
    TargetAgent::Protocol::Log4j::LogData logMessage;
    std::string lastValidDate;
    std::string lastValidLogLevel;
    static const int read_timeout = 1;
private:
    void resetEof(std::ifstream& ifs);
    unsigned long long extractDateTime(const std::string& dateTime);
    void tryParseDateTime(const std::string& logLine, std::string* payload,unsigned long long* ts);
    void dispatchMessage(unsigned long long ts);
};
}

#endif /* PLUGINS_LOG4J_PLUGIN_INC_LOG4JREADER_HPP_ */
