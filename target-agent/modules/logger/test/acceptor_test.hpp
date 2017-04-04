/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef TARGET_AGENT_MODULES_LOGGER_TEST_ACCEPTOR_TEST_CPP_
#define TARGET_AGENT_MODULES_LOGGER_TEST_ACCEPTOR_TEST_CPP_

#include "gmock/gmock.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketReactor.h"

class AcceptorServiceHandler
{
public:
    AcceptorServiceHandler(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor& reactor):
            _socket(socket),
            _reactor(reactor)
    {
    }

    MOCK_METHOD1(onReadable, void(Poco::Net::ReadableNotification* pNf));
    MOCK_METHOD1(onWritable, void(Poco::Net::WritableNotification* pNf));
    MOCK_METHOD1(onTimeout, void(Poco::Net::TimeoutNotification* pNf));


private:
    Poco::Net::StreamSocket       _socket;
    Poco::Net::SocketReactor&     _reactor;

};





#endif /* TARGET_AGENT_MODULES_LOGGER_TEST_ACCEPTOR_TEST_CPP_ */
