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



#include <stdio.h>
#include <string.h>

#include "comms.h"
#include "trace.h"
#include "utils.h"

#include "shmem.h"

#include "barrier-impl.h"

#ifdef HAVE_FEATURE_PSHMEM
#include "pshmem.h"
#endif /* HAVE_FEATURE_PSHMEM */

/*
 * TODO: tree is currently buggy, don't use it
 */

static char *default_implementation = "linear";

static void (*func) ();

/*
 * called during initialization of shmem
 *
 */

void
shmemi_barrier_dispatch_init (void)
{
    char *name = shmemi_comms_getenv ("SHMEM_BARRIER_ALGORITHM");

    if (EXPR_LIKELY (name == (char *) NULL)) {
        name = default_implementation;
    }

    if (strcmp (name, "linear") == 0) {
        func = shmemi_barrier_linear;
    }
#if 0
    else if (strcmp (name, "tree") == 0) {
        func = shmemi_barrier_tree;
    }
#endif
    else {
        shmemi_trace (SHMEM_LOG_FATAL,
                      "unsupported barrier \"%s\"",
                      name);
        return;
        /* NOT REACHED */
    }

    /*
     * report which barrier implementation we set up
     */
    shmemi_trace (SHMEM_LOG_BARRIER, "using barrier \"%s\"", name);
}

/*
 * the rest is what library users see
 *
 * in this case we don't have the 32/64 bit divide, so we just look at
 * the 32-bit version and use that pointer
 *
 */

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_barrier = pshmem_barrier
#define shmem_barrier pshmem_barrier
#endif /* HAVE_FEATURE_PSHMEM */

void
shmem_barrier (int PE_start, int logPE_stride, int PE_size, long *pSync)
{
    INIT_CHECK ();

    shmem_quiet ();

    func (PE_start, logPE_stride, PE_size, pSync);
}
