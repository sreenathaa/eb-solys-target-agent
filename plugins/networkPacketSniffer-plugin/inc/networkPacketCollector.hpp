/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/


#ifndef PACKETSNIFFER_H_
#define PACKETSNIFFER_H_

#include "Poco/Activity.h"
#include <tins/tins.h>
#include "TargetAgentPluginInterface.h"
#include "target_agent_prot_networkPacketSniffer_plugin.pb.h"
using Poco::Activity;


namespace TargetAgentnetworkPacketSnifferPlugin
{


class CPacketSniffer
{
public:
    CPacketSniffer(const TargetAgent::PluginInterface::CMessageDispatcher* const senderHandle,const std::string& interface,
    		const std::vector<int>& ports, const std::string & payloadType, const std::string & prototoclType);
public:
    void start(void){_activity.start();}
    void stop(void){_activity.stop();}
    bool handle(Tins::PDU& udp);
protected:
    void runActivity();
    void initialize(void);
private:
    Poco::Activity<CPacketSniffer> _activity;
    Poco::SharedPtr<Tins::Sniffer> sniffer;
    Poco::Logger* logger;
    const TargetAgent::PluginInterface::CMessageDispatcher* const mSenderHandle;
    TargetAgent::Protocol::NetworkPacketSniffer::NetworkPacket  nwMsg;
    std::map<unsigned int,std::pair<unsigned int,std::string>> resolvedSrcPids;
    std::map<unsigned int,std::pair<unsigned int,std::string>> resolvedDstPids;
    std::string protocolType;
    std::string payloadType;
    Poco::Mutex mMsgQMutex;

private:
    template <class T>
    void handlingSomeIPMessage (Tins::PDU& pdu);
    void getSomeIPPayload(const Tins::RawPDU::payload_type& payload, std::stringstream &payloadStream);
    void clearMessage(void);
    template <class T>
    std::string fillInJsonMessage(const T& prorocolT, std::stringstream & buffer);
    void dispatchMessage( TargetAgent::Protocol::NetworkPacketSniffer::NetworkPacket*  nwMsg);
    template <class T>
    void fillProtobuf(const std::string& payload, const Tins::IP& ip, const T& protocolT, TargetAgent::Protocol::NetworkPacketSniffer::MESSAGE_SOURCE typeOfPackt);
    char* decodeWebsockFrame(const Tins::RawPDU::payload_type& payload);
    template <class T>
    void handlingSocketMessage (Tins::PDU  &pdu);
public:
	static const int PROCESS_NAME_LENGTH = 256;
	static const int PAYLOAD_SIZE_126 = 126;
	static const int PAYLOAD_SIZE_127 = 127;
	static const int MASK_SIZE = 4;
	static const char DATA_FRAME_MASK = '\x81';
	static const char PAYLOAD_LENGTH_MASK = 0x7f;
	static const char PAYLOAD_MASKBIT_MASK = 0x80;
};
}

#endif /* PACKETSNIFFER_H_ */
