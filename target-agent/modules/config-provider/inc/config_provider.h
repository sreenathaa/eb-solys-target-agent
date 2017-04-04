/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#ifndef CONFIG_PROVIDER_H_
#define CONFIG_PROVIDER_H_

#include <string>
#include <map>
#include <list>
#include <string>

#include "Poco/AutoPtr.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/Path.h"
#include "Poco/AutoPtr.h"

#include "CTimestampProvider.h"

using Poco::AutoPtr;
using Poco::Util::XMLConfiguration;


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

    virtual ~PluginConfigItem() {
    }

    PluginConfigItem& operator=(const PluginConfigItem& other);
private:

    const std::string name;
    const std::string path;
    std::map<std::string, std::string> pluginProperties;
};

class ConfigChannel {
public:
    ConfigChannel() :
    isChannelActive(false),name(""){
    }

    void setName(const std::string& channel){
        name = channel;
    }

    void setIsChannelActive(bool isActive) {
        isChannelActive = isActive;
    }

    bool getIsChannelActive(void) const {
        return isChannelActive;
    }

    bool operator==(const std::string & r) const {
        return (0 == const_cast<const ConfigChannel *>(this)->getName().compare(r)); }

    const std::string& getName(void) const {
        return name;
    }

    virtual ~ConfigChannel() {
    }
private:
    ConfigChannel& operator=(const ConfigChannel& other);
private:
    bool isChannelActive;
    std::string name;
};

class ConfigProvider {

public:
    ConfigProvider();

    void initialize();

    /*Network Configuration*/
    Poco::UInt16 StreamingPort(void) const;
    Poco::UInt16 CmdCtrlPort(void) const;

    /*Recorder Configuration*/
    const std::string getRecorderFilePrefix() const;
    const bool isRecorderOn() const;
    const unsigned int getFileSizeLimit(void) const {
        return fileSizeLimit;
    }
    const bool doCreateOutputPath(void) const {
        return createOutputPath;
    }

    /*Logger Configuration*/
    const std::list<ConfigChannel>& getLogChannels() const {
        return logChannels;
    }
    const std::string& getLogLevel() const {
        return logLevel;
    }

    /*Plugin Configuration*/
    const std::list<PluginConfigItem>& Plugins() const;

    PluginManagement::ETimeRef getTimeReference(void) const {
        return timeReference;
    }

    const std::string getTargetDirPath(void) const;
    virtual ~ConfigProvider() {
    }
private:
    Poco::AutoPtr<XMLConfiguration> initializeGlobal() throw (Poco::Exception);
    void initializePlugins(Poco::AutoPtr<XMLConfiguration> pConf) throw (Poco::Exception);

    Poco::Int16 streamingPort;
    Poco::Int16 cmdCtrlPort;
    std::list<PluginConfigItem> pluginList;
    std::string logLevel;
    std::list<ConfigChannel> logChannels;
    bool recorderOn;
    Poco::UInt32 fileSizeLimit;
    bool createOutputPath;
    std::string recorderFilePrefix;
    std::string targetDirPath;
    Poco::Path pathToConfigurationFile;
    const std::string configurationFileName;
    PluginManagement::ETimeRef timeReference;
private:
    Poco::Path findConfigurationFile(const std::string& configFileName) const;
    ConfigProvider(const ConfigProvider& other);
    ConfigProvider& operator=(const ConfigProvider& other);
};

}
}


#endif /* CONFIG_PROVIDER_H_ */
