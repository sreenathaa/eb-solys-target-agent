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
#include "Poco/Activity.h"
#include "Poco/SharedLibrary.h"
#include "Poco/Path.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "dbus-monitor-plugin.h"
#include "TargetAgentPluginInterface.h"
#include <dbus/dbus.h>
#include "dbus_monitor_mocks.hpp"


namespace TargetAgentDbusMonitor {
using namespace TargetAgent;
namespace Tests {

using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::InSequence;
using namespace TargetAgent::PluginInterface;
typedef void* (*PluginFactoryMethod)(CMessageDispatcher* taDispatcherMock, CTimestampProvider* tsProvider);
class DBusMonitorTest: public ::testing::Test {

protected:
	static const int PLUGIN_LIVE_DURATION_IN_S = 5;
	PluginInterface::TargetAgentPluginInterface* dbusMonitorPlugin = 0;
    std::shared_ptr<CMessageDispatcher> taDispatcherMock;
    Poco::SharedLibrary pluginLibrary;

    DBusMonitorTest() :
    dbusMonitorPlugin() {
        taDispatcherMock.reset(new TaDispatcherMocker::TaDispatcher());
    }

    virtual ~DBusMonitorTest() {

    }

    void createDbusMonitor(CMessageDispatcher* taDispatcherMock, CTimestampProvider* tsProvider ) {

        Poco::Path pathToConfigFile = Poco::Path::current();
        Poco::Path::find(pathToConfigFile.toString(),
                         "../../bin/debug/libdbus-monitor-plugind.so", pathToConfigFile);

        try
        {
            pluginLibrary.load(pathToConfigFile.toString());
        }
        catch (Poco::LibraryLoadException& e)
        {
            std::cout << "Failed to load::" << pathToConfigFile.toString()
            << "::" << std::endl;
            std::cout << "Reason: " << e.what() << " :: " << e.name()
            << std::endl;
        }

        if (pluginLibrary.isLoaded())
        {

            if (pluginLibrary.hasSymbol("createPluginInstance"))
            {
                PluginFactoryMethod createPluginInstance =
                    (PluginFactoryMethod) pluginLibrary.getSymbol(
                        "createPluginInstance");

                if (createPluginInstance != NULL)
                {
                    void* rawPointerToPluginInstance = createPluginInstance(taDispatcherMock,tsProvider);

                    if (rawPointerToPluginInstance != NULL)
                    {
                        dbusMonitorPlugin =
                            static_cast<PluginInterface::TargetAgentPluginInterface*>(rawPointerToPluginInstance);
                        if (dbusMonitorPlugin == NULL)
                        {
                            free(rawPointerToPluginInstance);
                        }
                    }
                }
            }

        }

    }

    void onMessageReceivedDbusMonitor(int payloadLength,
                                      unsigned char* payloadBuffer) {
        dbusMonitorPlugin->onMessageReceived(payloadLength, payloadBuffer);
    }

    void MessageTypeDbusMonitor() {
        dbusMonitorPlugin->MessageType();

    }

    void onHostConnectionEstablishedDBusMonitor() {
        dbusMonitorPlugin->onConnectionEstablished();
    }

    void onHostConnectionLostDbusMonitor() {
        dbusMonitorPlugin->onConnectionLost();
    }

    bool startDBusMonitor() {
        return dbusMonitorPlugin->startPlugin();
    }

    bool stopDBusMonitor() {
        return dbusMonitorPlugin->stopPlugin();
    }

};

TEST_F(DBusMonitorTest, startStopTest) {
    dbusMonitorPlugin = 0;
    CMessageDispatcher* taDispatcherMock = new TaDispatcherMocker::TaDispatcher();
    PluginInterface::CTimestampProvider* tsProvider = new TaDispatcherMocker::CTimestampProviderMock();
    DBusMonitorTest::createDbusMonitor(taDispatcherMock,tsProvider);
    ASSERT_TRUE(dbusMonitorPlugin != NULL);
    sleep(PLUGIN_LIVE_DURATION_IN_S);
    dbusMonitorPlugin->stopPlugin();

    ASSERT_TRUE(dbusMonitorPlugin != NULL);
}

TEST_F(DBusMonitorTest, validMessageType) {
    dbusMonitorPlugin = 0;
    CMessageDispatcher* taDispatcherMock = new TaDispatcherMocker::TaDispatcher();
    PluginInterface::CTimestampProvider* tsProvider = new TaDispatcherMocker::CTimestampProviderMock();
    DBusMonitorTest::createDbusMonitor(taDispatcherMock,tsProvider);
    ASSERT_TRUE(dbusMonitorPlugin != NULL);
    bool messageTypeValid = (Protocol::CommonDefinitions::MSG_TYPE_DBUS
                             == dbusMonitorPlugin->MessageType());

    ASSERT_TRUE(messageTypeValid);
}

TEST_F(DBusMonitorTest, onMessageReceivedTest) {
    CMessageDispatcher* taDispatcherMock = new TaDispatcherMocker::TaDispatcher();
    PluginInterface::CTimestampProvider* tsProvider = new TaDispatcherMocker::CTimestampProviderMock();
    DBusMonitorTest::createDbusMonitor(taDispatcherMock,tsProvider);

    ASSERT_TRUE(dbusMonitorPlugin != NULL);
    unsigned char helloPlugin[] = { 0xa, 0xb, 0xc, 0xd };
    DBusMonitorTest::onMessageReceivedDbusMonitor(4, helloPlugin);
}
}
;
}
;

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();

}
