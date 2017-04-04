/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "Poco/NotificationQueue.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/ServerSocket.h"
#include "CommandControlServer.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"


namespace TargetAgent {
namespace CommunicatorTest {

using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::InSequence;

class CCommanControlServerTest: public ::testing::Test {
protected:

    CCommanControlServerTest() {
        connectionAcceptor = new TargetAgent::Communicator::CCSocketAcceptor(
                                 commandControlPort, reactor, &eventQueue);
        reactorThread.start(reactor);
        reactor.stop();
        reactorThread.join();
    }

    virtual ~CCommanControlServerTest() {
    }

    void createMessageRecorder() {

    }
private:
    Poco::NotificationQueue eventQueue;
    Poco::Net::SocketAcceptor<TargetAgent::Communicator::CCommanControlServer> *connectionAcceptor;
    Poco::Net::ServerSocket commandControlPort;
    Poco::Net::SocketReactor reactor;
    Poco::Thread reactorThread;
};

TEST_F(CCommanControlServerTest, CreateCommandControl) {

}

}
;

}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
