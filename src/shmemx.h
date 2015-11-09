/* Emacs: -*- mode: c -*- */
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



#ifndef _SHMEMX_H
#define _SHMEMX_H 1

#include <shmem.h>

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

    /*
     * new ideas (not part of formal 1.x API)
     */

    typedef void *shmemx_request_handle_t;

    void shmemx_short_put_nb (short *dest, const short *src, size_t nelems,
                              int pe, shmemx_request_handle_t *desc);
    void shmemx_int_put_nb (int *dest, const int *src, size_t nelems,
                            int pe, shmemx_request_handle_t *desc);
    void shmemx_long_put_nb (long *dest, const long *src, size_t nelems,
                             int pe, shmemx_request_handle_t *desc);
    void shmemx_longlong_put_nb (long long *dest, const long long *src,
                                 size_t nelems, int pe, shmemx_request_handle_t *desc);
    void shmemx_longdouble_put_nb (long double *dest, const long double *src,
                                   size_t nelems, int pe, shmemx_request_handle_t *desc);
    void shmemx_double_put_nb (double *dest, const double *src,
                               size_t nelems, int pe, shmemx_request_handle_t *desc);
    void shmemx_complexd_put_nb (COMPLEXIFY (double) * dest,
                                 const COMPLEXIFY (double) * src,
                                 size_t nelems, int pe, shmemx_request_handle_t *desc);
    void shmemx_float_put_nb (float *dest, const float *src, size_t nelems,
                              int pe, shmemx_request_handle_t *desc);
    void shmemx_putmem_nb (void *dest, const void *src, size_t nelems,
                           int pe, shmemx_request_handle_t *desc);
    void shmemx_put32_nb (void *dest, const void *src, size_t nelems,
                          int pe, shmemx_request_handle_t *desc);
    void shmemx_put64_nb (void *dest, const void *src, size_t nelems,
                          int pe, shmemx_request_handle_t *desc);
    void shmemx_put128_nb (void *dest, const void *src, size_t nelems,
                           int pe, shmemx_request_handle_t *desc);

    void shmemx_short_get_nb (short *dest, const short *src, size_t nelems,
                              int pe, shmemx_request_handle_t *desc);
    void shmemx_int_get_nb (int *dest, const int *src, size_t nelems,
                            int pe, shmemx_request_handle_t *desc);
    void shmemx_long_get_nb (long *dest, const long *src, size_t nelems,
                             int pe, shmemx_request_handle_t *desc);
    void shmemx_longlong_get_nb (long long *dest, const long long *src,
                                 size_t nelems, int pe, shmemx_request_handle_t *desc);
    void shmemx_longdouble_get_nb (long double *dest, const long double *src,
                                   size_t nelems, int pe, shmemx_request_handle_t *desc);
    void shmemx_double_get_nb (double *dest, const double *src,
                               size_t nelems, int pe, shmemx_request_handle_t *desc);
    void shmemx_complexd_get_nb (COMPLEXIFY (double) * dest,
                                 const COMPLEXIFY (double) * src,
                                 size_t nelems, int pe, shmemx_request_handle_t *desc);
    void shmemx_float_get_nb (float *dest, const float *src, size_t nelems,
                              int pe, shmemx_request_handle_t *desc);
    void shmemx_getmem_nb (void *dest, const void *src, size_t nelems,
                           int pe, shmemx_request_handle_t *desc);
    void shmemx_get32_nb (void *dest, const void *src, size_t nelems,
                          int pe, shmemx_request_handle_t *desc);
    void shmemx_get64_nb (void *dest, const void *src, size_t nelems,
                          int pe, shmemx_request_handle_t *desc);
    void shmemx_get128_nb (void *dest, const void *src, size_t nelems,
                           int pe, shmemx_request_handle_t *desc);

    void shmemx_wait_req (shmemx_request_handle_t desc);
    void shmemx_test_req (shmemx_request_handle_t desc, int *flag);

    /*
     * renamed & non-blocking memory management
     *
     */
    void *shmalloc_nb (size_t size) _WUR;
    void  shfree_nb (void *addr);

#define shmemx_malloc_nb(s)   shmalloc_nb(s)
#define shmemx_free_nb(a)     shfree_nb(a)

    /*
     * Proposed by IBM Zurich
     *
     */

    /**
     * @brief These routines perform an atomic exclusive-or (xor) operation
     * between a data value and the target data object.
     *
     * @b Synopsis:
     *
     * - C/C++:
     * @code
     void shmemx_int_xor (int *target, int value, int pe);
     void shmemx_long_xor (long *target, long value, int pe);
     void shmemx_longlong_xor (long long *target, long long value, int pe);
     * @endcode
     *
     * - Fortran:
     * @code
     INTEGER pe

     SHMEMX_INT4_XOR (target, value, pe)
     SHMEMX_INT8_XOR (target, value, pe)
     * @endcode
     *
     * @param target    Address of the symmetric data object where to save the data on the target pe.
     * @param value     The value with which the exclusive-or operation is atomically
     *                performed with the data at address target.
     * @param pe        An integer that indicates the PE number upon
     *                which target is to be updated. If you are using Fortran, it must
     *                be a default integer value.
     *
     * @b Constraints:
     *      - target must be the address of a symmetric data object.
     *      - If using C/C++, the type of value must match that implied in the Synopsis
     *      section. When calling from Fortran, the data type of value must be as follows:
     *          - For SHMEMX_INT4_XOR(), value must be of type Integer,
     *            with element size of 4 bytes
     *          - For SHMEMX_INT8_XOR(), value must be of type Integer,
     *            with element size of 8 bytes.
     *      - value must be the same type as the target data object.
     *      - This process must be carried out guaranteeing that it will not be interrupted by any other operation.
     *
     * @b Effect:
     *
     * The atomic exclusive-or routines perform an xor-operation between
     * value and the data at address target on PE pe. The operation must
     * be completed without the possibility of another process updating
     * target between the time of the fetch and the update.
     *
     * @return None.
     *
     */
    void shmemx_int_xor (int *target, int value, int pe);
    void shmemx_long_xor (long *target, long value, int pe);
    void shmemx_longlong_xor (long long *target, long long value, int pe);

    /**
     * @brief These routines perform an atomic fetch from a remote PE
     *
     * @b Synopsis:
     *
     * - C/C++:
     * @code
     ...
     * @endcode
     *
     * - Fortran:
     * @code
     ...
     * @endcode
     *
     * @param target    Address of the symmetric data object where to save the data on the target pe.
     * @param value     The value with which the exclusive-or operation is atomically
     *                performed with the data at address target.
     * @param pe        An integer that indicates the PE number upon
     *                which target is to be updated. If you are using Fortran, it must
     *                be a default integer value.
     *
     * @b Constraints:
     *      - target must be the address of a symmetric data object.
     *      - If using C/C++, the type of value must match that implied in the Synopsis
     *      section. When calling from Fortran, the data type of value must be as follows:
     *          - For SHMEMX_INT4_XOR(), value must be of type Integer,
     *            with element size of 4 bytes
     *          - For SHMEMX_INT8_XOR(), value must be of type Integer,
     *            with element size of 8 bytes.
     *      - value must be the same type as the target data object.
     *      - This process must be carried out guaranteeing that it will not be interrupted by any other operation.
     *
     * @b Effect:
     *
     * The atomic exclusive-or routines perform an xor-operation between
     * value and the data at address target on PE pe. The operation must
     * be completed without the possibility of another process updating
     * target between the time of the fetch and the update.
     *
     * @return None.
     *
     */
    int shmemx_int_fetch (int *target, int pe);
    long shmemx_long_fetch (long *target, int pe);
    long long shmemx_longlong_fetch (long long *target, int pe);

    /**
     * @brief These routines perform an atomic set of a variables on a
     * remote PE
     *
     * @b Synopsis:
     *
     * - C/C++:
     * @code
     ...
     * @endcode
     *
     * - Fortran:
     * @code
     ...
     * @endcode
     *
     * @param target    Address of the symmetric data object where to save the data on the target pe.
     * @param value     The value with which the exclusive-or operation is atomically
     *                performed with the data at address target.
     * @param pe        An integer that indicates the PE number upon
     *                which target is to be updated. If you are using Fortran, it must
     *                be a default integer value.
     *
     * @b Constraints:
     *      - target must be the address of a symmetric data object.
     *      - If using C/C++, the type of value must match that implied in the Synopsis
     *      section. When calling from Fortran, the data type of value must be as follows:
     *          - For SHMEMX_INT4_XOR(), value must be of type Integer,
     *            with element size of 4 bytes
     *          - For SHMEMX_INT8_XOR(), value must be of type Integer,
     *            with element size of 8 bytes.
     *      - value must be the same type as the target data object.
     *      - This process must be carried out guaranteeing that it will not be interrupted by any other operation.
     *
     * @b Effect:
     *
     * The atomic exclusive-or routines perform an xor-operation between
     * value and the data at address target on PE pe. The operation must
     * be completed without the possibility of another process updating
     * target between the time of the fetch and the update.
     *
     * @return None.
     *
     */
    void shmemx_int_set (int *target, int value, int pe);
    void shmemx_long_set (long *target, long value, int pe);
    void shmemx_longlong_set (long long *target, long long value, int pe);

    /*
     * wallclock time
     *
     */

    /**
     * @brief returns the number of seconds since the program
     * started running
     *
     * @section Synopsis:
     *
     * @substitute c C/C++
     * @code
     double shmemx_wtime (void);
     * @endcode
     *
     * @subsection f Fortran
     * @code
     double precision shmemx_wtime()
     * @endcode
     *
     * @return Returns the number of seconds since the program started (epoch).
     *
     * @section Note: shmemx_wtime does not indicate any error code; if it
     * is unable to detect the elapsed time, the return value is
     * undefined.  The time may be different on each PE, but the epoch
     * from which the time is measured will not change while OpenSHMEM is
     * active.
     *
     */

    double shmemx_wtime (void);

    /*
     * address translation
     *
     */

    /**
     * @brief returns the symmetric address on another PE corresponding to
     * the symmetric address on this PE
     *
     * @section Synopsis:
     *
     * @substitute c C/C++
     * @code
     void *shmemx_lookup_remote_addr (void *addr, int pe);
     * @endcode
     *
     * @subsection f Fortran
     * @code
     integer pe
     integer shmemx_lookup_remote_addr (addr, pe)
     * @endcode
     *
     * @return Returns the address corresponding to "addr" on PE "pe"
     *
     *
     */

    void *shmemx_lookup_remote_addr (void *addr, int pe);

    /*
     * non-blocking fence/quiet
     *
     */

    /**
     * @brief check whether all communication operations issued prior to
     * this call have satisfied the fence semantic.
     *
     * @section Synopsis:
     *
     * @substitute c C/C++
     * @code
     int shmemx_fence_test (void);
     * @endcode
     *
     * @subsection f Fortran
     * @code
     logical shmemx_fence_test
     * @endcode
     *
     * @return Returns non-zero if all communication operations issued
     * prior to this call have satisfied the fence semantic, 0 otherwise.
     *
     */

    int shmemx_fence_test (void);

    /**
     * @brief check whether all communication operations issued prior to
     * this call have satisfied the quiet semantic.
     *
     * @section Synopsis:
     *
     * @substitute c C/C++
     * @code
     int shmemx_quiet_test (void);
     * @endcode
     *
     * @subsection f Fortran
     * @code
     logical shmemx_quiet_test
     * @endcode
     *
     * @return Returns non-zero if all communication operations issued
     * prior to this call have satisfied the quiet semantic, 0 otherwise.
     *
     */

    int shmemx_quiet_test (void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _SHMEMX_H */
