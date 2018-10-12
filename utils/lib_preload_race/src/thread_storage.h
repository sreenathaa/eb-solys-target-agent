/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef THREAD_STORAGE_H
#define THREAD_STORAGE_H

#include <sys/types.h>


struct ThreadStorage;

/*
 * creates a new ThreadStorage structure, adds it to the list
 * and returns the pointer
 */
struct ThreadStorage* ts_add_thread();

/*
 * returns a pointer to the next ThreadStorage structure
 * if NULL is provided as parameter the first stucture is returned
 * if the end of the list is reached NULL will be returned
 */
struct ThreadStorage* ts_get_next(struct ThreadStorage* current);


clockid_t ts_get_clock_id(struct ThreadStorage* current);

struct timespec ts_get_clock(struct ThreadStorage* current);

void ts_set_clock(struct ThreadStorage* current, struct timespec);


pthread_t ts_get_pthread_id(struct ThreadStorage* current);

pid_t ts_get_sys_thread_id(struct ThreadStorage* current);

int ts_is_thread_alife(struct ThreadStorage* current);


void ts_count_allocate(struct ThreadStorage* current, size_t size);

void ts_count_deallocate(struct ThreadStorage* current, size_t size);


size_t ts_get_alloc_count(struct ThreadStorage* current);

size_t ts_get_memory_peak(struct ThreadStorage* current);

#endif /* THREAD_STORAGE_H */
