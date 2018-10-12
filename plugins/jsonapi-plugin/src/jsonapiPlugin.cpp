/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/


#include <iostream>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <Poco/Thread.h>
#include "jsonapiPlugin.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

extern "C" void* createPluginInstance(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider) {
	return new TargetAgentjsonapiPlugin::CjsonapiPlugin(senderHandle, tsProvider);
}

namespace TargetAgentjsonapiPlugin {

using namespace TargetAgent;
using namespace TargetAgent::Protocol;

CjsonapiPlugin::CjsonapiPlugin(const CMessageDispatcher* const senderHandle, const CTimestampProvider* tsProvider) :mMsgSenderHDL(senderHandle),mTimestampProvider(tsProvider),
		messageTypeSocketReader(
				Protocol::CommonDefinitions::MSG_TYPE_JSONAPI_PLUGIN), logger(
				0), activity(this, &CjsonapiPlugin::run) {
	logger = &(Poco::Logger::get("TargetAgent.CjsonapiPlugin")); // inherits configuration from Target Agent
}

CjsonapiPlugin::~CjsonapiPlugin() {
}

bool CjsonapiPlugin::setConfig(
		const std::map<std::string, std::string>& pluginConfiguration) {
    logger->warning("setConfig");
	return true;
}

void CjsonapiPlugin::onMessageReceived(int payloadLength,
		const unsigned char* payloadBuffer) {

}

Protocol::CommonDefinitions::MessageType CjsonapiPlugin::MessageType() {
	return messageTypeSocketReader;
}

void CjsonapiPlugin::onConnectionEstablished() {
}

void CjsonapiPlugin::onConnectionLost() {
	logger->warning(
			"Connection Lost: trigger metadata dispatch, currently none");
}

bool CjsonapiPlugin::startPlugin() {
    logger->warning("startPlugin");
	int status = mkfifo("/tmp/jsonapi", S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
	activity.start();
	return true;
}

void CjsonapiPlugin::run(){
	fd = open("/tmp/jsonapi", O_RDWR);
	if (fd == -1)
    	{
		cout << "Couldn't open pipe" << endl; 
        	return;
    	}
	
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	int msgPos = 0;
	char msg[4096];
	char buf[1024];
	while(!activity.isStopped())
	{
		int numChar = read(fd, &buf, sizeof(char)*1024);
		if(numChar > 0)
		{
			int bufPos = 0;
			for(int i = 0; i < numChar; i++)
			{
				if(buf[i] == '\n')
				{
					memcpy( msg + msgPos, buf + bufPos, i - bufPos );
					msg[msgPos+i] = '\0';
					printf("Send message: %s\n", msg);
					JsonAPIMsg taMsg;
					taMsg.set_content(msg);
					dispatch(taMsg);
					msgPos = 0;
					bufPos = i + 1;
				}
			}
		
			int remainingChars = numChar - bufPos; 
			if((msgPos + remainingChars) > 4096)
			{
				logger->warn("Must cut message, because it is longer than 4096 chars.");
				memcpy( msg + msgPos, buf + bufPos, 4096 - msgPos );
				JsonAPIMsg taMsg;
			       	taMsg.set_content(msg);
				dispatch(taMsg);
				msgPos = 0;
			}
			else
			{
				memcpy( msg + msgPos, buf + bufPos, remainingChars );
				msgPos += remainingChars;
			}

		}
		else
		{
			Thread::sleep(50);
		}
	}
}

void CjsonapiPlugin::dispatch(JsonAPIMsg msg) {

                unsigned char payload[msg.ByteSize()];

                if (msg.SerializeToArray((void*) &payload, msg.ByteSize())) {
                        mMsgSenderHDL->sendMessage(CommonDefinitions::MSG_TYPE_JSONAPI_PLUGIN,
                                        msg.ByteSize(), payload);

                }
        }


bool CjsonapiPlugin::stopPlugin() {
    logger->warning("stopPlugin");
	activity.stop();
	activity.wait();
	unlink("/tmp/jsonapi");
	return true;
}

}
;
