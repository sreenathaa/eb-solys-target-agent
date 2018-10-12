/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/


#ifndef PLUGINS_LINUX_APP_STATS_PLUGIN_INC_PERIODIC_TASK_HPP_
#define PLUGINS_LINUX_APP_STATS_PLUGIN_INC_PERIODIC_TASK_HPP_

#include "Poco/Logger.h"

namespace LinuxAppStatsPlugin {

class CPeriodicTaskInfo {
public:
    CPeriodicTaskInfo();
    bool configureTimer(unsigned int mSamplingRate);
    void waitTimerTrigger(void);
    void wakeupEvent();
public:
    int mTimer_fd;
    unsigned long long mWakeupsMissed;
    Poco::Logger* mLogger;
};

}

#endif /* PLUGINS_LINUX_APP_STATS_PLUGIN_INC_PERIODIC_TASK_HPP_ */
