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

#include "alltoall-impl.h"

#ifdef HAVE_FEATURE_PSHMEM
#include "pshmem.h"
#endif /* HAVE_FEATURE_PSHMEM */

/*
 * TODO: tree is currently unimplemented, don't use it.
 */

static char *default_implementation = "linear";

static void (*func32) ();
static void (*func64) ();

/*
 * called during initialization of shmem
 *
 */

void
shmemi_alltoall_dispatch_init (void)
{
    char *name = shmemi_comms_getenv ("SHMEM_ALLTOALL_ALGORITHM");
    if (EXPR_LIKELY (name == (char *) NULL)) {
        name = default_implementation;
    }

    if (strcmp (name, "linear") == 0) {
        func32 = shmemi_alltoall32_linear;
        func64 = shmemi_alltoall64_linear;
    }
#if 0
    else if (strcmp (name, "tree") == 0) {
        func32 = shmemi_alltoall32_tree;
        func64 = shmemi_alltoall64_tree;
    }
#endif
    else {
        ;                       /* error */
    }
    /*
     * report which alltoall implementation we set up
     */
    shmemi_trace (SHMEM_LOG_ALLTOALL, "using alltoall \"%s\"", name);
}

/*
 * the rest is what library users see
 *
 * in this case we don't have the 32/64 bit divide, so we just look at
 * the 32-bit version and use that pointer
 *
 */

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_alltoall32 = pshmem_alltoall32
#define shmem_alltoall32 pshmem_alltoall32
#endif /* HAVE_FEATURE_PSHMEM */

void
shmem_alltoall32 (void *target, const void* source, size_t nelems,
                  int PE_start, int logPE_stride, int PE_size,
                  long *pSync)
{
    DEBUG_NAME ("shmem_alltoall32");
    INIT_CHECK (debug_name);
    shmem_quiet ();

    SYMMETRY_CHECK (target, 1, debug_name);
    SYMMETRY_CHECK (source, 2, debug_name);
    SYMMETRY_CHECK (pSync,  7, debug_name);

    func32 (target, source, 1, 1, nelems,
            PE_start, logPE_stride, PE_size, pSync);
}

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_alltoall64 = pshmem_alltoall64
#define shmem_alltoall64 pshmem_alltoall64
#endif /* HAVE_FEATURE_PSHMEM */

void
shmem_alltoall64 (void *target, const void* source, size_t nelems,
                  int PE_start, int logPE_stride, int PE_size,
                  long *pSync)
{
    DEBUG_NAME ("shmem_alltoall64");
    INIT_CHECK (debug_name);
    shmem_quiet ();

    SYMMETRY_CHECK (target, 1, debug_name);
    SYMMETRY_CHECK (source, 2, debug_name);
    SYMMETRY_CHECK (pSync,  7, debug_name);

    func64 (target, source, 1, 1, nelems,
            PE_start, logPE_stride, PE_size, pSync);
}

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_alltoalls32 = pshmem_alltoalls32
#define shmem_alltoalls32 pshmem_alltoalls32
#endif /* HAVE_FEATURE_PSHMEM */

void
shmem_alltoalls32 (void *target, const void* source,
                   ptrdiff_t dst, ptrdiff_t sst, size_t nelems,
                   int PE_start, int logPE_stride, int PE_size,
                   long *pSync)
{
    DEBUG_NAME ("shmem_alltoalls32");
    INIT_CHECK (debug_name);
    shmem_quiet ();

    SYMMETRY_CHECK (target, 1, debug_name);
    SYMMETRY_CHECK (source, 2, debug_name);
    SYMMETRY_CHECK (pSync,  9, debug_name);

    func32 (target, source, dst, sst, nelems,
            PE_start, logPE_stride, PE_size, pSync);
}

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_alltoalls64 = pshmem_alltoalls64
#define shmem_alltoalls64 pshmem_alltoalls64
#endif /* HAVE_FEATURE_PSHMEM */

void
shmem_alltoalls64 (void *target, const void* source,
                   ptrdiff_t dst, ptrdiff_t sst, size_t nelems,
                   int PE_start, int logPE_stride, int PE_size,
                   long *pSync)
{
    DEBUG_NAME ("shmem_alltoalls64");
    INIT_CHECK (debug_name);
    shmem_quiet ();

    SYMMETRY_CHECK (target, 1, debug_name);
    SYMMETRY_CHECK (source, 2, debug_name);
    SYMMETRY_CHECK (pSync,  9, debug_name);

    func64 (target, source, dst, sst, nelems,
            PE_start, logPE_stride, PE_size, pSync);
}
