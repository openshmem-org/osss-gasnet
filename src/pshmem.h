/*
 *
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


#ifndef _PSHMEM_H
#define _PSHMEM_H 1

#include "shmem.h"

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

    void pshmem_info_get_version (int *major, int *minor);
    void pshmem_info_get_name (char *name);

    /*
     * init & query
     */

    void pstart_pes (int npes);
    void pshmem_init (void);
    void pshmem_finalize (void);

    void pshmem_global_exit (int status);

    int p_my_pe (void) _WUR;
    int pshmem_my_pe (void) _WUR;

    int p_num_pes (void) _WUR;
    int pshmem_n_pes (void) _WUR;

    /*
     * I/O
     */

    void pshmem_short_put (short *dest, const short *src, size_t nelems,
                           int pe);
    void pshmem_int_put (int *dest, const int *src, size_t nelems,
                         int pe);
    void pshmem_long_put (long *dest, const long *src, size_t nelems,
                          int pe);
    void pshmem_longlong_put (long long *dest, const long long *src,
                              size_t nelems, int pe);
    void pshmem_longdouble_put (long double *dest,
                                const long double *src, size_t nelems,
                                int pe);
    void pshmem_double_put (double *dest, const double *src,
                            size_t nelems, int pe);
    void pshmem_float_put (float *dest, const float *src, size_t nelems,
                           int pe);
    void pshmem_putmem (void *dest, const void *src, size_t nelems,
                        int pe);
    void pshmem_put32 (void *dest, const void *src, size_t nelems,
                       int pe);
    void pshmem_put64 (void *dest, const void *src, size_t nelems,
                       int pe);
    void pshmem_put128 (void *dest, const void *src, size_t nelems,
                        int pe);

    void pshmem_short_get (short *dest, const short *src, size_t nelems,
                           int pe);
    void pshmem_int_get (int *dest, const int *src, size_t nelems,
                         int pe);
    void pshmem_long_get (long *dest, const long *src, size_t nelems,
                          int pe);
    void pshmem_longlong_get (long long *dest, const long long *src,
                              size_t nelems, int pe);
    void pshmem_longdouble_get (long double *dest,
                                const long double *src, size_t nelems,
                                int pe);
    void pshmem_double_get (double *dest, const double *src,
                            size_t nelems, int pe);
    void pshmem_float_get (float *dest, const float *src, size_t nelems,
                           int pe);
    void pshmem_getmem (void *dest, const void *src, size_t nelems,
                        int pe);
    void pshmem_get32 (void *dest, const void *src, size_t nelems,
                       int pe);
    void pshmem_get64 (void *dest, const void *src, size_t nelems,
                       int pe);
    void pshmem_get128 (void *dest, const void *src, size_t nelems,
                        int pe);

    void pshmem_char_p (char *addr, char value, int pe);
    void pshmem_short_p (short *addr, short value, int pe);
    void pshmem_int_p (int *addr, int value, int pe);
    void pshmem_long_p (long *addr, long value, int pe);
    void pshmem_longlong_p (long long *addr, long long value, int pe);
    void pshmem_float_p (float *addr, float value, int pe);
    void pshmem_double_p (double *addr, double value, int pe);
    void pshmem_longdouble_p (long double *addr, long double value,
                              int pe);

    char pshmem_char_g (char *addr, int pe) _WUR;
    short pshmem_short_g (short *addr, int pe) _WUR;
    int pshmem_int_g (int *addr, int pe) _WUR;
    long pshmem_long_g (long *addr, int pe) _WUR;
    long long pshmem_longlong_g (long long *addr, int pe) _WUR;
    float pshmem_float_g (float *addr, int pe) _WUR;
    double pshmem_double_g (double *addr, int pe) _WUR;
    long double pshmem_longdouble_g (long double *addr, int pe) _WUR;


    /*
     * strided I/O
     */

    void pshmem_double_iput (double *target, const double *source,
                             ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                             int pe);

    void pshmem_float_iput (float *target, const float *source,
                            ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                            int pe);

    void pshmem_int_iput (int *target, const int *source, ptrdiff_t tst,
                          ptrdiff_t sst, size_t nelems, int pe);

    void pshmem_iput32 (void *target, const void *source, ptrdiff_t tst,
                        ptrdiff_t sst, size_t nelems, int pe);

    void pshmem_iput64 (void *target, const void *source, ptrdiff_t tst,
                        ptrdiff_t sst, size_t nelems, int pe);

    void pshmem_iput128 (void *target, const void *source, ptrdiff_t tst,
                         ptrdiff_t sst, size_t nelems, int pe);

    void pshmem_long_iput (long *target, const long *source,
                           ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                           int pe);

    void pshmem_longdouble_iput (long double *target,
                                 const long double *source,
                                 ptrdiff_t tst, ptrdiff_t sst,
                                 size_t nelems, int pe);

    void pshmem_longlong_iput (long long *target,
                               const long long *source, ptrdiff_t tst,
                               ptrdiff_t sst, size_t nelems, int pe);

    void pshmem_short_iput (short *target, const short *source,
                            ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                            int pe);


    void pshmem_double_iget (double *target, const double *source,
                             ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                             int pe);

    void pshmem_float_iget (float *target, const float *source,
                            ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                            int pe);

    void pshmem_int_iget (int *target, const int *source, ptrdiff_t tst,
                          ptrdiff_t sst, size_t nelems, int pe);

    void pshmem_iget32 (void *target, const void *source, ptrdiff_t tst,
                        ptrdiff_t sst, size_t nelems, int pe);

    void pshmem_iget64 (void *target, const void *source, ptrdiff_t tst,
                        ptrdiff_t sst, size_t nelems, int pe);

    void pshmem_iget128 (void *target, const void *source, ptrdiff_t tst,
                         ptrdiff_t sst, size_t nelems, int pe);

    void pshmem_long_iget (long *target, const long *source,
                           ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                           int pe);

    void pshmem_longdouble_iget (long double *target,
                                 const long double *source,
                                 ptrdiff_t tst, ptrdiff_t sst,
                                 size_t nelems, int pe);

    void pshmem_longlong_iget (long long *target,
                               const long long *source, ptrdiff_t tst,
                               ptrdiff_t sst, size_t nelems, int pe);

    void pshmem_short_iget (short *target, const short *source,
                            ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                            int pe);

    /*
     * non-blocking implicit put/get
     *
     */
    void pshmem_double_put_nbi (double *dest, const double *source,
                               size_t nelems, int pe);
    void pshmem_float_put_nbi (float *dest, const float *source, size_t nelems,
                              int pe);
    void pshmem_char_put_nbi (char *dest, const char *source, size_t nelems,
                            int pe);
    void pshmem_int_put_nbi (int *dest, const int *source, size_t nelems,
                            int pe);
    void pshmem_long_put_nbi (long *dest, const long *source, size_t nelems,
                             int pe);
    void pshmem_longdouble_put_nbi (long double *dest, const long double *source,
                                   size_t nelems, int pe);
    void pshmem_longlong_put_nbi (long long *dest, const long long *source,
                                 size_t nelems, int pe);
    void pshmem_put32_nbi (void *dest, const void *source, size_t nelems,
                          int pe);
    void pshmem_put64_nbi (void *dest, const void *source, size_t nelems,
                          int pe);
    void pshmem_put128_nbi (void *dest, const void *source, size_t nelems,
                           int pe);
    void pshmem_putmem_nbi (void *dest, const void *source, size_t nelems,
                           int pe);
    void pshmem_short_put_nbi (short *dest, const short *source, size_t nelems,
                              int pe);

    void pshmem_double_get_nbi (double *dest, const double *source,
                               size_t nelems, int pe);
    void pshmem_float_get_nbi (float *dest, const float *source, size_t nelems,
                              int pe);
    void pshmem_get32_nbi (void *dest, const void *source, size_t nelems,
                          int pe);
    void pshmem_get64_nbi (void *dest, const void *source, size_t nelems,
                          int pe);
    void pshmem_get128_nbi (void *dest, const void *source, size_t nelems,
                           int pe);
    void pshmem_getmem_nbi (void *dest, const void *source, size_t nelems,
                           int pe);
    void pshmem_int_get_nbi (int *dest, const int *source, size_t nelems,
                            int pe);
    void pshmem_char_get_nbi (char *dest, const char *source, size_t nelems,
                            int pe);
    void pshmem_long_get_nbi (long *dest, const long *source, size_t nelems,
                             int pe);
    void pshmem_longdouble_get_nbi (long double *dest, const long double *source,
                                   size_t nelems, int pe);
    void pshmem_longlong_get_nbi (long long *dest, const long long *source,
                                 size_t nelems, int pe);
    void pshmem_short_get_nbi (short *dest, const short *source, size_t nelems,
                              int pe);

#ifdef __STDC_VERSION__
#if  __STDC_VERSION__ >= 201112L

    /*
     * C11 Generic variants
     *
     */

    /* see \ref shmem_long_put () */
#define pshmem_put(dest, source, nelems, pe)                            \
    _Generic(*(dest),                                                   \
             float:       pshmem_float_put,                             \
             double:      pshmem_double_put,                            \
             long double: pshmem_longdouble_put,                        \
             char:        pshmem_char_put,                              \
             short:       pshmem_short_put,                             \
             int:         pshmem_int_put,                               \
             long:        pshmem_long_put,                              \
             long long:   pshmem_longlong_put) (dest, source, nelems, pe)

    /* see \ref shmem_long_get () */
#define pshmem_get(dest, source, nelems, pe)                            \
    _Generic(*(dest),                                                   \
             float:       pshmem_float_get,                             \
             double:      pshmem_double_get,                            \
             long double: pshmem_longdouble_get,                        \
             char:        pshmem_char_get,                              \
             short:       pshmem_short_get,                             \
             int:         pshmem_int_get,                               \
             long:        pshmem_long_get,                              \
             long long:   pshmem_longlong_get) (dest, source, nelems, pe)

    /* see \ref shmem_long_p () */
#define pshmem_p(dest, value, pe)                               \
    _Generic(*(dest),                                           \
             float:       pshmem_float_p,                       \
             double:      pshmem_double_p,                      \
             long double: pshmem_longdouble_p,                  \
             char:        pshmem_char_p,                        \
             short:       pshmem_short_p,                       \
             int:         pshmem_int_p,                         \
             long:        pshmem_long_p,                        \
             long long:   pshmem_longlong_p) (dest, value, pe)


    /* see \ref shmem_long_g () */
#define pshmem_g(addr, pe)                              \
    _Generic((addr),                                    \
             float:       pshmem_float_g,               \
             double:      pshmem_double_g,              \
             long double: pshmem_longdouble_g,          \
             char:        pshmem_char_g,                \
             short:       pshmem_short_g,               \
             int:         pshmem_int_g,                 \
             long:        pshmem_long_g,                \
             long long:   pshmem_longlong_g) (addr, pe)

    /* see \ref shmem_long_iput () */
#define pshmem_iput(dest, source, dst, sst, nelems, pe)                 \
    _Generic(*(dest),                                                   \
             float:       pshmem_float_iput,                            \
             double:      pshmem_double_iput,                           \
             long double: pshmem_longdouble_iput,                       \
             char:        pshmem_char_iput,                             \
             short:       pshmem_short_iput,                            \
             int:         pshmem_int_iput,                              \
             long:        pshmem_long_iput,                             \
             long long:   pshmem_longlong_iput) (dest, source, dst, sst, \
                                                 nelems, pe)

    /* see \ref shmem_long_iput () */
#define pshmem_iput(dest, source, dst, sst, nelems, pe)                 \
    _Generic(*(dest),                                                   \
             float:       pshmem_float_iput,                            \
             double:      pshmem_double_iput,                           \
             long double: pshmem_longdouble_iput,                       \
             char:        pshmem_char_iput,                             \
             short:       pshmem_short_iput,                            \
             int:         pshmem_int_iput,                              \
             long:        pshmem_long_iput,                             \
             long long:   pshmem_longlong_iput) (dest, source, dst, sst, \
                                                 nelems, pe)

    /* see \ref shmem_long_swap () */
#define pshmem_swap(dest, value, pe)                                \
    _Generic(*(dest),                                               \
             int:          pshmem_int_swap,                         \
             long:         pshmem_long_swap,                        \
             long long:    pshmem_longlong_swap,                    \
             float:        pshmem_float_swap,                       \
             double:       pshmem_double_swap) (dest, value, pe)

    /* see \ref shmem_long_cswap () */
#define pshmem_cswap(dest, cond, value, pe)                             \
    _Generic(*(dest),                                                   \
             int:          pshmem_int_cswap,                            \
             long:         pshmem_long_cswap,                           \
             long long:    pshmem_longlong_cswap) (dest, cond, value, pe)

    /* see \ref shmem_long_fadd () */
#define pshmem_fadd(dest, value, pe)                                \
    _Generic(*(dest),                                               \
             int:          pshmem_int_fadd,                         \
             long:         pshmem_long_fadd,                        \
             long long:    pshmem_longlong_fadd) (dest, value, pe)


    /* see \ref shmem_long_finc () */
#define pshmem_finc(dest, pe)                               \
    _Generic(*(dest),                                       \
             int:          pshmem_int_finc,                 \
             long:         pshmem_long_finc,                \
             long long:    pshmem_longlong_finc) (dest, pe)

    /* see \ref shmem_long_add () */
#define pshmem_add(dest, value, pe)                                 \
    _Generic(*(dest),                                               \
             int:          pshmem_int_add,                          \
             long:         pshmem_long_add,                         \
             long long:    pshmem_longlong_add) (dest, value, pe)

    /* see \ref shmem_long_add () */
#define pshmem_add(dest, value, pe)                                 \
    _Generic(*(dest),                                               \
             int:          pshmem_int_add,                          \
             long:         pshmem_long_add,                         \
             long long:    pshmem_longlong_add) (dest, value, pe)

    /* see \ref shmem_long_inc () */
#define pshmem_inc(dest, pe)                                \
    _Generic(*(dest),                                       \
             int:          pshmem_int_inc,                  \
             long:         pshmem_long_inc,                 \
             long long:    pshmem_longlong_inc) (dest, pe)

    /* see \ref shmem_long_fetch () */
#define pshmem_fetch(dest, pe)                              \
    _Generic(*(dest),                                       \
             int:          pshmem_int_fetch,                \
             const int:    pshmem_int_fetch,                \
             long:         pshmem_long_fetch,               \
             const long:   pshmem_long_fetch,               \
             long long:    pshmem_longlong_fetch,           \
             const long long: pshmem_longlong_fetch,        \
             float:        pshmem_float_fetch,              \
             const float:  pshmem_float_fetch,              \
             double:       pshmem_double_fetch,             \
             const double: pshmem_double_fetch) (dest, pe)

    /* see \ref shmem_long_set () */
#define pshmem_set(dest, value, pe)                             \
    _Generic(*(dest),                                           \
             int:          pshmem_int_set,                      \
             long:         pshmem_long_set,                     \
             long long:    pshmem_longlong_set,                 \
             float:        pshmem_float_set,                    \
             double:       pshmem_double_set) (dest, value, pe)

#endif   /* __STDC_VERSION__ >= 201112L test */
#endif /* __STDC_VERSION__ defined test */

    /*
     * barriers
     */

    void pshmem_barrier_all (void);
    void pshmem_barrier (int PE_start, int logPE_stride, int PE_size,
                         long *pSync);
    void pshmem_fence (void);
    void pshmem_quiet (void);

    /*
     * accessibility
     */

    int pshmem_pe_accessible (int pe) _WUR;
    int pshmem_addr_accessible (const void *addr, int pe) _WUR;

    /*
     * symmetric memory management
     */

    void *pshmalloc (size_t size) _WUR;
    void pshfree (void *ptr);
    void *pshrealloc (void *ptr, size_t size) _WUR;
    void *pshmemalign (size_t alignment, size_t size) _WUR;

    void *pshmem_malloc (size_t size) _WUR;
    void pshmem_free (void *ptr);
    void *pshmem_realloc (void *ptr, size_t size) _WUR;
    void *pshmem_memalign (size_t alignment, size_t size) _WUR;

    /*
     * wait operations
     */

    void pshmem_short_wait_until (volatile short *ivar, int cmp,
                                  short cmp_value);
    void pshmem_int_wait_until (volatile int *ivar, int cmp, int cmp_value);
    void pshmem_long_wait_until (volatile long *ivar, int cmp, long cmp_value);
    void pshmem_longlong_wait_until (volatile long long *ivar, int cmp,
                                            long long cmp_value);
    void pshmem_wait_until (volatile long *ivar, int cmp, long cmp_value);

    void pshmem_short_wait (volatile short *ivar, short cmp_value);
    void pshmem_int_wait (volatile int *ivar, int cmp_value);
    void pshmem_long_wait (volatile long *ivar, long cmp_value);
    void pshmem_longlong_wait (volatile long long *ivar, long long cmp_value);
    void pshmem_wait (volatile long *ivar, long cmp_value);

    /*
     * atomic swaps
     */

    int pshmem_int_swap (int *target, int value, int pe) _WUR;
    long pshmem_long_swap (long *target, long value, int pe) _WUR;
    long long pshmem_longlong_swap (long long *target, long long value,
                                           int pe) _WUR;
    float pshmem_float_swap (float *target, float value, int pe) _WUR;
    double pshmem_double_swap (double *target, double value,
                                      int pe) _WUR;
    int pshmem_int_cswap (int *target, int cond, int value, int pe) _WUR;
    long pshmem_long_cswap (long *target, long cond, long value,
                                   int pe) _WUR;
    long long pshmem_longlong_cswap (long long *target, long long cond,
                                            long long value, int pe) _WUR;

    /*
     * atomic fetch-{add,inc} & add,inc
     */

    int pshmem_int_fadd (int *target, int value, int pe) _WUR;
    long pshmem_long_fadd (long *target, long value, int pe) _WUR;
    long long pshmem_longlong_fadd (long long *target, long long value,
                                           int pe) _WUR;
    int pshmem_int_finc (int *target, int pe) _WUR;
    long pshmem_long_finc (long *target, int pe) _WUR;
    long long pshmem_longlong_finc (long long *target, int pe) _WUR;

    void pshmem_int_add (int *target, int value, int pe);
    void pshmem_long_add (long *target, long value, int pe);
    void pshmem_longlong_add (long long *target, long long value,
                                     int pe);
    void pshmem_int_inc (int *target, int pe);
    void pshmem_long_inc (long *target, int pe);
    void pshmem_longlong_inc (long long *target, int pe);

    /*
     * cache flushing
     */

    void pshmem_clear_cache_inv (void);
    void pshmem_set_cache_inv (void);
    void pshmem_clear_cache_line_inv (void *target);
    void pshmem_set_cache_line_inv (void *target);
    void pshmem_udcflush (void);
    void pshmem_udcflush_line (void *target);

    /*
     * reductions
     */

    void pshmem_complexd_sum_to_all (COMPLEXIFY (double) * target,
                                            COMPLEXIFY (double) * source,
                                            int nreduce,
                                            int PE_start, int logPE_stride,
                                            int PE_size,
                                            COMPLEXIFY (double) * pWrk,
                                            long *pSync);
    void pshmem_complexf_sum_to_all (COMPLEXIFY (float) * target,
                                            COMPLEXIFY (float) * source,
                                            int nreduce, int PE_start,
                                            int logPE_stride, int PE_size,
                                            COMPLEXIFY (float) * pWrk,
                                            long *pSync);
    void pshmem_double_sum_to_all (double *target, double *source,
                                          int nreduce, int PE_start,
                                          int logPE_stride, int PE_size,
                                          double *pWrk, long *pSync);
    void pshmem_float_sum_to_all (float *target, float *source,
                                         int nreduce, int PE_start,
                                         int logPE_stride, int PE_size,
                                         float *pWrk, long *pSync);
    void pshmem_int_sum_to_all (int *target, int *source, int nreduce,
                                       int PE_start, int logPE_stride,
                                       int PE_size, int *pWrk, long *pSync);
    void pshmem_long_sum_to_all (long *target, long *source, int nreduce,
                                        int PE_start, int logPE_stride,
                                        int PE_size, long *pWrk, long *pSync);
    void pshmem_longdouble_sum_to_all (long double *target,
                                              long double *source, int nreduce,
                                              int PE_start, int logPE_stride,
                                              int PE_size, long double *pWrk,
                                              long *pSync);
    void pshmem_longlong_sum_to_all (long long *target,
                                            long long *source, int nreduce,
                                            int PE_start, int logPE_stride,
                                            int PE_size, long long *pWrk,
                                            long *pSync);
    void pshmem_short_sum_to_all (short *target, short *source,
                                         int nreduce, int PE_start,
                                         int logPE_stride, int PE_size,
                                         short *pWrk, long *pSync);

    void pshmem_complexd_prod_to_all (COMPLEXIFY (double) * target,
                                             COMPLEXIFY (double) * source,
                                             int nreduce,
                                             int PE_start, int logPE_stride,
                                             int PE_size,
                                             COMPLEXIFY (double) * pWrk,
                                             long *pSync);
    void pshmem_complexf_prod_to_all (COMPLEXIFY (float) * target,
                                             COMPLEXIFY (float) * source,
                                             int nreduce, int PE_start,
                                             int logPE_stride, int PE_size,
                                             COMPLEXIFY (float) * pWrk,
                                             long *pSync);
    void pshmem_double_prod_to_all (double *target, double *source,
                                           int nreduce, int PE_start,
                                           int logPE_stride, int PE_size,
                                           double *pWrk, long *pSync);
    void pshmem_float_prod_to_all (float *target, float *source,
                                          int nreduce, int PE_start,
                                          int logPE_stride, int PE_size,
                                          float *pWrk, long *pSync);
    void pshmem_int_prod_to_all (int *target, int *source, int nreduce,
                                        int PE_start, int logPE_stride,
                                        int PE_size, int *pWrk, long *pSync);
    void pshmem_long_prod_to_all (long *target, long *source,
                                         int nreduce, int PE_start,
                                         int logPE_stride, int PE_size,
                                         long *pWrk, long *pSync);
    void pshmem_longdouble_prod_to_all (long double *target,
                                               long double *source, int nreduce,
                                               int PE_start, int logPE_stride,
                                               int PE_size, long double *pWrk,
                                               long *pSync);
    void pshmem_longlong_prod_to_all (long long *target,
                                             long long *source, int nreduce,
                                             int PE_start, int logPE_stride,
                                             int PE_size, long long *pWrk,
                                             long *pSync);
    void pshmem_short_prod_to_all (short *target, short *source,
                                          int nreduce, int PE_start,
                                          int logPE_stride, int PE_size,
                                          short *pWrk, long *pSync);

    void pshmem_int_and_to_all (int *target,
                                       int *source,
                                       int nreduce,
                                       int PE_start, int logPE_stride,
                                       int PE_size, int *pWrk, long *pSync);
    void pshmem_long_and_to_all (long *target, long *source, int nreduce,
                                        int PE_start, int logPE_stride,
                                        int PE_size, long *pWrk, long *pSync);
    void pshmem_longlong_and_to_all (long long *target,
                                            long long *source, int nreduce,
                                            int PE_start, int logPE_stride,
                                            int PE_size, long long *pWrk,
                                            long *pSync);
    void pshmem_short_and_to_all (short *target, short *source,
                                         int nreduce, int PE_start,
                                         int logPE_stride, int PE_size,
                                         short *pWrk, long *pSync);

    void pshmem_int_or_to_all (int *target,
                                      int *source,
                                      int nreduce,
                                      int PE_start, int logPE_stride,
                                      int PE_size, int *pWrk, long *pSync);
    void pshmem_long_or_to_all (long *target, long *source, int nreduce,
                                       int PE_start, int logPE_stride,
                                       int PE_size, long *pWrk, long *pSync);
    void pshmem_longlong_or_to_all (long long *target, long long *source,
                                           int nreduce, int PE_start,
                                           int logPE_stride, int PE_size,
                                           long long *pWrk, long *pSync);
    void pshmem_short_or_to_all (short *target, short *source,
                                        int nreduce, int PE_start,
                                        int logPE_stride, int PE_size,
                                        short *pWrk, long *pSync);

    void pshmem_int_xor_to_all (int *target,
                                       int *source,
                                       int nreduce,
                                       int PE_start, int logPE_stride,
                                       int PE_size, int *pWrk, long *pSync);
    void pshmem_long_xor_to_all (long *target, long *source, int nreduce,
                                        int PE_start, int logPE_stride,
                                        int PE_size, long *pWrk, long *pSync);
    void pshmem_longlong_xor_to_all (long long *target,
                                            long long *source, int nreduce,
                                            int PE_start, int logPE_stride,
                                            int PE_size, long long *pWrk,
                                            long *pSync);
    void pshmem_short_xor_to_all (short *target, short *source,
                                         int nreduce, int PE_start,
                                         int logPE_stride, int PE_size,
                                         short *pWrk, long *pSync);

    void pshmem_int_max_to_all (int *target,
                                       int *source,
                                       int nreduce,
                                       int PE_start, int logPE_stride,
                                       int PE_size, int *pWrk, long *pSync);
    void pshmem_long_max_to_all (long *target, long *source, int nreduce,
                                        int PE_start, int logPE_stride,
                                        int PE_size, long *pWrk, long *pSync);
    void pshmem_longlong_max_to_all (long long *target,
                                            long long *source, int nreduce,
                                            int PE_start, int logPE_stride,
                                            int PE_size, long long *pWrk,
                                            long *pSync);
    void pshmem_short_max_to_all (short *target, short *source,
                                         int nreduce, int PE_start,
                                         int logPE_stride, int PE_size,
                                         short *pWrk, long *pSync);
    void pshmem_longdouble_max_to_all (long double *target,
                                              long double *source, int nreduce,
                                              int PE_start, int logPE_stride,
                                              int PE_size, long double *pWrk,
                                              long *pSync);
    void pshmem_float_max_to_all (float *target, float *source,
                                         int nreduce, int PE_start,
                                         int logPE_stride, int PE_size,
                                         float *pWrk, long *pSync);
    void pshmem_double_max_to_all (double *target, double *source,
                                          int nreduce, int PE_start,
                                          int logPE_stride, int PE_size,
                                          double *pWrk, long *pSync);

    void pshmem_int_min_to_all (int *target,
                                       int *source,
                                       int nreduce,
                                       int PE_start, int logPE_stride,
                                       int PE_size, int *pWrk, long *pSync);
    void pshmem_long_min_to_all (long *target, long *source, int nreduce,
                                        int PE_start, int logPE_stride,
                                        int PE_size, long *pWrk, long *pSync);
    void pshmem_longlong_min_to_all (long long *target,
                                            long long *source, int nreduce,
                                            int PE_start, int logPE_stride,
                                            int PE_size, long long *pWrk,
                                            long *pSync);
    void pshmem_short_min_to_all (short *target, short *source,
                                         int nreduce, int PE_start,
                                         int logPE_stride, int PE_size,
                                         short *pWrk, long *pSync);
    void pshmem_longdouble_min_to_all (long double *target,
                                              long double *source, int nreduce,
                                              int PE_start, int logPE_stride,
                                              int PE_size, long double *pWrk,
                                              long *pSync);
    void pshmem_float_min_to_all (float *target, float *source,
                                         int nreduce, int PE_start,
                                         int logPE_stride, int PE_size,
                                         float *pWrk, long *pSync);
    void pshmem_double_min_to_all (double *target, double *source,
                                          int nreduce, int PE_start,
                                          int logPE_stride, int PE_size,
                                          double *pWrk, long *pSync);

    /*
     * broadcasts
     */

    void pshmem_broadcast32 (void *target, const void *source,
                                    size_t nelems, int PE_root, int PE_start,
                                    int logPE_stride, int PE_size, long *pSync);

    void pshmem_broadcast64 (void *target, const void *source,
                                    size_t nelems, int PE_root, int PE_start,
                                    int logPE_stride, int PE_size, long *pSync);

    /*
     * collects
     */

    void pshmem_fcollect32 (void *target, const void *source,
                                   size_t nelems, int PE_start,
                                   int logPE_stride, int PE_size, long *pSync);
    void pshmem_fcollect64 (void *target, const void *source,
                                   size_t nelems, int PE_start,
                                   int logPE_stride, int PE_size, long *pSync);

    void pshmem_collect32 (void *target, const void *source,
                                  size_t nelems, int PE_start, int logPE_stride,
                                  int PE_size, long *pSync);
    void pshmem_collect64 (void *target, const void *source,
                                  size_t nelems, int PE_start, int logPE_stride,
                                  int PE_size, long *pSync);

    /*
     * locks/critical section
     */

    void pshmem_set_lock (volatile long *lock);
    void pshmem_clear_lock (volatile long *lock);
    int  pshmem_test_lock (volatile long *lock) _WUR;

    /*
     * atomic fetch and set
     */

    int pshmem_int_fetch (int *target, int pe);
    long pshmem_long_fetch (long *target, int pe);
    long long pshmem_longlong_fetch (long long *target, int pe);
    float pshmem_float_fetch (float *target, int pe);
    double pshmem_double_fetch (double *target, int pe);

    void pshmem_int_set (int *target, int value, int pe);
    void pshmem_long_set (long *target, long value, int pe);
    void pshmem_longlong_set (long long *target, long long value, int pe);
    void pshmem_float_set (float *target, float value, int pe);
    void pshmem_double_set (double *target, double value, int pe);


    /*
     * --end--
     */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* _PSHMEM_H */
