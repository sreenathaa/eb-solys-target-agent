/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "thread_storage.h"
#include "libc_internals.h"

#include <sys/syscall.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>


struct ThreadStorage
{
    volatile size_t alloc_count;
    volatile size_t mem_current;
    volatile size_t count_overall;
    size_t mem_peak;
    unsigned mem_peak_needs_reset;

    clockid_t cpu_clock_id;
    struct timespec last_clock;

    pthread_t pthread_id;
    pid_t sys_thread_id;
    void* next;
    int thread_alife;
};


static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct ThreadStorage* g_thread_storage = 0;


struct ThreadStorage* ts_add_thread()
{
    struct ThreadStorage* result = (struct ThreadStorage*)__libc_malloc(sizeof(struct ThreadStorage));
    result->pthread_id = pthread_self();
    result->sys_thread_id = syscall(SYS_gettid);
    result->alloc_count = 0;
    result->mem_current = 0;
    result->count_overall = 0;
    result->mem_peak = 0;
    result->mem_peak_needs_reset = 0;
    pthread_getcpuclockid(result->pthread_id, &result->cpu_clock_id);
    result->last_clock.tv_sec = 0;
    result->last_clock.tv_nsec = 0;
    result->thread_alife = 1;

    pthread_mutex_lock(&g_mutex);
    result->next = g_thread_storage;
    g_thread_storage = result;
    pthread_mutex_unlock(&g_mutex);

    return result;
}


struct ThreadStorage* ts_get_next(struct ThreadStorage* current)
{
    struct ThreadStorage* next = g_thread_storage;
    if(current)
    {
        next = current->next;
        if(!current->thread_alife && (current->alloc_count == 0))
        {
            pthread_mutex_lock(&g_mutex);
            struct ThreadStorage** ptrPtr;
            for(ptrPtr = &g_thread_storage; *ptrPtr != 0; ptrPtr = (struct ThreadStorage**)&((*ptrPtr)->next))
            {
                if(*ptrPtr == current)
                {
                    struct ThreadStorage* next = (*ptrPtr)->next;
                    __libc_free(*ptrPtr);
                    *ptrPtr = next;
                }
            }
            pthread_mutex_unlock(&g_mutex);
        }
    }
    if(next)
    {
        next->thread_alife = (pthread_kill(next->pthread_id, 0) == 0);
    }
    return next;
}


clockid_t ts_get_clock_id(struct ThreadStorage* ts)
{
    return ts->cpu_clock_id;
}


struct timespec ts_get_clock(struct ThreadStorage* ts)
{
    return ts->last_clock;
}


void ts_set_clock(struct ThreadStorage* ts, struct timespec time_spec)
{
    ts->last_clock = time_spec;
}


pthread_t ts_get_pthread_id(struct ThreadStorage* ts)
{
    return ts->pthread_id;
}


pid_t ts_get_sys_thread_id(struct ThreadStorage* ts)
{
    return ts->sys_thread_id;
}


void ts_count_allocate(struct ThreadStorage* ts, size_t size)
{
    __sync_fetch_and_add(&ts->alloc_count, 1);
    size_t count = __sync_add_and_fetch(&ts->mem_current, size);
    if(ts->mem_peak_needs_reset || count > ts->mem_peak)
    {
        ts->mem_peak_needs_reset = 0;
        ts->mem_peak = count;
    }
    __sync_fetch_and_add(&ts->count_overall, 1);
}


void ts_count_deallocate(struct ThreadStorage* ts, size_t size)
{
    __sync_fetch_and_sub(&ts->alloc_count, 1);
    __sync_fetch_and_sub(&ts->mem_current, size);
}


size_t ts_get_alloc_count(struct ThreadStorage* ts)
{
    size_t count = ts->count_overall;
    __sync_fetch_and_sub(&ts->count_overall, count);
    return count;
}


size_t ts_get_memory_peak(struct ThreadStorage* ts)
{
    size_t size = ts->mem_peak;
    ts->mem_peak_needs_reset = 1;
    return size;
}

int ts_is_thread_alife(struct ThreadStorage* ts)
{
    return ts->thread_alife;
}
