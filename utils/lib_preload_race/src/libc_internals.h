/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

// definition of libc malloc functions
extern void* __libc_malloc(size_t size);
extern void* __libc_calloc(size_t nmem, size_t item_size);
extern void* __libc_realloc(void *current, size_t size);
extern void  __libc_free(void* ptr);

// definition of libc symbols
extern char* __progname;
