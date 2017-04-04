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

#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include "procFsReader.h"

extern "C"
{

    static char gFileDataBuffer[8 * 1024];


    static bool readProcEntry(const char * filename)
    {
        int fd;
        bool result = true;
        int bytes = 0;

        fd = open(filename, O_RDONLY);

        if ((fd == -1)
            || ((bytes = read(fd, gFileDataBuffer, sizeof gFileDataBuffer - 1))
                < 0))
        {

            printf("error reading file %s file descriptor %d bytes %d", filename,
                   fd, bytes);
            result = false;
        }
        else
            gFileDataBuffer[bytes] = C_STRING_TERMINATOR;

        close(fd);
        return result;
    }

    bool readCpuInfo(sysCpuInfo* pCpuInfo)
    {
        bool result = false;

        if(readProcEntry(PROC_STAT_FILE))
        {
            sscanf(gFileDataBuffer, "cpu %llu %llu %llu %llu", &pCpuInfo->userModeTime, &pCpuInfo->niceUserModeTime, &pCpuInfo->kernelModeTime, &pCpuInfo->idleTime);
            pCpuInfo->userModeTime *= MSEC_PER_CLOCK;
            pCpuInfo->niceUserModeTime *= MSEC_PER_CLOCK;
            pCpuInfo->kernelModeTime *= MSEC_PER_CLOCK;
            pCpuInfo->idleTime *= MSEC_PER_CLOCK;
            result = true;
        }
        else
            printf("error reading file %s ",PROC_STAT_FILE);

        return result;
    }

    bool readMeminfo(sysMemInfo* pMemInfo)
    {
        bool result = false;

        struct sysinfo memInfo =
            {
                0
            };

        int status = sysinfo(&memInfo);

        if (0 == status)
        {

            pMemInfo->totalVirtualMem = memInfo.totalram;

            pMemInfo->totalVirtualMem += memInfo.totalswap;
            pMemInfo->totalVirtualMem *= memInfo.mem_unit;

            pMemInfo->virtualMemUsed = memInfo.totalram - memInfo.freeram;
            pMemInfo->virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
            pMemInfo->virtualMemUsed *= memInfo.mem_unit;

            pMemInfo->totalPhysMem = memInfo.totalram;
            pMemInfo->totalPhysMem *= memInfo.mem_unit;

            pMemInfo->physMemUsed = memInfo.totalram - memInfo.freeram;
            pMemInfo->physMemUsed *= memInfo.mem_unit;
            result = true;
        }
        return result;
    }

    bool readProcFs(descriptor* entryInfo, sstat* statInfo, sstatm* statmInfo)
    {

        char filename[PATH_MAX];
        bool result = true;

        /* skip until either a successful read or
         *  until last entry in the folder is reached
         * */
        while ((NULL != entryInfo->dir) && (NULL != (entryInfo->dp = readdir(entryInfo->dir)))
               && (0 == isdigit(entryInfo->dp->d_name[0])))
        {
            ;
        }

        if (NULL != entryInfo->dp)
        {
            /*a valid entry is found*/

            if (entryInfo->pid > 0)
            {
                snprintf(filename, PATH_MAX, "/proc/%d/task/%s/stat",
                         entryInfo->pid, entryInfo->dp->d_name);
            }
            else
                snprintf(filename, PATH_MAX, "/proc/%s/%s", entryInfo->dp->d_name,
                         "stat");

            result = result && readProcEntry(filename);

            sscanf(gFileDataBuffer,
                   "%d  (%[^)]) %c %d %d %d %d %d %lu %lu %lu %lu %lu %lu %lu %d %d",
                   &statInfo->pid, (statInfo->fullName), &statInfo->state,
                   &statInfo->ppid, &statInfo->pgrp, &statInfo->session,
                   &statInfo->tty, &statInfo->tpgid, &statInfo->flags,
                   &statInfo->minflt, &statInfo->majflt, &statInfo->cminflt,
                   &statInfo->cmajflt, &statInfo->new_utime, &statInfo->new_stime,
                   &statInfo->cutime, &statInfo->cstime);

            if (entryInfo->pid > 0)
            {
                snprintf(filename, PATH_MAX, "/proc/%d/task/%s/statm",
                         entryInfo->pid, entryInfo->dp->d_name);
            }
            else
                snprintf(filename, PATH_MAX, "/proc/%s/%s", entryInfo->dp->d_name,
                         "statm");
            /*todo: change implementation so pagesize is computed only once: changes in protocol definition*/
            statmInfo->PageSize = sysconf(_SC_PAGESIZE);
            statInfo->new_stime *= MSEC_PER_CLOCK;
            statInfo->new_utime *= MSEC_PER_CLOCK;


            result = result && readProcEntry(filename);

            sscanf(gFileDataBuffer, "%d %d %d %d %d %d %d", &statmInfo->Size,
                   &statmInfo->Rss, &statmInfo->Shared, &statmInfo->Text,
                   &statmInfo->Data, &statmInfo->Lib, &statmInfo->Dirty);
            statmInfo->Rss *= statmInfo->PageSize;

        }
        else
            result = false;

        return result;
    }
    bool getProcIDbyName(const char* procName, unsigned int pidsArrayMaxSize, int* pidsArray,
                         unsigned int* pidsArraySize)
    {

        bool result = false;
        sstat statInfo = { 0 };
        DIR* dir = opendir(PROC_FS_ROOT);
        *pidsArraySize = 0;

        if (dir)
        {
            struct dirent* de = 0;

            while ((de = readdir(dir)) != 0)
            {
                if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
                    continue;

                int pid = -1;
                int res = sscanf(de->d_name, "%d", &pid);

                if (res == 1)
                {
                    char statFile[NAME_MAX] = { 0 };
                    sprintf(statFile, "%s/%d/stat", PROC_FS_ROOT, pid);

                    int fd = open(statFile, O_RDONLY);

                    if (read(fd, gFileDataBuffer, sizeof gFileDataBuffer - 1) > 0)
                    {
                        sscanf(gFileDataBuffer,
                               "%d  (%[^)]) %c %d %d %d %d %d %lu %lu %lu %lu %lu %lu %lu %d %d",
                               &statInfo.pid, (statInfo.fullName), &statInfo.state,
                               &statInfo.ppid, &statInfo.pgrp, &statInfo.session,
                               &statInfo.tty, &statInfo.tpgid, &statInfo.flags,
                               &statInfo.minflt, &statInfo.majflt,
                               &statInfo.cminflt, &statInfo.cmajflt,
                               &statInfo.new_utime, &statInfo.new_stime,
                               &statInfo.cutime, &statInfo.cstime);

                        if ((0 == strcmp(statInfo.fullName, procName)))
                        {
                            pidsArray[*pidsArraySize] = pid;
                            result =
                                (pidsArrayMaxSize > (++(*pidsArraySize))) ?
                                true : false;
                        }
                    }

                    close(fd);
                }
            }

            closedir(dir);
        }

        return result;
    }

}
;
