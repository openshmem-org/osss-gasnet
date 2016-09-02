/* Emacs: -*- mode: c -*- */
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


#ifndef _PSHMEMX_H
#define _PSHMEMX_H 1

#include <pshmem.h>
#include <shmemx.h>

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

    /*
     * new ideas (not part of formal 1.x API)
     */

    void pshmemx_short_put_nb (short *target, const short *source,
                               size_t nelems, int pe,
                               shmemx_request_handle_t *desc);
    void pshmemx_int_put_nb (int *target, const int *source,
                             size_t nelems, int pe,
                             shmemx_request_handle_t *desc);
    void pshmemx_long_put_nb (long *target, const long *source,
                              size_t nelems, int pe,
                              shmemx_request_handle_t *desc);
    void pshmemx_longlong_put_nb (long long *target,
                                  const long long *source, size_t nelems,
                                  int pe, shmemx_request_handle_t *desc);
    void pshmemx_longdouble_put_nb (long double *target,
                                    const long double *source,
                                    size_t nelems, int pe,
                                    shmemx_request_handle_t *desc);
    void pshmemx_double_put_nb (double *target, const double *source,
                                size_t nelems, int pe,
                                shmemx_request_handle_t *desc);
    void pshmemx_float_put_nb (float *target, const float *source,
                               size_t nelems, int pe,
                               shmemx_request_handle_t *desc);
    void pshmemx_putmem_nb (void *target, const void *source,
                            size_t nelems, int pe,
                            shmemx_request_handle_t *desc);
    void pshmemx_put32_nb (void *target, const void *source,
                           size_t nelems, int pe,
                           shmemx_request_handle_t *desc);
    void pshmemx_put64_nb (void *target, const void *source,
                           size_t nelems, int pe,
                           shmemx_request_handle_t *desc);
    void pshmemx_put128_nb (void *target, const void *source,
                            size_t nelems, int pe,
                            shmemx_request_handle_t *desc);

    void pshmemx_short_get_nb (short *dest, const short *src, size_t nelems,
                               int pe, shmemx_request_handle_t *desc);
    void pshmemx_int_get_nb (int *dest, const int *src, size_t nelems,
                             int pe, shmemx_request_handle_t *desc);
    void pshmemx_long_get_nb (long *dest, const long *src, size_t nelems,
                              int pe, shmemx_request_handle_t *desc);
    void pshmemx_longlong_get_nb (long long *dest, const long long *src,
                                  size_t nelems, int pe,
                                  shmemx_request_handle_t *desc);
    void pshmemx_longdouble_get_nb (long double *dest,
                                    const long double *src,
                                    size_t nelems, int pe,
                                    shmemx_request_handle_t *desc);
    void pshmemx_double_get_nb (double *dest, const double *src,
                                size_t nelems, int pe,
                                shmemx_request_handle_t *desc);
    void pshmemx_complexd_get_nb (COMPLEXIFY (double) * dest,
                                  const COMPLEXIFY (double) * src,
                                  size_t nelems, int pe,
                                  shmemx_request_handle_t *desc);
    void pshmemx_float_get_nb (float *dest, const float *src, size_t nelems,
                               int pe, shmemx_request_handle_t *desc);
    void pshmemx_getmem_nb (void *dest, const void *src, size_t nelems,
                            int pe, shmemx_request_handle_t *desc);
    void pshmemx_get32_nb (void *dest, const void *src, size_t nelems,
                           int pe, shmemx_request_handle_t *desc);
    void pshmemx_get64_nb (void *dest, const void *src, size_t nelems,
                           int pe, shmemx_request_handle_t *desc);
    void pshmemx_get128_nb (void *dest, const void *src, size_t nelems,
                            int pe, shmemx_request_handle_t *desc);

    void pshmemx_wait_req (shmemx_request_handle_t desc);
    void pshmemx_test_req (shmemx_request_handle_t desc, int *flag);


    /*
     * symmetric memory management
     */
    void *pshmalloc_nb (size_t size);
    void  pshfree_nb (void *addr);

#define pshmemx_malloc_nb(s)   pshmalloc_nb(s)
#define pshmemx_free_nb(a)     pshfree_nb(a)

    /*
     * Proposed by IBM Zurich
     *
     */
    void pshmemx_int_xor (int *target, int value, int pe);
    void pshmemx_long_xor (long *target, long value, int pe);
    void pshmemx_longlong_xor (long long *target, long long value, int pe);

    /*
     * wallclock time
     *
     */
    double pshmemx_wtime (void);

    /*
     * symmetric address translation
     *
     */
    void *pshmemx_lookup_remote_addr (void *addr, int pe);

    /*
     * non-blocking fence and quiet checks
     *
     */
    int pshmemx_fence_test (void);
    int pshmemx_quiet_test (void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _PSHMEMX_H */
