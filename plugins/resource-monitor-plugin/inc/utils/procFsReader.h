/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

/*! \file procFsReader.h
 \brief Parser for the proc stat structure
 plain C module parsing the proc stat filesystem
 
 known limitations:
 *system wise memory gathering still relies on C API call instead of directly accessing the proc filesystem /proc/meminfo
 */

#ifndef PROCFSREADER_H_
#define PROCFSREADER_H_

#include<stdio.h>

#if not defined(__QNXNTO__)
#include <linux/limits.h>
#else
#include <limits.h>
#endif

#include <dirent.h>

extern "C" {

    /*! \def PROC_FS_ROOT
     \brief root of proc filesystem
     -
     */
#define PROC_FS_ROOT    "/proc"
#define PROC_FS_TASK    "/task"
#define PROC_STAT_FILE    "/proc/stat"

    /*
     ! \def TASK_ROOT(PID)
     \brief computes the path of the task root directory
     Details.
     
     
     #define STR_HELPER(x) #x
     #define STR(x) STR_HELPER(x)
     
     */
#define UPDATE_TASK_ROOT(taskRootFolder,PID) sprintf (taskRootFolder, "/proc/%d/task", PID)

    /*! \def PROC_MEMINFO
     \brief memory info path
     -
     */
#define PROC_MEMINFO   "/proc/meminfo"

    /*! \def C_STRING_TERMINATOR
     \brief use macro due to code readability concerns
     -
     */
#define C_STRING_TERMINATOR  '\0'

    /*! \def PIDS_MAX
     \Maximum occurrences of a process
     -
     */
#define PIDS_MAX  10U

#define MSEC_PER_CLOCK  1000U / sysconf(_SC_CLK_TCK)

    /*! \struct  sstat container
     \brief see /proc/[pid]/stat
     -
     */
    typedef struct t_sstat {
        char fullName[NAME_MAX];
        long unsigned int new_utime;
        long unsigned int new_stime;
        char state;
        int pid;
        int ppid;
        int pgrp;
        int session;
        int tty;
        int tpgid;
        long unsigned int flags;
        long unsigned int minflt;
        long unsigned int majflt;
        long unsigned int cminflt;
        long unsigned int cmajflt;
        int cutime;
        int cstime;
    }
    sstat;

    /*! \struct  sstatm container
     \brief see /proc/[pid]/statm
     -
     */
    typedef struct t_sstatm {
        int PageSize;
        int Rss;
        int Text;
        int Data;
        int Lib;
        int Shared;
        int Size;
        int Dirty;
    }
    sstatm;

    /*! \struct  unique descriptor of iterator
     \brief used to iterate inside proc stat
     -
     */
    typedef struct t_descriptor {
        struct dirent *dp;
        DIR* dir;
        int pid;
    }
    descriptor;

    /*! \struct  internal container for the result of sysinfo
     \brief -
     -
     */
    typedef struct t_sysMemInfo {
        long long totalVirtualMem;
        long long virtualMemUsed;
        long long totalPhysMem;
        long long physMemUsed;
    }
    sysMemInfo;

    /*! \struct  internal container for the result of reading /proc/stat
     \brief -
     -
     */
    typedef struct t_sysCpuInfo {
        unsigned long long userModeTime;
        unsigned long long niceUserModeTime;
        unsigned long long kernelModeTime;
        unsigned long long idleTime;
    }
    sysCpuInfo;

    /*! \fn bool readMeminfo(sysMemInfo* pMemInfo)
     \brief gathers the memory related information and populates and internal structure
     \param pMemInfo pointer to structure which will contain the result; only to be interpreted if return value is true
     */
    bool readMeminfo(sysMemInfo* pMemInfo);

    /*! \fn bool readCpuInfo(sysCpuInfo* pCpuInfo)
     \brief gathers the cpu related information and populates and internal structure
     \param pCpuInfo pointer to structure which will contain the result; only to be interpreted if return value is true
     */
    bool readCpuInfo(sysCpuInfo* pCpuInfo);

    /*! \fn size_t write(int fd,const char *buf, size_t count)
     \brief gets one entry of the proc fs, based on the descriptor and populates the output structure
     \param entryInfo descriptor of the current entry in the proc stat structure
     \param statInfo structure to be populated with the result for cpu information
     \param statmInfo structure to be populated with the result for memory information
     */
    bool readProcFs(descriptor* entryInfo, sstat* statInfo, sstatm* statmInfo);

    /*! \fn bool getProcIDbyName(const char* name, int* id);
     \brief retrieve the pids corresponding to all instances of a process based on the process name
     \param input  procName: the process name
     \param input  pidsArrayMaxSize: the maximum number of items that can be written in the array
     \param output pidsArray: the list of process IDs
     \param output pidsArraySize the number of instances that the process had
     */
    bool getProcIDbyName(const char* procName, unsigned int pidsArrayMaxSize,
                         int* pidsArray, unsigned int* pidsArraySize);

}

#endif /* PROCFSREADER_H_ */
