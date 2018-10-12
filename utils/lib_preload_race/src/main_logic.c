/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "thread_storage.h"
#include "clock_times.h"
#include "libc_internals.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>


void* communicationFunc(void*);


static pid_t process_id = 0;
static pthread_t thread;
static int running = 0;


__attribute__ ((constructor)) void constructor(void)
{
    process_id = getpid();
    ct_initialize();

    running = 1;
    pthread_create(&thread, NULL, communicationFunc, NULL);
}


__attribute__ ((destructor)) void destructor(void)
{
    running = 0;
    pthread_join(thread, NULL);
}


static size_t send_data(int fd, char* send_buffer, const char* data)
{
    sprintf(send_buffer, "$%02x %s", (unsigned int)strlen(data), data);
    return send(fd, send_buffer, strlen(send_buffer), 0);
}

void* communicationFunc(void* p __attribute__((unused)))
{
    char buffer[256];
    char buffer_name[16] = {'\0'};
    char rec_buffer[256];
    char send_buffer[256];

    int socket_fd = 0;
    if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error: could not create socket\n");
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(7778);
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("Error: inet pton error\n");
    }

    int connected = 0;
    while(running)
    {
        if(!connected)
        {
            connected = (connect(socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) >= 0);
            if(!connected)
            {
                printf("Error: connect failed.\n");
                sleep(1);
                continue;
            }
        }

        int n = 0;
        do
        {
            rec_buffer[0] = '\0';
            n = read(socket_fd, rec_buffer, 1);
        }
        while((n >= 0) && (rec_buffer[0] != '$'));
        n = read(socket_fd, rec_buffer, 3);
        if(n < 0)
        {
            continue;
        }
        rec_buffer[2] = '\0';
        int length = 0;
        sscanf(rec_buffer, "%x", &length);
        n = read(socket_fd, rec_buffer, length);
        if(n < 0)
        {
            continue;
        }

        struct timespec monotime;
        clock_gettime(CLOCK_MONOTONIC, &monotime);
        unsigned int millisec = monotime.tv_nsec / 1000000ul;
        millisec += monotime.tv_sec * 1000u;

        unsigned long long base = ct_get_measure_base();

        struct ThreadStorage* it;
        for(it = ts_get_next(NULL); it != NULL; it = ts_get_next(it))
        {
            char* ptr = buffer;
            ptr += sprintf(ptr, "CC:%u PI:%u TI:%u", millisec,(unsigned int)process_id, (unsigned int)ts_get_sys_thread_id(it));

            pthread_getname_np(ts_get_pthread_id(it), buffer_name, sizeof(buffer_name));
            ptr += sprintf(ptr, " TN:\"%s\"", buffer_name);

            ptr += sprintf(ptr, " TP:%llu", (long long int)ts_get_memory_peak(it));

            ptr += sprintf(ptr, " TC:%llu", (long long int)ts_get_alloc_count(it));

            unsigned int thread_time = ct_calculate_percent(ct_measure_thread(it), base);
            ptr += sprintf(ptr, " TT:%u.0", thread_time);

            //ptr += sprintf(ptr, " TS:%d", ts_is_thread_alife(it));

            send_data(socket_fd, send_buffer, buffer);
        }

        unsigned int process_time = ct_calculate_percent(ct_measure_process(), base);
        char* ptr = buffer;
        ptr += sprintf(ptr, "CC:%u PI:%u PN:\"%s\" PT:%u.0", millisec, (unsigned int)process_id, __progname, process_time);

        send_data(socket_fd, send_buffer, buffer);
    }
    return NULL;
}
