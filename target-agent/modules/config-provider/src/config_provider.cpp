/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include <string>
#include <iostream>
#include <sstream>

#include "Poco/Util/IniFileConfiguration.h"
#include "Poco/Util/LayeredConfiguration.h"
#include "Poco/Exception.h"
#include "Poco/Exception.h"
#include "config_provider.h"

#ifdef _WIN32
#define RETAILMSG(cond,printf_exp)
#endif
using namespace TargetAgent::PluginManagement;
namespace TargetAgent {
namespace Configuration {

using Poco::Util::AbstractConfiguration;

ConfigProvider::ConfigProvider() :
        streamingPort(51789), cmdCtrlPort(1235),pluginList(), logLevel(""), logChannels(), recorderOn(
            false),fileSizeLimit(0),createOutputPath(false),recorderFilePrefix(""),targetDirPath(""),configurationFileName("conf.xml"), timeReference(
        eTimeRefUpTime) {
}

void ConfigProvider::initialize() {
    try
    {

        initializePlugins(initializeGlobal());

    }
    catch (const Poco::Exception& exp)
    {
#ifndef _WIN32
        std::cout << __func__ << "::" << __LINE__ << "::" << "Exception"
        << exp.displayText() << std::endl;
#else

        wchar_t ws[100];
        swprintf(ws, L"%hs", exp.displayText().c_str());
        RETAILMSG(1, (TEXT("TargetAgent[%s] Exception: %s \n"), _T(__FUNCTION__), ws));
#endif

    }

}

AutoPtr<XMLConfiguration> ConfigProvider::initializeGlobal()
throw (Poco::Exception) {

    Poco::Path cpathToConfigurationFile = findConfigurationFile(
                                              configurationFileName);

    AutoPtr<XMLConfiguration> pConf(
        new XMLConfiguration(cpathToConfigurationFile.toString()));

    Poco::Util::AbstractConfiguration* netCfg = pConf->createView("Network");

    streamingPort = netCfg->getInt("Port");

    cmdCtrlPort = netCfg->getInt("CmdCtrlPort");
    /*logging logic*/
    Poco::Util::AbstractConfiguration* loggerCfg =
        pConf->createView("Logging")->createView("Channels");

    AbstractConfiguration::Keys lkeys;
    loggerCfg->keys(lkeys);

    AbstractConfiguration::Keys::iterator logChannelKey = lkeys.begin();

    for (; logChannelKey != lkeys.end(); logChannelKey++)
    {

        Poco::Util::AbstractConfiguration* entryView = loggerCfg->createView(
                    *logChannelKey);
        ConfigChannel ch;

        ch.setName(*logChannelKey);
        ch.setIsChannelActive(entryView->getBool("ChannelActive"));

        logChannels.push_back(ch);
    }

    logLevel = pConf->createView("Logging")->getString("LoggingLevel");

    /*logging logic*/
    Poco::Util::AbstractConfiguration* recCfg = pConf->createView("Recorder");

    recorderOn = recCfg->getBool("enabled");
    fileSizeLimit = recCfg->getInt("fileSizeLimit");
    createOutputPath = recCfg->getBool("doCreateOutputPath");
    recorderFilePrefix = recCfg->getString("filePrefix");
    targetDirPath = recCfg->getString("targetDirPath");

    std::string sTimeUnit = pConf->createView("Units")->getString("TimeReference");

    if (0 == sTimeUnit.compare("UP_TIME"))
    {
        timeReference = eTimeRefUpTime;
    }
    else if (0 == sTimeUnit.compare("ABSOLUTE_SYSTEM_TIME"))
    {
        timeReference = eTimeRefAbsoluteTime;
    }

    return pConf;
}
void ConfigProvider::initializePlugins(AutoPtr<XMLConfiguration> pConf)
throw (Poco::Exception) {

    Poco::Util::AbstractConfiguration* pluginsCfg = pConf->createView("Plugin");

    std::vector<std::string> pluginKeys;
    AbstractConfiguration::Keys keys;
    pluginsCfg->keys(keys);

    AbstractConfiguration::Keys::iterator pluginKey = keys.begin();

    for (; pluginKey != keys.end(); pluginKey++)
    {

        std::vector<std::string> props;
        AbstractConfiguration::Keys kkkeys;
        pluginsCfg->keys(*pluginKey, kkkeys);

        PluginConfigItem pluginConfigItem(*pluginKey,
                                          pluginsCfg->getString(*pluginKey + ".Path[@attr]"));
        AbstractConfiguration::Keys::iterator it = kkkeys.begin();

        for (; it != kkkeys.end(); it++)
        {
            pluginConfigItem.addProperty(*it,
                                         pluginsCfg->getString(*pluginKey + "." + *it + "[@attr]"));
        }

        pluginList.push_back(pluginConfigItem);
    }
}


Poco::Path ConfigProvider::findConfigurationFile(
    const std::string& configFileName) const {
#ifndef _WIN32
    Poco::Path pathToConfigFile = Poco::Path::current();
#else

    HMODULE hModule = GetModuleHandleW(NULL);

    WCHAR* path = new WCHAR[MAX_PATH];
    GetModuleFileNameW(hModule, path, MAX_PATH);
    RETAILMSG(1, (TEXT("TargetAgent[%s] PATH: %s \r\n"), _T(__FUNCTION__), path));

    WCHAR* dir = new WCHAR[MAX_PATH];
    _wsplitpath_s(path, NULL, 0, dir, MAX_PATH, NULL, 0, NULL, 0);
    RETAILMSG(1, (TEXT("TargetAgent[%s] DIR: %s \r\n"), _T(__FUNCTION__), dir));

    char *cDir = new char[MAX_PATH];

    wcstombs(cDir, dir, MAX_PATH);

    Poco::Path pathToConfigFile(cDir);

    delete [] path, dir, cDir;
#endif

    if (Poco::Path::find(pathToConfigFile.toString(), configFileName,
                         pathToConfigFile))
    {
    } //empty if
    return pathToConfigFile;
}

Poco::UInt16 ConfigProvider::StreamingPort(void) const {
    return streamingPort;
}

Poco::UInt16 ConfigProvider::CmdCtrlPort(void) const {
    return cmdCtrlPort;
}

const std::list<PluginConfigItem>& ConfigProvider::Plugins() const {
    return pluginList;
}

const std::string ConfigProvider::getRecorderFilePrefix() const {
    return recorderFilePrefix;
}

const std::string ConfigProvider::getTargetDirPath(void) const
{
    return targetDirPath;
}

const bool ConfigProvider::isRecorderOn() const {
    return recorderOn;
}

}
;
}
;

