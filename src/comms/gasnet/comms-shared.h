/*
 *
 * Copyright (c) 2011 - 2015
 *   University of Houston System and Oak Ridge National Laboratory.
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
 * o Neither the name of the University of Houston System, Oak Ridge
 *   National Laboratory nor the names of its contributors may be used to
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

#ifndef _SHMEM_COMMS_H
#define _SHMEM_COMMS_H 1

#include <gasnet.h>

#if defined(GASNET_SEGMENT_FAST)
# define HAVE_MANAGED_SEGMENTS 1
#elif defined(GASNET_SEGMENT_LARGE)
# define HAVE_MANAGED_SEGMENTS 1
#elif defined(GASNET_SEGMENT_EVERYTHING)
# undef HAVE_MANAGED_SEGMENTS
#else
# error "I don't know what kind of GASNet segment model you're trying to use"
#endif



/**
 * set up segment/symmetric handling
 *
 */

extern gasnet_seginfo_t *seginfo_table;

#if ! defined(HAVE_MANAGED_SEGMENTS)

/**
 * this will be malloc'ed so we can respect setting from environment
 * variable
 */

#define DEFAULT_HEAP_SIZE 33554432L	/* 32M */

extern void *great_big_heap;

#else

typedef struct
{
  size_t nbytes;		/* size of write */
  void *target;			/* where to write */
  void *source;			/* data we want to get */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	/* addr of marker */
} globalvar_payload_t;

#endif /* ! HAVE_MANAGED_SEGMENTS */

/**
 * remotely modified, stop it being put in a register
 */
extern volatile int seg_setup_replies_received;

extern gasnet_hsl_t setup_out_lock;
extern gasnet_hsl_t setup_bak_lock;


/**
 * handler locks
 */

extern gasnet_hsl_t amo_swap_lock;
extern gasnet_hsl_t amo_cswap_lock;
extern gasnet_hsl_t amo_fadd_lock;
extern gasnet_hsl_t amo_add_lock;
extern gasnet_hsl_t amo_finc_lock;
extern gasnet_hsl_t amo_inc_lock;
extern gasnet_hsl_t amo_xor_lock;

/**
 * NB we make the cond/value "long long" throughout
 * to be used by smaller types as self-contained payload
 */

typedef struct
{
  void *local_store;            /* sender saves here */
  void *r_symm_addr;            /* recipient symmetric var */
  volatile int completed;       /* transaction end marker */
  volatile int *completed_addr; /* addr of marker */
  size_t nbytes;                /* how big the value is */
  long long value;              /* value to be swapped */
  long long cond;               /* conditional value */
} atomic_payload_t;


/**
 * global barrier
 */

extern long barcount;
extern int barflag;

#endif /* ! _SHMEM_COMMS_H */
