/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef CONFIG_MOCK_HPP_
#define CONFIG_MOCK_HPP_

#include <list>
#include <map>
#include <string>
#include "Poco/Path.h"
#include <memory>


namespace TargetAgent {

namespace Configuration {

class PluginConfigItem {
public:
    PluginConfigItem(std::string name, std::string path) :
    name(name), path(path) {
    }

    const std::string& Path() const {
        return path;
    }
    const std::string& Name() const {
        return name;
    }
    void addProperty(const std::string& name, const std::string& value) {
        pluginProperties.insert(
            std::pair<std::string, std::string>(name, value));
    }

    const std::map<std::string, std::string>& getPluginProperties() const {
        return pluginProperties;
    }

    /*compiler generated functions behavior*/
PluginConfigItem(const PluginConfigItem&) = default;
    PluginConfigItem(PluginConfigItem&&) = delete;
    PluginConfigItem& operator=(const PluginConfigItem&) = delete;
    PluginConfigItem& operator=(PluginConfigItem&&) = delete;
    virtual ~PluginConfigItem() {
    }

private:
    const std::string name;
    const std::string path;
    std::map<std::string, std::string> pluginProperties;
};

class ConfigChannel {
public:
    ConfigChannel() :
    isChannelActive(false), name("") {
    }

    void setName(const std::string& channel) {
        name = channel;
    }

    void setIsChannelActive(bool isActive) {
        isChannelActive = isActive;
    }

    bool getIsChannelActive(void) const {
        return isChannelActive;
    }

    bool operator==(const std::string & r) const {
        return (0
                == const_cast<const ConfigChannel *>(this)->getName().compare(r));
    }

    const std::string& getName(void) const {
        return name;
    }

    /*compiler generated functions behavior*/
    //ConfigChannel(const ConfigChannel&) = delete;
    /* ConfigChannel(ConfigChannel&&) = delete;
     ConfigChannel& operator=(const ConfigChannel&) = delete;
     ConfigChannel& operator=(ConfigChannel&&) = delete;*/
    virtual ~ConfigChannel() {
    }
private:
    bool isChannelActive;
    std::string name;
};

class ConfigProvider {
public:
    ConfigProvider();

    /*Network Configuration*/
    Poco::UInt16 TCPPort();

    bool isOfflineModeEnabled() const;

    /*Recorder Configuration*/
    const std::string getRecorderFilePrefix() const;

    const bool isRecorderOn() const;


    const bool keepRecordedFile() const;
    void setRecordedFile(bool rec) {
        recorderOn = rec;
    };
    const std::string getTargetDirPath(void) const;
    void setTargetDirPath(const std::string& path) {
        mTargetDirPath =path;
    }
    void setDoCreateOutputPath(bool val) {
        createOutputPath =val;
    }
    const bool doCreateOutputPath(void) const {
        return createOutputPath;
    }
    /*Logger Configuration*/
    const std::list<Configuration::ConfigChannel>& getLogChannels() const;
    void addLogChannel(ConfigChannel& ch) {
        logChannels.push_back(ch);
    }
    void clearChannel(void) {
        logChannels.clear();
    }
    const std::string& getLogLevel() const ;
    void setLogLevel(std::string& level) {
        logLevel = level;
    }

    void addPlugin(Configuration::PluginConfigItem& plugin){
        pluginList.push_back(plugin);
    }

    /*Plugin Configuration*/
    const std::list<Configuration::PluginConfigItem>& Plugins() const;

    /*compiler generated functions behavior*/
    ConfigProvider(const ConfigProvider&) = delete;
    ConfigProvider(ConfigProvider&&) = delete;
    ConfigProvider& operator=(const ConfigProvider&) = delete;
    ConfigProvider& operator=(ConfigProvider&&) = delete;
    virtual ~ConfigProvider() {
    }
private:
    Poco::Int16 streamingPort;
    bool isOfflineServiceEnabled;
    std::list<Configuration::PluginConfigItem> pluginList;
    std::string logLevel;
    std::list<Configuration::ConfigChannel> logChannels;
    bool recorderOn;
    std::string recorderFilePrefix;
    Poco::Path pathToConfigurationFile;
    const std::string configurationFileName;
    std::string mTargetDirPath;
    bool createOutputPath;

private:
    Poco::Path findConfigurationFile(const std::string& configFileName) const;

};
}
;

namespace ts_logger{
class CLogger;
}

namespace Mediator {

namespace PluginManagement {
class PluginProxy;
}

namespace Communicator {
class ComController;
}

namespace MessageRecorder {
class MessageRecorder;
}

class CTargetAgentRuntime {
public:
    static CTargetAgentRuntime* getUniqueInstance();
    void run() {
    }
    Configuration::ConfigProvider* getConfigProvider(void);
    std::shared_ptr<PluginManagement::PluginProxy> getPluginManager(void) const;

    Communicator::ComController* getComController(void) {
        return 0;
    }
    ;

    MessageRecorder::MessageRecorder * getMessageRecorder(void) {
        return 0;
    }
    ;

    ts_logger::CLogger* getLogger(void) const;

    /*compiler generated functions behavior*/
    CTargetAgentRuntime(const CTargetAgentRuntime&) = delete;
    CTargetAgentRuntime(CTargetAgentRuntime&&) = delete;
    CTargetAgentRuntime& operator=(const CTargetAgentRuntime&) = delete;
    CTargetAgentRuntime& operator=(CTargetAgentRuntime&&) = delete;
    virtual ~CTargetAgentRuntime() {
    }
private:
    CTargetAgentRuntime() {
    }
    static CTargetAgentRuntime* single;
};

}
;
}
;

#endif /* CONFIG_MOCK_HPP_ */
