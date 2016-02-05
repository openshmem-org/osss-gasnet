/*
 *
 * Copyright (c) 2011 - 2016
 *   University of Houston System and UT-Battelle, LLC.
 * Copyright (c) 2009 - 2016
 *   Silicon Graphics International Corp.  SHMEM is copyrighted
 *   by Silicon Graphics International Corp. (SGI) The OpenSHMEM API
 *   (shmem) is released by Open Source Software Solutions, Inc., under an
 *   agreement with Silicon Graphics International Corp. (SGI).
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * o Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * o Neither the name of the University of Houston System,
 *   UT-Battelle, LLC. nor the names of its contributors may be used to
 *   endorse or promote products derived from this software without specific
 *   prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * all the non-inlined bits that need to be shared per-PE
 *
 */

#include <gasnet.h>

/**
 * This file provides the layer on top of GASNet, ARMCI or whatever.
 * API should be formalized at some point.
 */

#include "comms-shared.h"

/**
 * set up segment/symmetric handling
 *
 */

gasnet_seginfo_t *seginfo_table;

#if ! defined(HAVE_MANAGED_SEGMENTS)

/**
 * this will be malloc'ed so we can respect setting from environment
 * variable
 */

void *great_big_heap;

/**
 * remotely modified, stop it being put in a register
 */
volatile int seg_setup_replies_received = 0;

gasnet_hsl_t setup_out_lock = GASNET_HSL_INITIALIZER;
gasnet_hsl_t setup_bak_lock = GASNET_HSL_INITIALIZER;

#endif /* ! HAVE_MANAGED_SEGMENTS */

/**
 * remotely modified, stop it being put in a register
 */
volatile int globalexit_replies_received = 0;

gasnet_hsl_t globalexit_out_lock = GASNET_HSL_INITIALIZER;
gasnet_hsl_t globalexit_bak_lock = GASNET_HSL_INITIALIZER;

/**
 * Initialize handler locks.  OpenSHMEM 1.3++ guarantees per-datatype
 * exclusivity, so prep for that below.
 */

#define AMO_LOCK_DECL_EMIT(Name, Type) \
    gasnet_hsl_t amo_lock_##Name = GASNET_HSL_INITIALIZER

AMO_LOCK_DECL_EMIT (int, int);
AMO_LOCK_DECL_EMIT (long, long);
AMO_LOCK_DECL_EMIT (longlong, long long);
AMO_LOCK_DECL_EMIT (float, float);
AMO_LOCK_DECL_EMIT (double, double);

/**
 * global barrier counters
 */

long barcount = 0;
int barflag = 0;
