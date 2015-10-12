/*
 *
 * Copyright (c) 2011 - 2015
 *  University of Houston System and UT-Battelle, LLC.
 * Copyright (c) 2009 - 2015
 *  Silicon Graphics International Corp.  SHMEM is copyrighted
 *  by Silicon Graphics International Corp. (SGI) The OpenSHMEM API
 *  (shmem) is released by Open Source Software Solutions, Inc., under an
 *  agreement with Silicon Graphics International Corp. (SGI).
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



/*
 *    Copyright (c) 1996-2002 by Quadrics Supercomputers World Ltd.
 *    Copyright (c) 2003-2005 by Quadrics Ltd.
 *
 *    For licensing information please see the supplied COPYING file
 *
 */

#include <stdlib.h>
#include <stdint.h>

#include "atomic.h"
#include "trace.h"

#include "comms.h"

#include "state.h"

#include "shmem.h"

#ifdef HAVE_FEATURE_PSHMEM
#include "pshmem.h"
#endif /* HAVE_FEATURE_PSHMEM */


#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_set_lock = pshmem_set_lock
#define shmem_set_lock pshmem_set_lock
#pragma weak shmem_test_lock = pshmem_test_lock
#define shmem_test_lock pshmem_test_lock
#pragma weak shmem_clear_lock = pshmem_clear_lock
#define shmem_clear_lock pshmem_clear_lock
#endif /* HAVE_FEATURE_PSHMEM */

void
shmem_set_lock (volatile long *lock)
{
    shmemi_comms_lock_acquire (&((SHMEM_LOCK *) lock)[1],
                               &((SHMEM_LOCK *) lock)[0],
                               GET_STATE (mype));
}

void
shmem_clear_lock (volatile long *lock)
{
    /* The Cray man pages suggest we also need to do this (addy
       12.10.05) */
    shmem_quiet ();

    shmemi_comms_lock_release (&((SHMEM_LOCK *) lock)[1],
                               &((SHMEM_LOCK *) lock)[0],
                               GET_STATE (mype));
}

int
shmem_test_lock (volatile long *lock)
{
    return shmemi_comms_lock_test (&((SHMEM_LOCK *) lock)[1],
                                   &((SHMEM_LOCK *) lock)[0],
                                   GET_STATE (mype));
}
