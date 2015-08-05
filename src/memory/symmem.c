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
#include <assert.h>
#include <sys/types.h>

#include "state.h"
#include "comms.h"
#include "trace.h"
#include "memalloc.h"
#include "utils.h"

#include "shmem.h"

#ifdef HAVE_FEATURE_PSHMEM
#include "pshmem.h"
#endif /* HAVE_FEATURE_PSHMEM */

/*
 * Not present in SGI API any more.  I'm going to leave it in the
 * code, because Fortran needs it.  Removed from shmem.h.
 *
 */
long malloc_error = _SHMEM_MALLOC_OK;   /* exposed for error codes */


#ifdef HAVE_FEATURE_DEBUG

/**
 * check that all PEs see the same symmetric malloc size: return first
 * mis-matching PE id if there's a mis-match, return -1 to record
 * correct symmetry (no offending PE)
 */

static inline int
__shmalloc_symmetry_check (size_t size)
{
    int pe;
    int any_failed_pe = -1;
    long shmalloc_received_size;
    long *shmalloc_remote_size;

    /* record for everyone else to see */
    shmalloc_remote_size =
        (long *) shmemi_mem_alloc (sizeof (*shmalloc_remote_size));
    if (shmalloc_remote_size == (long *) NULL) {
        shmemi_trace (SHMEM_LOG_FATAL,
                      "internal error: couldn't allocate memory for symmetry check");
        /* NOT REACHED */
    }
    *shmalloc_remote_size = size;
    shmem_barrier_all ();

    malloc_error = _SHMEM_MALLOC_OK;

    /* 
     * everyone checks everyone else's sizes, barf if mis-match
     *
     * TODO: probably some kind of Eureka! optimization opportunity here
     *
     */
    for (pe = 0; pe < GET_STATE (numpes); pe += 1) {
        if (pe == GET_STATE (mype)) {
            continue;
        }
        shmalloc_received_size = shmem_long_g (shmalloc_remote_size, pe);
        if (shmalloc_received_size != size) {
            shmemi_trace (SHMEM_LOG_NOTICE,
                          "shmalloc expected %ld, but saw %ld on PE %d",
                          size, shmalloc_received_size, pe);
            malloc_error = _SHMEM_MALLOC_SYMMSIZE_FAILED;
            any_failed_pe = pe;
            break;
            /* NOT REACHED */
        }
    }
    /* make sure everyone is here before freeing things */
    shmem_barrier_all ();
    shmemi_mem_free (shmalloc_remote_size);
    return any_failed_pe;
}
#endif /* HAVE_FEATURE_DEBUG */

/**
 * this call avoids the symmetry check that the real shmalloc() has to
 * do and is thus cheaper.  Intended for internal use when we know in
 * advance the supplied size is symmetric.
 *
 */

static inline void *
__shmalloc_no_check (size_t size)
{
    void *addr;

    addr = shmemi_mem_alloc (size);

    if (addr == (void *) NULL) {
        shmemi_trace (SHMEM_LOG_NOTICE, "shmalloc(%ld bytes) failed", size);
        malloc_error = _SHMEM_MALLOC_FAIL;
    }
    else {
        malloc_error = _SHMEM_MALLOC_OK;
    }

    shmemi_trace (SHMEM_LOG_MEMORY, "shmalloc(%ld bytes) @ %p", size, addr);

    return addr;
}



#ifdef HAVE_FEATURE_EXPERIMENTAL
#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmalloc_nb = pshmalloc_nb
#define shmalloc_nb pshmalloc_nb
#endif /* HAVE_FEATURE_PSHMEM */
#endif /* HAVE_FEATURE_EXPERIMENTAL */

/**
 * Symmetrically allocate "size" byte of memory across all PEs,
 * don't wait for everyone to be ready
 */

void *
shmalloc_nb (size_t size)
{
    void *addr;

    INIT_CHECK ();

#ifdef HAVE_FEATURE_DEBUG
    if (__shmalloc_symmetry_check (size) != -1) {
        malloc_error = _SHMEM_MALLOC_SYMMSIZE_FAILED;
        return (void *) NULL;
        /* NOT REACHED */
    }
#endif /* HAVE_FEATURE_DEBUG */

    shmemi_trace (SHMEM_LOG_MEMORY,
                  "shmalloc(%ld bytes) passed symmetry check", size);

    addr = __shmalloc_no_check (size);

    malloc_error = (addr != NULL)
        ? _SHMEM_MALLOC_OK : _SHMEM_MALLOC_FAIL;

    return addr;
}

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmalloc = pshmalloc
#define shmalloc pshmalloc
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * Symmetrically allocate "size" byte of memory across all PEs,
 * everyone will be ready for remote memory use afterward
 */

static inline void *
shmalloc_private (size_t size)
{
    void *addr = shmalloc_nb (size);

    shmem_barrier_all ();

    return addr;
}

/*
 * Deprecated call as of 1.2
 */
void *
shmalloc (size_t size)
{
    return shmalloc_private (size);
}

void *
shmem_malloc (size_t size)
{
    return shmalloc_private (size);
}


#ifdef HAVE_FEATURE_EXPERIMENTAL
#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shfree_nb = pshfree_nb
#define shfree_nb pshfree_nb
#endif /* HAVE_FEATURE_PSHMEM */
#endif /* HAVE_FEATURE_EXPERIMENTAL */

/**
 * Symmetrically free previously allocated memory,
 * don't wait for everyone to be ready
 */

void
shfree_nb (void *addr)
{
    INIT_CHECK ();

    if (addr == (void *) NULL) {
        shmemi_trace (SHMEM_LOG_MEMORY,
                      "address passed to shfree() already null");
        malloc_error = _SHMEM_MALLOC_ALREADY_FREE;
        return;
        /* NOT REACHED */
    }

    shmemi_trace (SHMEM_LOG_MEMORY,
                  "shfree(%p) in pool @ %p", addr, shmemi_mem_base ());

    shmemi_mem_free (addr);

    malloc_error = _SHMEM_MALLOC_OK;
}

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shfree = pshfree
#define shfree pshfree
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * Symmetrically free previously allocated memory,
 * everyone has synched beforehand
 */

static inline void
shfree_private (void *addr)
{
    shmem_barrier_all ();

    shfree_nb (addr);
}

/*
 * Deprecated call as of 1.2
 */
void
shfree (void *addr)
{
    shfree_private (addr);
}

void
shmem_free (void *addr)
{
    shfree_private (addr);
}

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shrealloc = pshrealloc
#define shrealloc pshrealloc
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * Resize previously allocated symmetric memory
 */

static inline void *
shrealloc_private (void *addr, size_t size)
{
    void *newaddr;

    INIT_CHECK ();

    if (addr == (void *) NULL) {
        shmemi_trace (SHMEM_LOG_MEMORY,
                      "address passed to shrealloc() is null, handing to shmalloc()");
        return shmem_malloc (size);
        /* NOT REACHED */
    }

    if (size == 0) {
        shmemi_trace (SHMEM_LOG_MEMORY,
                      "size passed to shrealloc() is 0, handing to shfree()");
        shmem_free (addr);
        return (void *) NULL;
        /* NOT REACHED */
    }

#ifdef HAVE_FEATURE_DEBUG
    if (__shmalloc_symmetry_check (size) != -1) {
        malloc_error = _SHMEM_MALLOC_SYMMSIZE_FAILED;
        return (void *) NULL;
        /* NOT REACHED */
    }
#endif /* HAVE_FEATURE_DEBUG */

    newaddr = shmemi_mem_realloc (addr, size);

    if (newaddr == (void *) NULL) {
        shmemi_trace (SHMEM_LOG_MEMORY,
                      "shrealloc(%ld bytes) failed @ original address %p",
                      size, addr);
        malloc_error = _SHMEM_MALLOC_REALLOC_FAILED;
    }
    else {
        malloc_error = _SHMEM_MALLOC_OK;
    }

    shmem_barrier_all ();

    return newaddr;
}

/*
 * Deprecated call as of 1.2
 */
void *
shrealloc (void *addr, size_t size)
{
    return shrealloc_private (addr, size);
}

void *
shmem_realloc (void *addr, size_t size)
{
    return shrealloc_private (addr, size);
}

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmemalign = pshmemalign
#define shmemalign pshmemalign
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * The shmemalign function allocates a block in the symmetric heap that
 * has a byte alignment specified by the alignment argument.
 */

static inline void *
shmemalign_private (size_t alignment, size_t size)
{
    void *addr;

    INIT_CHECK ();

#ifdef HAVE_FEATURE_DEBUG
    if (__shmalloc_symmetry_check (size) != -1) {
        malloc_error = _SHMEM_MALLOC_SYMMSIZE_FAILED;
        return (void *) NULL;
        /* NOT REACHED */
    }
#endif /* HAVE_FEATURE_DEBUG */

    addr = shmemi_mem_align (alignment, size);

    if (addr == (void *) NULL) {
        shmemi_trace (SHMEM_LOG_MEMORY,
                      "shmem_memalign(%ld bytes) couldn't realign to %ld",
                      size, alignment);
        malloc_error = _SHMEM_MALLOC_MEMALIGN_FAILED;
    }
    else {
        malloc_error = _SHMEM_MALLOC_OK;
    }

    shmem_barrier_all ();

    return addr;
}


/*
 * Deprecated call as of 1.2
 */
void *
shmemalign (size_t alignment, size_t size)
{
    return shmemalign_private (alignment, size);
}

void *
shmem_align (size_t alignment, size_t size)
{
    return shmemalign_private (alignment, size);
}

/**
 * readable error message for error code "e"
 */

typedef struct
{
    long code;
    char *msg;
} malloc_error_code_t;

static malloc_error_code_t error_table[] = {
    {_SHMEM_MALLOC_OK,
     "no symmetric memory allocation error"},
    {_SHMEM_MALLOC_FAIL,
     "symmetric memory allocation failed"},
    {_SHMEM_MALLOC_ALREADY_FREE,
     "attempt to free already null symmetric memory address"},
    {_SHMEM_MALLOC_MEMALIGN_FAILED,
     "attempt to align symmetric memory address failed"},
    {_SHMEM_MALLOC_REALLOC_FAILED,
     "attempt to reallocate symmetric memory address failed"},
    {_SHMEM_MALLOC_SYMMSIZE_FAILED,
     "asymmetric sizes passed to symmetric memory allocator"},
    {_SHMEM_MALLOC_BAD_SIZE,
     "size of data to allocate can not be negative"},
    {_SHMEM_MALLOC_NOT_ALIGNED,
     "address is not block-aligned"},
    {_SHMEM_MALLOC_NOT_IN_SYMM_HEAP,
     "address falls outside of symmetric heap"},
};

static const int nerrors = TABLE_SIZE (error_table);

#ifdef HAVE_FEATURE_PSHMEM
extern char *sherror (void);    /* ! API */
#pragma weak sherror = psherror
#define sherror psherror
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * Return human-readable error message
 */

char *
sherror (void)
{
    malloc_error_code_t *etp = error_table;
    int i;

    INIT_CHECK ();

    for (i = 0; i < nerrors; i += 1) {
        if (malloc_error == etp->code) {
            return etp->msg;
            /* NOT REACHED */
        }
        etp += 1;
    }

    return "unknown memory error";
}
