/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, 
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef RESOURCE_MONITOR_UTILS_H_
#define RESOURCE_MONITOR_UTILS_H_

#include <stdlib.h>
#include <string>
#include <windows.h>


namespace ResourceMonitor
{

class CProcMetrics {
public:
    CProcMetrics();
    ~CProcMetrics(){
    }

    void setSysCPU(const LONGLONG& sysCPU){
        mSysCPU = sysCPU/10000;
    }
    LONGLONG getSysCPU(void) const{
        return mSysCPU;
    }

    void setUserCPU(const LONGLONG& userCPU){
        mUserCPU = userCPU/10000;
    }
    LONGLONG getUserCPU(void) const{
        return mUserCPU;
    }


    void setPid(const DWORD& pid){
        mPid = pid;
    }
    DWORD getPid(void) const {
        return mPid;
    }

    void setName(const std::string name){
        mName = name;
    }
    std::string getName(void) const {
        return mName;
    }

    void setWorkingSetSize(const DWORD& wss){
        mWss = wss;
    }
    DWORD getWorkingSetSize(void) const {
        return mWss;
    }

    void setPrivateMemory(const SIZE_T& privateMem){
        mPrivateMem = privateMem;
    }
    SIZE_T getPrivateMemory(void) const {
        return mPrivateMem;
    }

private:
    LONGLONG  mSysCPU;
    LONGLONG  mUserCPU;
    DWORD mPid;
    std::string mName;
    DWORD mWss;
    SIZE_T mPrivateMem;
    CProcMetrics(const CProcMetrics& c);
    CProcMetrics& operator=(const CProcMetrics& rhs);
};

class CSystemInfo {
public:
    CSystemInfo();
    ~CSystemInfo();
    void update();
    DWORDLONG  getTotalPhys() const;
    DWORDLONG  getAvailPhys() const;
    DWORDLONG  getTotalVirtual() const;
    DWORDLONG  getAvailVirtual() const;

private:
    DWORDLONG  dwTotalPhys;
    DWORDLONG  dwAvailPhys;

    DWORDLONG  dwTotalVirtual;
    DWORDLONG  dwAvailVirtual;

    CSystemInfo(const CSystemInfo& c);
    CSystemInfo& operator=(const CSystemInfo& rhs);
};

};

#endif /* RESOURCE_MONITOR_UTILS_H_ */
