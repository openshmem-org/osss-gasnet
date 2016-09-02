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



#include <sys/types.h>
#include <stdint.h>

#include "utils.h"
#include "trace.h"

#include "fortran-common.h"

#include "shmem.h"

#ifdef HAVE_FEATURE_PSHMEM
#include "pshmem.h"
#endif /* HAVE_FEATURE_PSHMEM */

/*
 * used by Fortran routines, but not part of API
 */
extern char *sherror (void);

/*
 * Fortran symmetric memory operations
 */

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shpalloc_ = pshpalloc_
#define shpalloc_ pshpalloc_
#pragma weak shpdeallc_ = pshpdeallc_
#define shpdeallc_ pshpdeallc_
#pragma weak shpclmove_ = pshpclmove_
#define shpclmove_ pshpclmove_
#endif /* HAVE_FEATURE_PSHMEM */

/**
 *
 * SYNOPSIS
 *   POINTER (addr, A(1))
 *   INTEGER (length, errcode, abort)
 *   CALL SHPALLOC(addr, length, errcode, abort)
 *
 * DESCRIPTION
 *   SHPALLOC  allocates a block of memory from the program's symmetric heap
 *   that is greater than or equal  to  the  size  requested.   To  maintain
 *   symmetric  heap  consistency,  all PEs in an program must call SHPALLOC
 *   with the same value of length; if any  processing  elements  (PEs)  are
 *   missing, the program will hang.
 */

/**
 *
 * we've removed this from the C API since it's a hold-over from an
 * old SGI version.  Left it in the code for now, though, (a) for
 * Fortran to use and (b) in case people want it back.
 */
extern long malloc_error;

void FORTRANIFY (shpalloc) (uintptr_t **addr, int *length,
                            int *errcode, int *abort)
{
    DEBUG_NAME ("shpalloc");
    /* convert 32-bit words to bytes */
    const int scale = sizeof (int32_t);
    void *symm_addr;

    INIT_CHECK (debug_name);

    if (*length <= 0) {
        *errcode = SHMEM_MALLOC_BAD_SIZE;
        return;
        /* NOT REACHED */
    }

    symm_addr = shmem_malloc (*length * scale);

    /* pass back status code */
    *errcode = malloc_error;

    /* if malloc succeeded, nothing else to do */
    if (malloc_error == SHMEM_MALLOC_OK) {
        *addr = symm_addr;

        shmemi_trace (SHMEM_LOG_MEMORY,
                      "shpalloc(addr = %p, length = %d,"
                      " errcode = %d, abort = %d)",
                      addr, *length,
                      *errcode, *abort
                      );

        return;
        /* NOT REACHED */
    }

    /* failed somehow, we might have to abort */
    shmemi_trace (*abort ? SHMEM_LOG_FATAL : SHMEM_LOG_MEMORY,
                  "shpalloc() was given non-symmetric memory sizes");
    /* MAYBE NOT REACHED */

    addr = NULL;
}

/**
 *
 * SYNOPSIS
 *   POINTER (addr, A(1))
 *   INTEGER errcode, abort
 *   CALL SHPDEALLC(addr, errcode, abort)
 *
 *
 * DESCRIPTION
 *   SHPDEALLC  returns  a block of memory (allocated using SHPALLOC) to the
 *   list of available space in the symmetric heap.  To  maintain  symmetric
 *   heap  consistency, all processing elements (PEs) in a program must call
 *   SHPDEALLC with the same value of addr; if  any  PEs  are  missing,  the
 *   program hangs.
 */

void FORTRANIFY (shpdeallc) (uintptr_t **addr, int *errcode, int *abort)
{
    DEBUG_NAME ("shpdeallc");
    INIT_CHECK (debug_name);

    shmemi_trace (SHMEM_LOG_MEMORY,
                  "shpdeallc(addr = %p, errcode = %d, abort = %d)",
                  *addr, *errcode, *abort);

    shmem_free (*addr);

    /* pass back status code */
    *errcode = malloc_error;

    /* if malloc succeeded, nothing else to do */
    if (malloc_error == SHMEM_MALLOC_OK) {
        return;
        /* NOT REACHED */
    }

    /* failed somehow, we might have to abort */
    shmemi_trace (*abort ? SHMEM_LOG_FATAL : SHMEM_LOG_MEMORY,
                  "shpdeallc() failed: %s", sherror ());
    /* MAYBE NOT REACHED */
}

/**
 *
 * SYNOPSIS
 *   POINTER (addr, A(1))
 *   INTEGER length, status, abort
 *   CALL SHPCLMOVE (addr, length, status, abort)
 *
 *
 * DESCRIPTION
 *   The SHPCLMOVE function either extends a symmetric  heap  block  if  the
 *   block  is  followed by a large enough free block or copies the contents
 *   of the existing block to a larger  block  and  returns  a  status  code
 *   indicating that the block was moved.  This function also can reduce the
 *   size of a block if the new length is less than  the  old  length.   All
 *   processing  elements  (PEs)  in  a program must call SHPCLMOVE with the
 *   same value of addr to maintain symmetric heap consistency; if  any  PEs
 *   are missing, the program hangs.
 */

void FORTRANIFY (shpclmove) (uintptr_t **addr, int *length,
                             int *errcode, int *abort)
{
    DEBUG_NAME ("shpclmove");
    INIT_CHECK (debug_name);

    *addr = shmem_realloc (*addr, *length);

    /* pass back status code */
    *errcode = malloc_error;

    /* if malloc succeeded, nothing else to do */
    if (malloc_error == SHMEM_MALLOC_OK) {
        return;
        /* NOT REACHED */
    }

    /* failed somehow, we might have to abort */
    shmemi_trace (*abort ? SHMEM_LOG_FATAL : SHMEM_LOG_MEMORY,
                  "shpdeallc() failed: %s", sherror ());
    /* MAYBE NOT REACHED */
}
