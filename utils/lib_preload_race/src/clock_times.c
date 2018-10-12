/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "clock_times.h"

#include <time.h>


static struct timespec last_mono_clock;
static struct timespec last_proc_clock;


static unsigned long long diff_ns(struct timespec first, struct timespec second)
{
	unsigned long long temp =
        (second.tv_sec - first.tv_sec) * 1000000000ull;
    temp += (second.tv_nsec - first.tv_nsec);
    return temp;
}


void ct_initialize()
{
    clock_gettime(CLOCK_MONOTONIC, &last_mono_clock);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &last_proc_clock);
}


unsigned long long ct_get_measure_base()
{
    struct timespec current_clock;
    clock_gettime(CLOCK_MONOTONIC, &current_clock);
    unsigned long long diff = diff_ns(last_mono_clock, current_clock);
    last_mono_clock = current_clock;
    return diff;
}


unsigned long long ct_measure_thread(struct ThreadStorage* thread_storage)
{
    struct timespec current_clock;
    clock_gettime(ts_get_clock_id(thread_storage), &current_clock);
    struct timespec last_clock = ts_get_clock(thread_storage);
    unsigned long long diff = diff_ns(last_clock, current_clock);
    if(last_clock.tv_sec == 0 && last_clock.tv_nsec == 0)
    {
        diff = 0;
    }
    ts_set_clock(thread_storage, current_clock);
    return diff;
}


unsigned long long ct_measure_process()
{
    struct timespec current_clock;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &current_clock);
    unsigned long long diff = diff_ns(last_proc_clock, current_clock);
    last_proc_clock = current_clock;
    return diff;
}

unsigned int ct_calculate_percent(unsigned long long measure, unsigned long long base)
{
    return 100ull * measure / base;
}
