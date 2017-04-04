/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef TARGET_AGENT_MODULES_LOGGER_INC_CCONNECTOR_HPP_
#define TARGET_AGENT_MODULES_LOGGER_INC_CCONNECTOR_HPP_

#include "Poco/Notification.h"
#include "Poco/NotificationQueue.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/AutoPtr.h"
#include "Poco/Net/SocketConnector.h"
#include "Poco/Message.h"

namespace TargetAgent
{
namespace logger
{


class CLogNotification: public Poco::Notification
{
public:
    CLogNotification(const Poco::Message& data): _data(data)
    {
        _data.getPriority();
    }
    const Poco::Message&  data() const
    {
        return _data;
    }
private:
    Poco::Message _data;
};



class CLoggerServiceHandler
{
public:
    CLoggerServiceHandler(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor& reactor);

    ~CLoggerServiceHandler();
    void onError(Poco::Net::ErrorNotification* pNotification);
    void setQueue(Poco::NotificationQueue* queue);
    void onTimeout(Poco::Net::TimeoutNotification* pNf);
    void onWritable(Poco::Net::WritableNotification* pNf);
    void onShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf);
    void sendXMLConfiguration(void);
    void buildMessage(const char* message, unsigned int  messageLength);
    void sendVersionInformation(void);
private:
    bool writeNBytesToSocket(const char* bytes, unsigned int bytesToWrite);
private:
    enum
    {
        BUFFER_SIZE = 1024
    };

    Poco::Net::StreamSocket   _socket;
    Poco::Net::SocketReactor& _reactor;
    char*          _pBuffer;
    Poco::NotificationQueue* metaDataQueue;
};

class CSocketConnector: public Poco::Net::SocketConnector<CLoggerServiceHandler>
{
public:
    CSocketConnector(Poco::Net::SocketAddress& sa, Poco::Net::SocketReactor& reactor,  Poco::NotificationQueue* queue);
    CLoggerServiceHandler* createServiceHandler();

    void onReadable(Poco::Net::WritableNotification* r);
    void onError(int errorCode);
public:
    Poco::NotificationQueue* metaDataQueue;

};
}
}


#endif /* TARGET_AGENT_MODULES_LOGGER_INC_CCONNECTOR_HPP_ */
