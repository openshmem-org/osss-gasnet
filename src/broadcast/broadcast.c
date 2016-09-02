/*
 *
 * Copyright (c) 2016
 *   Stony Brook University
 * Copyright (c) 2015-2016
 *   Los Alamos National Security, LLC.
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



#include <stdio.h>
#include <string.h>

#include "comms.h"
#include "trace.h"
#include "utils.h"

#include "shmem.h"

#ifdef HAVE_FEATURE_PSHMEM
#include "pshmem.h"
#endif /* HAVE_FEATURE_PSHMEM */


#include "broadcast-impl.h"

static char *default_implementation = "tree";

static void (*func32) ();
static void (*func64) ();

/*
 * called during initialization of shmem
 *
 */

void
shmemi_broadcast_dispatch_init (void)
{
    char *name = shmemi_comms_getenv ("SHMEM_BROADCAST_ALGORITHM");
    if (EXPR_LIKELY (name == (char *) NULL)) {
        name = default_implementation;
    }

    if (strcmp (name, "linear") == 0) {
        func32 = shmemi_broadcast32_linear;
        func64 = shmemi_broadcast64_linear;
    }
    else if (strcmp (name, "tree") == 0) {
        func32 = shmemi_broadcast32_tree;
        func64 = shmemi_broadcast64_tree;
    }
    else {
        ;                       /* error */
    }
    /*
     * report which broadcast implementation we set up
     */
    shmemi_trace (SHMEM_LOG_BROADCAST, "using broadcast \"%s\"", name);
}

/*
 * the rest is what library users see
 *
 */

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_broadcast32 = pshmem_broadcast32
#define shmem_broadcast32 pshmem_broadcast32
#endif /* HAVE_FEATURE_PSHMEM */

void
shmem_broadcast32 (void *target, const void *source, size_t nelems,
                   int PE_root, int PE_start, int logPE_stride, int PE_size,
                   long *pSync)
{
    DEBUG_NAME ("shmem_broadcast32");
    INIT_CHECK (debug_name);
    SYMMETRY_CHECK (target, 1, debug_name);
    SYMMETRY_CHECK (source, 2, debug_name);
    SYMMETRY_CHECK (pSync, 8, debug_name);
    PE_RANGE_CHECK (PE_start, 5, debug_name);

    func32 (target, source, nelems,
            PE_root, PE_start, logPE_stride, PE_size, pSync);
}

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_broadcast64 = pshmem_broadcast64
#define shmem_broadcast64 pshmem_broadcast64
#endif /* HAVE_FEATURE_PSHMEM */

void
shmem_broadcast64 (void *target, const void *source, size_t nelems,
                   int PE_root, int PE_start, int logPE_stride, int PE_size,
                   long *pSync)
{
    DEBUG_NAME ("shmem_broadcast64");
    INIT_CHECK (debug_name);
    SYMMETRY_CHECK (target, 1, debug_name);
    SYMMETRY_CHECK (source, 2, debug_name);
    SYMMETRY_CHECK (pSync, 8, debug_name);
    PE_RANGE_CHECK (PE_start, 5, debug_name);

    func64 (target, source, nelems,
            PE_root, PE_start, logPE_stride, PE_size, pSync);
}
