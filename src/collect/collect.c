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

#include "collect-impl.h"

#ifdef HAVE_FEATURE_PSHMEM
#include "pshmem.h"
#endif /* HAVE_FEATURE_PSHMEM */

static char *default_implementation = "linear";

static void (*func32) ();
static void (*func64) ();

void
shmemi_collect_dispatch_init (void)
{
    char *name = shmemi_comms_getenv ("SHMEM_COLLECT_ALGORITHM");

    if (EXPR_LIKELY (name == (char *) NULL)) {
        name = default_implementation;
    }

    if (strcmp (name, "linear") == 0) {
        func32 = shmemi_collect32_linear;
        func64 = shmemi_collect64_linear;
    }
    else {
        ;                       /* error */
    }

    /*
     * report which implementation we set up
     */
    shmemi_trace (SHMEM_LOG_BROADCAST, "using collect \"%s\"", name);
}

/*
 * the rest is what library users see
 *
 */

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_collect32 = pshmem_collect32
#define shmem_collect32 pshmem_collect32
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * Collective concatenation of 32-bit data from participating PEs
 * into a target array on all those PEs
 */

void
shmem_collect32 (void *target, const void *source, size_t nelems,
                 int PE_start, int logPE_stride, int PE_size, long *pSync)
{
    INIT_CHECK ();
    SYMMETRY_CHECK (target, 1, "shmem_collect32");
    SYMMETRY_CHECK (source, 2, "shmem_collect32");
    SYMMETRY_CHECK (pSync, 7, "shmem_collect32");
    PE_RANGE_CHECK (PE_start, 4);
    /* PE_RANGE_CHECK (PE_size, 6); */

    func32 (target, source, nelems, PE_start, logPE_stride, PE_size, pSync);
}


#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_collect64 = pshmem_collect64
#define shmem_collect64 pshmem_collect64
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * Collective concatenation of 64-bit data from participating PEs
 * into a target array on all those PEs
 */

void
shmem_collect64 (void *target, const void *source, size_t nelems,
                 int PE_start, int logPE_stride, int PE_size, long *pSync)
{
    INIT_CHECK ();
    SYMMETRY_CHECK (target, 1, "shmem_collect64");
    SYMMETRY_CHECK (source, 2, "shmem_collect64");
    SYMMETRY_CHECK (pSync, 7, "shmem_collect64");
    PE_RANGE_CHECK (PE_start, 4);
    /* PE_RANGE_CHECK (PE_size, 6); */

    func64 (target, source, nelems, PE_start, logPE_stride, PE_size, pSync);
}
