/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "logger_mock.hpp"
#include "config_mock.hpp"

namespace TargetAgent {

namespace ts_logger {
class CLogger {
public:

    CLogger() :
            taLogger(0), commLogger(0), configLogger(0), recLogger(0), pluginLogger(
        0), runtimeLogger(0), stringToEnum() { /*flush all messages*/
        AutoPtr<ConsoleChannel> pChannel(new ConsoleChannel);
        Logger::root().setChannel(pChannel);
        // AutoPtr<NullChannel> pNull(new NullChannel);
        //Poco::Logger::root().setChannel(pNull);

        pluginLogger = &Logger::get("test");
        configLogger = &Logger::get("test");
        recLogger = &Logger::get("test");
        runtimeLogger = &Logger::get("test");
    }

    ~CLogger();

    Poco::Logger& getGlobalLogger(void) const {
        return *commLogger;
    }
    ;
    Poco::Logger& getCommLogger(void) const {
        return *commLogger;
    }
    ;
    Poco::Logger& getConfigLogger(void) const {
        return *configLogger;
    }
    ;
    Poco::Logger& getRecorderLogger(void) const {
        return *recLogger;
    }
    ;
    Poco::Logger& getPluginProviderLogger(void) const {
        return *pluginLogger;
    }
    ;
    Poco::Logger& getRuntimeLogger(void) const {
        return *runtimeLogger;
    }
    ;

private:
    CLogger(const CLogger&);
    CLogger& operator =(const CLogger&);
private:
    Poco::Logger* taLogger;
    Poco::Logger* commLogger;
    Poco::Logger* configLogger;
    Poco::Logger* recLogger;
    Poco::Logger* pluginLogger;
    Poco::Logger* runtimeLogger;
    std::map<std::string, Poco::Message::Priority> stringToEnum;
};

}
namespace Mediator {

CTargetAgentRuntime* single = 0;

CTargetAgentRuntime* CTargetAgentRuntime::getUniqueInstance() {
    static CTargetAgentRuntime* runtime = new CTargetAgentRuntime();
    return runtime;
}

ts_logger::CLogger* CTargetAgentRuntime::getLogger(void) const {
    static ts_logger::CLogger* cfg = new ts_logger::CLogger();
    return cfg;
}

Configuration::ConfigProvider* CTargetAgentRuntime::getConfigProvider(void) {
    static Configuration::ConfigProvider* cfg =
        new Configuration::ConfigProvider();
    return cfg;
}

}
;
}
;

namespace TargetAgent {

namespace Configuration {

ConfigProvider::ConfigProvider() {
}

/*Network Configuration*/
Poco::UInt16 ConfigProvider::TCPPort() {
    return 0;
}
;
bool ConfigProvider::isOfflineModeEnabled() const {
    return false;
}

/*Recorder Configuration*/
const std::string ConfigProvider::getRecorderFilePrefix() const {
    return std::string("");
}
const bool ConfigProvider::isRecorderOn() const {
    return recorderOn;
}
const bool ConfigProvider::keepRecordedFile() const {
    return false;
}

/*Logger Configuration*/

const std::string& ConfigProvider::getLogLevel() const {
    return logLevel;
}


const std::string ConfigProvider::getTargetDirPath(void) const{
    return mTargetDirPath;
}
/*Plugin Configuration*/
const std::list<Configuration::PluginConfigItem>& ConfigProvider::Plugins() const {
    return pluginList;
}

/*Logger Configuration*/
const std::list<Configuration::ConfigChannel>& ConfigProvider::getLogChannels() const {
    return logChannels;
}
}
;
}
;
