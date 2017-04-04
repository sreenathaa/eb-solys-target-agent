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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "procFsReader.h"
#include "Poco/Environment.h"
#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>
#include <devctl.h>
#include <sys/procfs.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/mman.h>

extern "C" {

    static bool getProcessName(const int& fd, const int& pid, char * name);
    static bool getCpuInfo(const int& fd, const int& pid, procfs_info* cpuInfo);
    static bool getProcMemoryInfo(const int fd, unsigned long long  & memUsage);

    bool readCpuInfo(sysCpuInfo* pCpuInfo) {
        bool result = false;

        pCpuInfo->userModeTime = 0;
        pCpuInfo->niceUserModeTime = 0;
        pCpuInfo->kernelModeTime = 0;
        pCpuInfo->idleTime = 0;
        result = true;

        return result;
    }

    static void getTotalMemory(unsigned long long int& totalMemory){
        size_t asIt = 0;
        struct asinfo_entry *adressSpaceEntries = SYSPAGE_ENTRY(asinfo);
        size_t nrOfAsEntries = SYSPAGE_ENTRY_SIZE(asinfo)
                               / sizeof(struct asinfo_entry);
        char *strings = SYSPAGE_ENTRY(strings)->data;

        for (asIt = 0; asIt < nrOfAsEntries; asIt++)
        {
            struct asinfo_entry *adressSpaceEntry = &adressSpaceEntries[asIt];
            if (strcmp(strings + adressSpaceEntry->name, "ram") == 0)
            {
                totalMemory += adressSpaceEntry->end - adressSpaceEntry->start + 1;
            }
        }

    }

    bool readMeminfo(sysMemInfo* pMemInfo) {
        bool result = false;


        struct stat statbuf = {
                                  0 };
        paddr_t freeMemory = 0;

        unsigned long long int totalMemory = 0;
        getTotalMemory(totalMemory);

        stat("/proc", &statbuf);
        freeMemory = (paddr_t) statbuf.st_size;

        pMemInfo->totalVirtualMem = totalMemory;

        pMemInfo->virtualMemUsed = totalMemory - freeMemory;

        pMemInfo->totalPhysMem = totalMemory;

        pMemInfo->physMemUsed = totalMemory - freeMemory;

        result = true;
        return result;
    }

    bool readProcFs(descriptor* entryInfo, sstat* statInfo, sstatm* statmInfo) {

        bool result = true;

        /* skip until either a successful read or
         *  until last entry in the folder is reached
         * */
        while ((NULL != entryInfo->dir)
               && (NULL != (entryInfo->dp = readdir(entryInfo->dir)))
               && (0 == isdigit(entryInfo->dp->d_name[0])))
        {
            ;
        }

        if (NULL != entryInfo->dp)
        {
            /*a valid entry is found*/

            statInfo->pid = atoi(entryInfo->dp->d_name);

            procfs_info cpuUsage = { 0 };
            unsigned long long summedUpMem = 0;
            char addressSpacePath[PATH_MAX];
            char processName[PATH_MAX];
            int fd = -1;

            sprintf(addressSpacePath, "/proc/%d/as", statInfo->pid);

            if ((fd = open(addressSpacePath, O_RDONLY)) == -1)
            {
                printf("error opening file %s\n", addressSpacePath);
                result = false;
            }
            else
            {

                if (getProcessName(fd, statInfo->pid, processName)
                    && getCpuInfo(fd, statInfo->pid, &cpuUsage)
                    && getProcMemoryInfo(fd, summedUpMem))
                {
                    statInfo->new_stime = cpuUsage.stime;
                    statInfo->new_utime = cpuUsage.cstime;
                    statInfo->new_stime /= 1000 * 1000;
                    statInfo->new_utime /= 1000 * 1000;

                    statmInfo->PageSize = sysconf(_SC_PAGESIZE);
                    statmInfo->Rss = summedUpMem;

                    strncpy(statInfo->fullName, processName, NAME_MAX);
                }
                close(fd);
            }

        }
        else
        {
            result = false;
        }

        return result;
    }

    static bool getProcessName(const int& fd, const int& pid, char * name) {
        bool retVal = true;
        static struct {
            procfs_debuginfo info;
            char buff[PATH_MAX];
        }
        nameInfo;

        if (devctl(fd, DCMD_PROC_MAPDEBUG_BASE, &nameInfo, sizeof(nameInfo), 0)
            != EOK)
        {
            printf("error reading file name info for pid %d\n", pid);
            strcpy(name, "(n/a)");

        }
        else
            strcpy(name, nameInfo.info.path);

        return retVal;
    }

    static bool getCpuInfo(const int& fd, const int& pid, procfs_info* cpuInfo) {
        bool retVal = false;
        if (devctl(fd, DCMD_PROC_INFO, cpuInfo, sizeof(*cpuInfo), NULL) != EOK)
        {
            retVal = false;
            printf("error requesting cpu related information for %d\n", pid);
        }
        else
        {
            retVal = true;
        }
        return retVal;
    }

    static bool getProcMemoryInfo(const int fd, unsigned long long  & memUsage) {

        memUsage = 0;
        procfs_mapinfo * mapinfos = NULL;
        static int num_mapinfos = 0;
        procfs_mapinfo * mapinfo_p = NULL;
        bool retVal = false;

        int nrOfMapInfos, mapInfosIt;

        if (devctl(fd, DCMD_PROC_MAPINFO, NULL, 0, &nrOfMapInfos) == EOK)
        {

            if ((mapinfos = (procfs_mapinfo *) malloc(
                                nrOfMapInfos * sizeof(procfs_mapinfo))) != NULL)
            {

                num_mapinfos = nrOfMapInfos;
                mapinfo_p = mapinfos;

                if (devctl(fd, DCMD_PROC_MAPINFO, mapinfo_p,
                           nrOfMapInfos * sizeof(procfs_mapinfo), &nrOfMapInfos)
                    == EOK)
                {

                    if (nrOfMapInfos > num_mapinfos)
                    {
                        nrOfMapInfos = num_mapinfos;
                    }

                    for (mapinfo_p = mapinfos, mapInfosIt = 0;
                         mapInfosIt < nrOfMapInfos; mapInfosIt++, mapinfo_p++)
                    {

                        if ((mapinfo_p->flags
                             & (MAP_ANON | MAP_STACK | MAP_ELF | MAP_TYPE))
                            == (MAP_ANON | MAP_PRIVATE))
                            memUsage += mapinfo_p->size;
                        if (mapinfo_p->flags & MAP_STACK)
                            memUsage += mapinfo_p->size;

                    }
                    retVal = true;

                }
                free(mapinfos);
            }
        }

        return retVal;
    }

}
;
