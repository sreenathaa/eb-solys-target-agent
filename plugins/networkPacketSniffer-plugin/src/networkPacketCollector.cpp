/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <networkPacketCollector.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <sstream>

using namespace Tins;

namespace TargetAgentnetworkPacketSnifferPlugin {

CPacketSniffer::CPacketSniffer(const TargetAgent::PluginInterface::CMessageDispatcher* const senderHandle,const std::string& interface, const std::vector<int>& ports,
		const std::string & payloadT, const std::string & prototoclT):
		_activity(this, &CPacketSniffer::runActivity),mSenderHandle(senderHandle),nwMsg(),
		resolvedSrcPids(),resolvedDstPids(), payloadType(payloadT), protocolType(prototoclT){

	SnifferConfiguration config;
    for (std::vector<int>::const_iterator port=ports.begin() ; port != ports.end(); ++port) // access by reference to avoid copying
    {
		std::stringstream strStream;
		strStream<<*port;
        config.set_filter("port "+strStream.str());
    }


    config.set_promisc_mode(true);
    config.set_snap_len(4000);
    sniffer.assign(new Sniffer(interface,config));
    logger = &(Poco::Logger::get("TargetAgent.CnetworkPacketSnifferPlugin")); // inherits configuration from Target Agent
}

void CPacketSniffer::runActivity() {
    while (!_activity.isStopped())
    {
        sniffer->sniff_loop(make_sniffer_handler(this, &CPacketSniffer::handle));
    }
}

bool CPacketSniffer::handle(PDU& pdu) {

	Poco::Mutex::ScopedLock lock(mMsgQMutex);
	{
		if (payloadType.compare("SOMEIP") == 0)
		{
			if ( protocolType.compare("UDP") == 0)
				handlingSomeIPMessage<UDP>(pdu);
			else
				handlingSomeIPMessage<TCP>(pdu);
		}
		else if (payloadType.compare("WEBSOCKET")== 0)
		{

			if ( protocolType.compare("UDP") == 0)
				handlingSocketMessage<UDP>(pdu);
			else
				handlingSocketMessage<TCP>(pdu);
		}

	}
return true;
}

template <class T>
void CPacketSniffer::handlingSocketMessage (PDU  &pdu)
{
	const RawPDU&raw = pdu.rfind_pdu<RawPDU>();
	const IP &ip = pdu.rfind_pdu<IP>();
	const RawPDU::payload_type& payload = raw.payload();
	const T&protocolTypeDef = pdu.rfind_pdu<T>();


	if(payload.size() <= 0)
	{
		logger->error("invalid payload size");
		return;
	}
	char* buffer = decodeWebsockFrame(payload);
	std::stringstream payloadStream;
	payloadStream << buffer;
	if(payloadStream.rdbuf()->in_avail())
	{
		clearMessage();
		std::string wrappedMessage = fillInJsonMessage(protocolTypeDef, payloadStream);
		fillProtobuf(wrappedMessage, ip, protocolTypeDef, TargetAgent::Protocol::NetworkPacketSniffer::MESSAGE_SOURCE_WEBSOCK);
		dispatchMessage(&nwMsg);
	}
}


template <class T>
void CPacketSniffer::handlingSomeIPMessage (PDU& pdu)
{
	Poco::Mutex::ScopedLock lock(mMsgQMutex);
	{
		const RawPDU&raw = pdu.rfind_pdu<RawPDU>();
		const IP &ip = pdu.rfind_pdu<IP>();
		const RawPDU::payload_type& payload = raw.payload();

		if(payload.size() <= 0)
		{
			logger->error("invalid payload size");
			return;
		}
		const T&protocolTypeDef = pdu.rfind_pdu<T>();
		std::stringstream payloadStream;
		getSomeIPPayload(payload, payloadStream);

		if(payloadStream.rdbuf()->in_avail())
		{
			clearMessage();
			std::string wrappedMessage = fillInJsonMessage(protocolTypeDef, payloadStream);
			fillProtobuf(wrappedMessage, ip, protocolTypeDef, TargetAgent::Protocol::NetworkPacketSniffer::MESSAGE_SOURCE_SOMEIP);
			dispatchMessage(&nwMsg);
		}
	}
}

void CPacketSniffer::getSomeIPPayload(const RawPDU::payload_type& payload,  std::stringstream &payloadStream)
{
	for (std::vector<uint8_t>::const_iterator it = payload.begin(); it != payload.end(); ++it)
	{
		payloadStream<<*it;
	}
}


char* CPacketSniffer::decodeWebsockFrame(const RawPDU::payload_type& payload)
{
    bool isMasked = false;
    unsigned int offset = 0;
	#ifdef _WIN32
		static char buffer[2048];
	#else
		 char* buffer=NULL;
	#endif

    uint8_t* nonConstPayload = const_cast<uint8_t*>(&payload[0]);
    unsigned int payloadLength = payload.size();
    if (payload[offset] == DATA_FRAME_MASK)
    {
        unsigned char mask[MASK_SIZE];
        unsigned char secondPayloadByte = ((unsigned char)(nonConstPayload[++offset]));
        if ((secondPayloadByte & PAYLOAD_MASKBIT_MASK) == PAYLOAD_MASKBIT_MASK)
        {
            logger->warning("Frame is masked");
            isMasked = true;
        }
        payloadLength = ((unsigned char)(nonConstPayload[offset])) & PAYLOAD_LENGTH_MASK;
        logger->information(Poco::format("packet length %u offset %u", payloadLength, offset));
        if (payloadLength == PAYLOAD_SIZE_126)
        {
            int lenght_byte_1 = nonConstPayload[++offset];
            int lenght_byte_2 = nonConstPayload[++offset];
            payloadLength = (lenght_byte_1 << 8) + lenght_byte_2;
        }
        else
            if (payloadLength == PAYLOAD_SIZE_127)
            {
                return NULL;
                logger->error(Poco::format("packet length %u not handled", payloadLength));
            }

        if (isMasked)
        {
            mask[0] = nonConstPayload[++offset];
            mask[1] = nonConstPayload[++offset];
            mask[2] = nonConstPayload[++offset];
            mask[3] = nonConstPayload[++offset];

            for (unsigned int i = 0;i < payloadLength;i++)
                nonConstPayload[offset + i+1] ^= mask[i % MASK_SIZE];
        }

        ++offset;
    }
#ifdef _WIN32
	int rc = sprintf(buffer, "%.*s", payloadLength, nonConstPayload + offset);
#else
	int rc = asprintf(&buffer, "%.*s", payloadLength, nonConstPayload + offset);
#endif
    if (rc != payloadLength)
    {
        logger->warning("bytes written not equal to expected %u rc %d packet_length %u ", offset, rc, payloadLength);
    }

    return buffer;
}


void CPacketSniffer::clearMessage(void)
{
    nwMsg.Clear();
    nwMsg.clear_pdu_payload();
    nwMsg.clear_src_addr();
    nwMsg.clear_dst_addr();
}

template <class T>
std::string CPacketSniffer::fillInJsonMessage(const T& protocolT, std::stringstream & buffer)
{
    unsigned int pid = 0;
    char processName[CPacketSniffer::PROCESS_NAME_LENGTH];
    memset(processName, 0x00, CPacketSniffer::PROCESS_NAME_LENGTH);
	  std::stringstream payloadStream;
    payloadStream << "{\"websock\":{";
    payloadStream << "\"payload\":"<<buffer.str();
    payloadStream << ",\"metadata\": {  \"sender\":"<<"{  \"name\":"  << "\"" << processName << "\"" << "," << "\"pid\":" << pid << "},";
    pid = 0;
    memset(processName, 0x00, CPacketSniffer::PROCESS_NAME_LENGTH);

    payloadStream << "\"receiver\": { "<<"\"name\":" << "\"" << processName << "\"" << "," << "\"pid\":" << pid << "}";
    payloadStream << "}}}";
    std::string payloadFinal = payloadStream.str();
    std::replace(payloadFinal.begin(), payloadFinal.end(), '\n', ' ');
    return payloadFinal;
}

template <class T>
void CPacketSniffer::fillProtobuf(const std::string& payload, const IP& ip, const T& protocolT,TargetAgent::Protocol::NetworkPacketSniffer::MESSAGE_SOURCE typeOfPackt )
{
    nwMsg.set_source(typeOfPackt);
    nwMsg.set_pdu_payload(payload.c_str());
    std::cout<<"called here"<<payload<<std::endl;
    nwMsg.set_src_addr(ip.src_addr().to_string());
    nwMsg.set_dst_addr(ip.dst_addr().to_string());
    nwMsg.mutable_tcp()->set_src_port(protocolT.sport());
    nwMsg.mutable_tcp()->set_dst_port(protocolT.dport());
}

void CPacketSniffer::dispatchMessage( TargetAgent::Protocol::NetworkPacketSniffer::NetworkPacket*  nwMsg)
{
    unsigned char *protoSerialized =  new unsigned char[nwMsg->ByteSize()];

    if (nwMsg->SerializeToArray((void*)(protoSerialized), nwMsg->ByteSize()))
    {
        mSenderHandle->sendMessage(TargetAgent::Protocol::CommonDefinitions::MSG_TYPE_NETWORKPACKETSNIFFER_PLUGIN, nwMsg->ByteSize(), protoSerialized);
    }

	delete protoSerialized;
}

}
;
