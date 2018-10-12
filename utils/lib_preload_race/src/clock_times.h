/*******************************************************************************
 * Copyright 2018, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/

#ifndef CLOCK_TIMES_H
#define CLOCK_TIMES_H

#include "thread_storage.h"


void ct_initialize();

unsigned long long ct_get_measure_base();

unsigned long long ct_measure_thread(struct ThreadStorage*);

unsigned long long ct_measure_process();

unsigned int ct_calculate_percent(unsigned long long measure, unsigned long long base);

#endif /* CLOCK_TIMES_H */
