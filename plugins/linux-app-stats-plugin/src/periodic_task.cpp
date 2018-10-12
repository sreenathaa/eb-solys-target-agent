/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

/*
 * periodic_task.cpp
 *
 *  Created on: 26.11.2015
 *      Author: vagrant
 */

#include <sys/timerfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

#include "periodic_task.hpp"

#define PRETTY_PRINT_ERRNO(FKT) mLogger->error(Poco::format("%s failed with the following error: %s",std::string(#FKT),std::string(strerror(errno))))

namespace LinuxAppStatsPlugin {

CPeriodicTaskInfo::CPeriodicTaskInfo() :
        mTimer_fd(), mWakeupsMissed(), mLogger(
    &(Poco::Logger::get("TargetAgent.CThreadStatsPlugin"))) {
}

bool CPeriodicTaskInfo::configureTimer(unsigned int mSamplingRate) {
    int ret;
    bool retVal = true;
    unsigned long int ns;
    unsigned long int sec;
    int fd;
    struct itimerspec itval;

#ifdef _POSIX_THREADS

    if (0 != pthread_setname_np(pthread_self(), "PAppStatsTask"))
    {
        mLogger->error("Failed to set thread name for PAppStatsTask");
    }
#endif

    fd = timerfd_create(CLOCK_MONOTONIC, 0);
    mWakeupsMissed = 0;
    mTimer_fd = fd;
    if (fd == -1)
    {
        PRETTY_PRINT_ERRNO(timerfd_create);
        retVal = false;
    }

    sec = mSamplingRate / 1000;
    ns = (mSamplingRate - (sec * 1000)) * 1000000;

    itval.it_interval.tv_sec = sec;
    itval.it_interval.tv_nsec = ns;
    itval.it_value.tv_sec = sec;
    itval.it_value.tv_nsec = ns;
    ret = timerfd_settime(fd, 0, &itval, NULL);

    if (ret == -1)
    {
        PRETTY_PRINT_ERRNO(timerfd_settime);
        retVal = false;
    }

    return retVal;

}
void CPeriodicTaskInfo::waitTimerTrigger(void) {
    int ret = 0;
    unsigned long long missedIteration;

    ret = read(mTimer_fd, &missedIteration, sizeof(missedIteration));
    if (ret == -1)
    {
        PRETTY_PRINT_ERRNO(read);
    }

    if (missedIteration > 0)
    {
        mWakeupsMissed += (missedIteration - 1);
        mLogger->warning(Poco::format("Missed %Lu", mWakeupsMissed));
    }
}

void CPeriodicTaskInfo::wakeupEvent() {
    fcntl(mTimer_fd, F_SETFL, fcntl(mTimer_fd, F_GETFL, 0) | O_NONBLOCK);
}
}
