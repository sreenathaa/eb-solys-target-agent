/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/
#include "Poco/Timestamp.h"
#include "Poco/Thread.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "TargetAgentPluginInterface.h"
#include "TargetAgentTsProviderInterface.h"
#include "socket_reader_mocks.hpp"
#include "../inc/socket_reader_plugin.h"
#include "Poco/Net/SocketConnector.h"

namespace TargetAgentLog4jPlugin
{
using namespace TargetAgent;
using namespace Log4jReaderMocks;


namespace Tests
{

using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::InSequence;

class ClientServiceHandler
{
public:
    ClientServiceHandler(StreamSocket& socket, SocketReactor& reactor):
            _socket(socket),
            _reactor(reactor)
    {
    }

    ~ClientServiceHandler()
    {
    }


private:
    StreamSocket       _socket;
    SocketReactor&     _reactor;
};

class CSocketReaderPluginTest: public ::testing::Test
{
protected:
    TargetAgentLog4jPlugin::CSocketReaderPlugin* socketReaderPlugin;

    CSocketReaderPluginTest() :
            socketReaderPlugin(NULL)
    {
        CMessageDispatcher* taDispatcherMock =
            new Log4jReaderMocks::TaDispatcher();
        PluginInterface::CTimestampProvider* tsProvider =
            new Log4jReaderMocks::CTimestampProviderMock();
        socketReaderPlugin = new CSocketReaderPlugin(taDispatcherMock,
                             tsProvider);
    }

    virtual ~CSocketReaderPluginTest()
    {
        delete socketReaderPlugin;
    }

    void createResourceMonitor()
    {
        CMessageDispatcher* taDispatcherMock =
            new Log4jReaderMocks::TaDispatcher();
        PluginInterface::CTimestampProvider* tsProvider =
            new Log4jReaderMocks::CTimestampProviderMock();
        socketReaderPlugin = new CSocketReaderPlugin(taDispatcherMock,
                             tsProvider);
    }

    void onMessageReceivedResourceMonitor(int payloadLength,
                                          const unsigned char* payloadBuffer)
    {
        socketReaderPlugin->onMessageReceived(payloadLength, payloadBuffer);
    }

    void MessageTypeResourceMonitor()
    {
        socketReaderPlugin->MessageType();

    }

    void onHostConnectionEstablishedResourceMonitor()
    {
        socketReaderPlugin->onConnectionEstablished();
    }

    void onHostConnectionLostResourceMonitor()
    {
        socketReaderPlugin->onConnectionLost();
    }

    void startResourceMonitor()
    {
        socketReaderPlugin->startPlugin();
    }

    void stopResourceMonitor()
    {
        socketReaderPlugin->stopPlugin();
    }

};

TEST_F(CSocketReaderPluginTest, createResourceMonitorTest)
{
    socketReaderPlugin = NULL;
    CSocketReaderPluginTest::createResourceMonitor();
    ASSERT_TRUE(socketReaderPlugin != 0);
}

TEST_F(CSocketReaderPluginTest, onMessageReceivedTest)
{
    socketReaderPlugin = NULL;
    CSocketReaderPluginTest::createResourceMonitor();
    socketReaderPlugin->onMessageReceived(5, 0);
}

TEST_F(CSocketReaderPluginTest, connectionEstablish)
{
    socketReaderPlugin = NULL;
    CSocketReaderPluginTest::createResourceMonitor();
    Poco::Net::SocketAddress ssa;
    SocketReactor reactor;
    Poco::Net::SocketAddress sa("localhost", 1223);
    Poco::Net::SocketConnector<ClientServiceHandler> connector(sa, reactor);
}


}
;
}
;

int main(int argc, char** argv)
{
    ::testing::InitGoogleMock(&argc, argv);
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();

}
