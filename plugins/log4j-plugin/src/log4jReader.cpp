/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/



#include <iostream>
#include <fstream>
#include <numeric>
#include <log4jReader.hpp>

#include "Poco/DateTimeParser.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/DateTime.h"
#include "Poco/Timestamp.h"
#include "Poco/Exception.h"
#include "Poco/StringTokenizer.h"
#include "Poco/Thread.h"

#include "target_agent_prot_log4j_plugin.pb.h"
#include "TargetAgentPluginInterface.h"



using Poco::DateTime;
using Poco::DateTimeFormat;
using Poco::DateTimeParser;
using Poco::Timestamp;
using Poco::SyntaxException;
using Poco::DateTimeFormatter;
using Poco::Thread;


namespace TargetAgentlog4jPlugin {

log4jReader::log4jReader(
    const TargetAgent::PluginInterface::CMessageDispatcher * const msgSenderHDL,const TargetAgent::PluginInterface::CTimestampProvider * const  timestampProvider,
    const std::string logFileName) :
        mMsgSenderHDL(msgSenderHDL),mTimestampProvider(timestampProvider), _activity(this, &log4jReader::runActivity), mLogFileName(
            logFileName), logger(
        &(Poco::Logger::get("TargetAgent.Clog4jPlugin"))), logMessage(),lastValidDate(),lastValidLogLevel() {

}

void log4jReader::runActivity() {
    std::ifstream logFileStream(mLogFileName.c_str());

    if (!logFileStream.is_open())
    {
        logger->error("failed to open file %s",mLogFileName);
        return;
    }
    std::ios::streampos gpos = logFileStream.tellg();

    while (!_activity.isStopped())
    {
        std::string logLine;

        while (std::getline(logFileStream, logLine))
        {
            gpos = logFileStream.tellg();
            std::string payload;
            unsigned long long timestamp=0;
            tryParseDateTime(logLine,&payload,&timestamp);

            logMessage.set_trace(payload);
            logMessage.set_loglevel(lastValidLogLevel);
            dispatchMessage(timestamp);
            logMessage.clear_trace();
        }
        Thread::sleep(read_timeout*1000);

        resetEof(logFileStream);
        logFileStream.seekg(gpos);
    }
}


void log4jReader::tryParseDateTime(const std::string& logLine, std::string* payload,unsigned long long* ts) {

    Poco::StringTokenizer tokens(logLine, " ");
    Poco::StringTokenizer::Iterator msgIterator = tokens.begin();
    std::string date;
    std::string time;

    if(tokens.count()>=2)
    {

        date = *(tokens.begin());
        time = *(++tokens.begin());

        *ts = extractDateTime(date+time);
    }

    if(*ts != 0)
    {
        lastValidDate = *msgIterator+*(++msgIterator);
        lastValidLogLevel = *(++msgIterator);
        ++msgIterator;
    }
    else if(!lastValidDate.empty())
    {
        *ts = extractDateTime(lastValidDate);
        if(*ts==0)
        {
            *ts = mTimestampProvider->createTimestamp();
            lastValidLogLevel = "Invalid_Log4j";
        }
    }


    for(Poco::StringTokenizer::Iterator it = msgIterator; it != tokens.end(); it++)
        *payload+=*it+" ";

}

unsigned long long log4jReader::extractDateTime(const std::string& dateTime)
{
    unsigned long long ts = 0;
    int timezone = 0;
    try
    {
        DateTime dt = DateTimeParser::parse(DateTimeFormat::ISO8601_FRAC_FORMAT, dateTime, timezone);
        dt.makeUTC(timezone);
        ts = mTimestampProvider->createTimestampFromDateTime(DateTimeFormatter::format(dt, DateTimeFormat::ISO8601_FRAC_FORMAT));
    }
    catch(Poco::SyntaxException& e)
    {
        logger->information("cannot parse date");
    }
    return ts;
}

void log4jReader::dispatchMessage(unsigned long long ts) {

    unsigned char *payload = new unsigned char[logMessage.ByteSize()];

    if (logMessage.SerializeToArray((void*) payload, logMessage.ByteSize()))
    {
        mMsgSenderHDL->sendMessage(
            TargetAgent::Protocol::CommonDefinitions::MSG_TYPE_LOG4J_PLUGIN,
            logMessage.ByteSize(), payload,ts);

    }
    delete[] payload;
}

void log4jReader::resetEof(std::ifstream& ifs) {
    if (ifs.eof())
        ifs.clear();
}
log4jReader::~log4jReader() {
    // TODO Auto-generated destructor stub
}
}
