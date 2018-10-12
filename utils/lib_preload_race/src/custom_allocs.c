/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#include "thread_storage.h"
#include "libc_internals.h"


typedef struct
{
    size_t size;
    struct ThreadStorage* counters;
}
AllocHeader;


struct ThreadStorage* get_thread_storage()
{
    static __thread struct ThreadStorage* counters = 0;

    if(counters == 0)
    {
        counters = ts_add_thread();
    }

    return counters;
}


void* malloc(size_t size)
{
    struct ThreadStorage* tl_counters = get_thread_storage();
    ts_count_allocate(tl_counters, size);

    void* ptr = __libc_malloc(size + sizeof(AllocHeader));
    AllocHeader* ptr_header = (AllocHeader*)ptr;
    ptr_header->size = size;
    ptr_header->counters = tl_counters;
	return ptr + sizeof(AllocHeader);
}


void* calloc(size_t nmem, size_t item_size)
{
    const size_t size = nmem * item_size;

    struct ThreadStorage* tl_counters = get_thread_storage();
    ts_count_allocate(tl_counters, size);

    void* ptr = __libc_calloc(size + sizeof(AllocHeader), 1);
    AllocHeader* ptr_header = (AllocHeader*)ptr;
    ptr_header->size = size;
    ptr_header->counters = tl_counters;
	return ptr + sizeof(AllocHeader);
}


void* realloc(void *cur, size_t size)
{
    if(cur)
    {
        cur -= sizeof(AllocHeader);

        AllocHeader* ptr_header = (AllocHeader*)cur;
        const size_t old_size = ptr_header->size;
        struct ThreadStorage* old_counters = ptr_header->counters;
        ts_count_deallocate(old_counters, old_size);
    }

    struct ThreadStorage* tl_counters = get_thread_storage();
    ts_count_allocate(tl_counters, size);

    void* ptr = __libc_realloc(cur, size + sizeof(AllocHeader));
    AllocHeader* ptr_header = (AllocHeader*)ptr;
    ptr_header->size = size;
    ptr_header->counters = tl_counters;
	return ptr + sizeof(AllocHeader);
}


void free(void* ptr)
{
    if(ptr)
    {
        ptr -= sizeof(AllocHeader);

        AllocHeader* ptr_header = (AllocHeader*)ptr;
        const size_t size = ptr_header->size;
        struct ThreadStorage* counters = ptr_header->counters;
        ts_count_deallocate(counters, size);
    }

    __libc_free(ptr);
}
