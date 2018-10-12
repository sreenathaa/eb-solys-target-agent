/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>


static size_t send_data(int fd, const char* data)
{
    static char buffer[256];
    sprintf(buffer, "$%02x %s", (unsigned int)strlen(data), data);
    return write(fd, buffer, strlen(buffer));
}


void* send_func(void* param)
{
    int n = 0;
    while(n >= 0)
    {
        printf("Sending measure request.\n");
        n = send_data((size_t)param, "Reserved:\"for future request flags\"");
        usleep(100000); //100 msec
    }
    return NULL;
}


void* rec_func(void* param)
{
    size_t fd = (size_t)param;
    char rec_buffer[256];
    int n = 0;
    while(n >= 0)
    {
        do
        {
            rec_buffer[0] = '\0';
            n = read(fd, rec_buffer, 1);
        }
        while((n >= 0) && (rec_buffer[0] != '$'));
        n = read(fd, rec_buffer, 3);
        if(n < 0)
        {
            continue;
        }
        rec_buffer[2] = '\0';
        int length = 0;
        sscanf(rec_buffer, "%x", &length);
        n = read(fd, rec_buffer, length);
        rec_buffer[n] = '\0';
        if(n < 0)
        {
            continue;
        }
        printf("Received: %s\n", rec_buffer);
    }
    return NULL;
}


int main(int argc __attribute__((unused)), const char* argv[] __attribute__((unused)))
{
    int socket_fd = 0;
    if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error: could not create socket\n");
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(7778);
    if(bind(socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Error: on binding\n");
        return -1;
    }

    listen(socket_fd, 5);

    while(1)
    {
        struct sockaddr_in cli_addr;
        socklen_t cl_addr_len = sizeof(cli_addr);
        int new_sock_fd = accept(socket_fd, (struct sockaddr*)&cli_addr, &cl_addr_len);
        if(new_sock_fd < 0)
        {
            printf("Error: on accept\n");
        }
        else
        {
            pthread_t thread;
            pthread_create(&thread, NULL, send_func, (void*)(size_t)new_sock_fd);
            pthread_create(&thread, NULL, rec_func, (void*)(size_t)new_sock_fd);
        }
    }
    return 0;
}
