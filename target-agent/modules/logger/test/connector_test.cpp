/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketNotification.h"
#include "acceptor_test.hpp"

#include "CConnector.hpp"

namespace TargetAgentTests {

namespace Logger {

using testing::_;
using testing::Return;
using testing::InSequence;
using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::InSequence;
using namespace TargetAgent::logger;

AcceptorServiceHandler* socketAcceptorInst;

class CCSocketAcceptor: public Poco::Net::SocketAcceptor<AcceptorServiceHandler> {
public:
    CCSocketAcceptor(Poco::Net::ServerSocket& socket,
                     Poco::Net::SocketReactor& reactor) :
            Poco::Net::SocketAcceptor<AcceptorServiceHandler>(socket, reactor), ret(
        0) {
    }

    MOCK_METHOD1(createServiceHandler, AcceptorServiceHandler*(Poco::Net::StreamSocket& socket));
    //AcceptorServiceHandler* createServiceHandler(
    //  Poco::Net::StreamSocket& socket)
    /*    {
     socketAcceptorInst = new AcceptorServiceHandler(socket, *reactor());
     return socketAcceptorInst;
     }*/
private:

    //;                   // Constructor? (the {} brackets) are needed here.
    CCSocketAcceptor(CCSocketAcceptor const&);// Don't Implement
    void operator=(CCSocketAcceptor const&);// Don't implement
    AcceptorServiceHandler *ret;

public:
    ~CCSocketAcceptor()
    {
    }
    // CCSocketAcceptor(CCSocketAcceptor const&)               = delete;
    //void operator=(CCSocketAcceptor const&)  = delete;
}
;

TEST(ConnectorTest, testConnectionEstablished) {
    /*here we test that the connection was established and a CSocketAcceptor was created*/

    Poco::Net::SocketAddress sa("localhost", 1223);
    Poco::Net::ServerSocket ss(sa);
    Poco::Net::SocketReactor reactor1;
    Poco::Net::SocketReactor reactor2;
    CCSocketAcceptor* acceptor = new CCSocketAcceptor(ss, reactor1);
    EXPECT_CALL(*acceptor, createServiceHandler(_));
    Poco::NotificationQueue emptyQueue;
    CSocketConnector *connector = new CSocketConnector(sa, reactor2,
                                  &emptyQueue);

    Poco::Thread thr1;

    thr1.start(reactor1);
    sleep(1);
    reactor1.stop();
    reactor2.stop();
    Poco::Thread thr2;

    thr2.start(reactor2);
    thr1.join();
    thr2.join();

    delete socketAcceptorInst;
    delete acceptor;
}

}
;
}
;

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();

}
