/*
 *
 * Copyright (c) 2011 - 2015
 *   University of Houston System and UT-Battelle, LLC.
 * Copyright (c) 2009 - 2015
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


#ifdef HAVE_FEATURE_DEBUG

#include "uthash.h"

#include "trace.h"

#include "debug_alloc.h"

static alloc_table_t *atp = NULL;   /* our allocation hash table */

/**
 * create a new hash entry with address A and size S
 */

static inline alloc_table_t *
debug_alloc_new (void *a, size_t s)
{
    alloc_table_t *at = (alloc_table_t *) malloc (sizeof (*at));

    if (at == NULL) {
        shmemi_trace (SHMEM_LOG_FATAL,
                      "internal error: out of memory allocating address/size record");
        return NULL;
        /* NOT REACHED */
    }
    at->addr = a;
    at->size = s;
    return at;
}

/**
 * locate hash table entry for address A
 */

void *
debug_alloc_find (void *a)
{
    alloc_table_t *at = NULL;   /* entry corresponding to "a" */

    HASH_FIND_PTR (atp, &a, at);
    return at;
}

/**
 * does address A lie within a known allocation in the symmetric heap?
 * Return 1 if so, 0 if not.
 *
 * TODO: is there a better-than-linear way of discovering this?  Some
 * tree-based or inverted-map approach?
 *
 * TODO: we can also take the put/get parameters and do a full check
 * on the extent of the call.  Strided could be a weirdness.
 */

int
debug_alloc_check (void *a)
{
    alloc_table_t *tmp;
    alloc_table_t *s;

    HASH_ITER (hh, atp, s, tmp) {
        const size_t off = a - s->addr;

        /* if ( (s->size > off) && (off >= 0) ) */
        if (s->size > off) {
            return 1;
            /* NOT REACHED */
        }
    }

    shmemi_trace (SHMEM_LOG_MEMORY,
                  "address %p is not in a known symmetric allocation", a);
    return 0;
}

/**
 * when we allocate anew, add entry for address A and size S to the
 * table
 */

void
debug_alloc_add (void *a, size_t s)
{
    alloc_table_t *at = debug_alloc_new (a, s);

    HASH_ADD_PTR (atp, addr, at);
}

/**
 * when data is freed, remove from hash table
 */

void
debug_alloc_del (void *a)
{
    alloc_table_t *at = debug_alloc_find (a);

    if (at == NULL) {
        shmemi_trace (SHMEM_LOG_FATAL,
                      "internal error: no hash table entry for address %p", a);
        return;
        /* NOT REACHED */
    }
    HASH_DEL (atp, at);

    free (at);
}

/**
 * when data realigned, replace existing hash table entry
 */

void
debug_alloc_replace (void *a, size_t s)
{
#if 1
    /*
     * TODO: could be a typo in HASH_REPLACE_PTR
     * DONE: now fixed in uthash >= 1.9.9 (TC contributed fix)
     */
    alloc_table_t *at = debug_alloc_new (a, s);
    alloc_table_t *replaced_stub;
    HASH_REPLACE_PTR (atp, addr, at, replaced_stub);
#else
    debug_alloc_del (a);
    debug_alloc_add (a, s);
#endif
}

/**
 * simple utility to help debugging of this code
 */

void
debug_alloc_dump (void)
{
    alloc_table_t *tmp;
    alloc_table_t *s;

    HASH_ITER (hh, atp, s, tmp) {
        shmemi_trace (SHMEM_LOG_MEMORY,
                      "addr = %p, size = %ld", s->addr, s->size);
    }
}

#endif /* HAVE_FEATURE_DEBUG */
