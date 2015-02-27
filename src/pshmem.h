/*
 *
 * Copyright (c) 2011 - 2015
 *   University of Houston System and Oak Ridge National Laboratory.
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
 * o Neither the name of the University of Houston System, Oak Ridge
 *   National Laboratory nor the names of its contributors may be used to
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
#endif	/* __cplusplus */

  extern void pshmem_info_get_version (int *major, int *minor);
  extern void pshmem_info_get_name (char *name);

  /*
   * init & query
   */

  extern void pstart_pes (int npes);
  extern void pshmem_init (void);
  extern void pshmem_finalize (void);

  extern void pshmem_global_exit (int status);

  extern int p_my_pe (void) _WUR;
  extern int pshmem_my_pe (void) _WUR;

  extern int p_num_pes (void) _WUR;
  extern int pshmem_n_pes (void) _WUR;

  /*
   * I/O
   */

  extern void pshmem_short_put (short *dest, const short *src, size_t nelems,
                                int pe);
  extern void pshmem_int_put (int *dest, const int *src, size_t nelems,
                              int pe);
  extern void pshmem_long_put (long *dest, const long *src, size_t nelems,
                               int pe);
  extern void pshmem_longlong_put (long long *dest, const long long *src,
                                   size_t nelems, int pe);
  extern void pshmem_longdouble_put (long double *dest,
                                     const long double *src, size_t nelems,
                                     int pe);
  extern void pshmem_double_put (double *dest, const double *src,
                                 size_t nelems, int pe);
  extern void pshmem_float_put (float *dest, const float *src, size_t nelems,
                                int pe);
  extern void pshmem_putmem (void *dest, const void *src, size_t nelems,
                             int pe);
  extern void pshmem_put32 (void *dest, const void *src, size_t nelems,
                            int pe);
  extern void pshmem_put64 (void *dest, const void *src, size_t nelems,
                            int pe);
  extern void pshmem_put128 (void *dest, const void *src, size_t nelems,
                             int pe);

  extern void pshmem_short_get (short *dest, const short *src, size_t nelems,
                                int pe);
  extern void pshmem_int_get (int *dest, const int *src, size_t nelems,
                              int pe);
  extern void pshmem_long_get (long *dest, const long *src, size_t nelems,
                               int pe);
  extern void pshmem_longlong_get (long long *dest, const long long *src,
                                   size_t nelems, int pe);
  extern void pshmem_longdouble_get (long double *dest,
                                     const long double *src, size_t nelems,
                                     int pe);
  extern void pshmem_double_get (double *dest, const double *src,
                                 size_t nelems, int pe);
  extern void pshmem_float_get (float *dest, const float *src, size_t nelems,
                                int pe);
  extern void pshmem_getmem (void *dest, const void *src, size_t nelems,
                             int pe);
  extern void pshmem_get32 (void *dest, const void *src, size_t nelems,
                            int pe);
  extern void pshmem_get64 (void *dest, const void *src, size_t nelems,
                            int pe);
  extern void pshmem_get128 (void *dest, const void *src, size_t nelems,
                             int pe);

  extern void pshmem_char_p (char *addr, char value, int pe);
  extern void pshmem_short_p (short *addr, short value, int pe);
  extern void pshmem_int_p (int *addr, int value, int pe);
  extern void pshmem_long_p (long *addr, long value, int pe);
  extern void pshmem_longlong_p (long long *addr, long long value, int pe);
  extern void pshmem_float_p (float *addr, float value, int pe);
  extern void pshmem_double_p (double *addr, double value, int pe);
  extern void pshmem_longdouble_p (long double *addr, long double value,
                                   int pe);

  extern char pshmem_char_g (char *addr, int pe) _WUR;
  extern short pshmem_short_g (short *addr, int pe) _WUR;
  extern int pshmem_int_g (int *addr, int pe) _WUR;
  extern long pshmem_long_g (long *addr, int pe) _WUR;
  extern long long pshmem_longlong_g (long long *addr, int pe) _WUR;
  extern float pshmem_float_g (float *addr, int pe) _WUR;
  extern double pshmem_double_g (double *addr, int pe) _WUR;
  extern long double pshmem_longdouble_g (long double *addr, int pe) _WUR;


  /*
   * strided I/O
   */

  extern void pshmem_double_iput (double *target, const double *source,
                                  ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                                  int pe);

  extern void pshmem_float_iput (float *target, const float *source,
                                 ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                                 int pe);

  extern void pshmem_int_iput (int *target, const int *source, ptrdiff_t tst,
                               ptrdiff_t sst, size_t nelems, int pe);

  extern void pshmem_iput32 (void *target, const void *source, ptrdiff_t tst,
                             ptrdiff_t sst, size_t nelems, int pe);

  extern void pshmem_iput64 (void *target, const void *source, ptrdiff_t tst,
                             ptrdiff_t sst, size_t nelems, int pe);

  extern void pshmem_iput128 (void *target, const void *source, ptrdiff_t tst,
                              ptrdiff_t sst, size_t nelems, int pe);

  extern void pshmem_long_iput (long *target, const long *source,
                                ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                                int pe);

  extern void pshmem_longdouble_iput (long double *target,
                                      const long double *source,
                                      ptrdiff_t tst, ptrdiff_t sst,
                                      size_t nelems, int pe);

  extern void pshmem_longlong_iput (long long *target,
                                    const long long *source, ptrdiff_t tst,
                                    ptrdiff_t sst, size_t nelems, int pe);

  extern void pshmem_short_iput (short *target, const short *source,
                                 ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                                 int pe);


  extern void pshmem_double_iget (double *target, const double *source,
                                  ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                                  int pe);

  extern void pshmem_float_iget (float *target, const float *source,
                                 ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                                 int pe);

  extern void pshmem_int_iget (int *target, const int *source, ptrdiff_t tst,
                               ptrdiff_t sst, size_t nelems, int pe);

  extern void pshmem_iget32 (void *target, const void *source, ptrdiff_t tst,
                             ptrdiff_t sst, size_t nelems, int pe);

  extern void pshmem_iget64 (void *target, const void *source, ptrdiff_t tst,
                             ptrdiff_t sst, size_t nelems, int pe);

  extern void pshmem_iget128 (void *target, const void *source, ptrdiff_t tst,
                              ptrdiff_t sst, size_t nelems, int pe);

  extern void pshmem_long_iget (long *target, const long *source,
                                ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                                int pe);

  extern void pshmem_longdouble_iget (long double *target,
                                      const long double *source,
                                      ptrdiff_t tst, ptrdiff_t sst,
                                      size_t nelems, int pe);

  extern void pshmem_longlong_iget (long long *target,
                                    const long long *source, ptrdiff_t tst,
                                    ptrdiff_t sst, size_t nelems, int pe);

  extern void pshmem_short_iget (short *target, const short *source,
                                 ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                                 int pe);


  /*
   * barriers
   */

  extern void pshmem_barrier_all (void);
  extern void pshmem_barrier (int PE_start, int logPE_stride, int PE_size,
                              long *pSync);
  extern void pshmem_fence (void);
  extern void pshmem_quiet (void);

  /*
   * accessibility
   */

  extern int pshmem_pe_accessible (int pe) _WUR;
  extern int pshmem_addr_accessible (void *addr, int pe) _WUR;

  /*
   * symmetric memory management
   */

  extern void *pshmalloc (size_t size) _WUR;
  extern void pshfree (void *ptr);
  extern void *pshrealloc (void *ptr, size_t size) _WUR;
  extern void *pshmemalign (size_t alignment, size_t size) _WUR;

  extern void *pshmem_malloc (size_t size) _WUR;
  extern void pshmem_free (void *ptr);
  extern void *pshmem_realloc (void *ptr, size_t size) _WUR;
  extern void *pshmem_memalign (size_t alignment, size_t size) _WUR;

  /*
   * wait operations
   */

  extern void pshmem_short_wait_until (short *ivar, int cmp, short cmp_value);
  extern void pshmem_int_wait_until (int *ivar, int cmp, int cmp_value);
  extern void pshmem_long_wait_until (long *ivar, int cmp, long cmp_value);
  extern void pshmem_longlong_wait_until (long long *ivar, int cmp,
                                          long long cmp_value);
  extern void pshmem_wait_until (long *ivar, int cmp, long cmp_value);

  extern void pshmem_short_wait (short *ivar, short cmp_value);
  extern void pshmem_int_wait (int *ivar, int cmp_value);
  extern void pshmem_long_wait (long *ivar, long cmp_value);
  extern void pshmem_longlong_wait (long long *ivar, long long cmp_value);
  extern void pshmem_wait (long *ivar, long cmp_value);

  /*
   * atomic swaps
   */

  extern int pshmem_int_swap (int *target, int value, int pe) _WUR;
  extern long pshmem_long_swap (long *target, long value, int pe) _WUR;
  extern long long pshmem_longlong_swap (long long *target, long long value,
                                         int pe) _WUR;
  extern float pshmem_float_swap (float *target, float value, int pe) _WUR;
  extern double pshmem_double_swap (double *target, double value,
                                    int pe) _WUR;
  extern long pshmem_swap (long *target, long value, int pe) _WUR;

  extern int pshmem_int_cswap (int *target, int cond, int value, int pe) _WUR;
  extern long pshmem_long_cswap (long *target, long cond, long value,
                                 int pe) _WUR;
  extern long long pshmem_longlong_cswap (long long *target, long long cond,
                                          long long value, int pe) _WUR;

  /*
   * atomic fetch-{add,inc} & add,inc
   */

  extern int pshmem_int_fadd (int *target, int value, int pe) _WUR;
  extern long pshmem_long_fadd (long *target, long value, int pe) _WUR;
  extern long long pshmem_longlong_fadd (long long *target, long long value,
                                         int pe) _WUR;
  extern int pshmem_int_finc (int *target, int pe) _WUR;
  extern long pshmem_long_finc (long *target, int pe) _WUR;
  extern long long pshmem_longlong_finc (long long *target, int pe) _WUR;

  extern void pshmem_int_add (int *target, int value, int pe);
  extern void pshmem_long_add (long *target, long value, int pe);
  extern void pshmem_longlong_add (long long *target, long long value,
                                   int pe);
  extern void pshmem_int_inc (int *target, int pe);
  extern void pshmem_long_inc (long *target, int pe);
  extern void pshmem_longlong_inc (long long *target, int pe);

  /*
   * cache flushing
   */

  extern void pshmem_clear_cache_inv (void);
  extern void pshmem_set_cache_inv (void);
  extern void pshmem_clear_cache_line_inv (void *target);
  extern void pshmem_set_cache_line_inv (void *target);
  extern void pshmem_udcflush (void);
  extern void pshmem_udcflush_line (void *target);

  /*
   * reductions
   */

  extern void pshmem_complexd_sum_to_all (COMPLEXIFY (double) * target,
                                          COMPLEXIFY (double) * source,
                                          int nreduce,
                                          int PE_start, int logPE_stride,
                                          int PE_size,
                                          COMPLEXIFY (double) * pWrk,
                                          long *pSync);
  extern void pshmem_complexf_sum_to_all (COMPLEXIFY (float) * target,
                                          COMPLEXIFY (float) * source,
                                          int nreduce, int PE_start,
                                          int logPE_stride, int PE_size,
                                          COMPLEXIFY (float) * pWrk,
                                          long *pSync);
  extern void pshmem_double_sum_to_all (double *target, double *source,
                                        int nreduce, int PE_start,
                                        int logPE_stride, int PE_size,
                                        double *pWrk, long *pSync);
  extern void pshmem_float_sum_to_all (float *target, float *source,
                                       int nreduce, int PE_start,
                                       int logPE_stride, int PE_size,
                                       float *pWrk, long *pSync);
  extern void pshmem_int_sum_to_all (int *target, int *source, int nreduce,
                                     int PE_start, int logPE_stride,
                                     int PE_size, int *pWrk, long *pSync);
  extern void pshmem_long_sum_to_all (long *target, long *source, int nreduce,
                                      int PE_start, int logPE_stride,
                                      int PE_size, long *pWrk, long *pSync);
  extern void pshmem_longdouble_sum_to_all (long double *target,
                                            long double *source, int nreduce,
                                            int PE_start, int logPE_stride,
                                            int PE_size, long double *pWrk,
                                            long *pSync);
  extern void pshmem_longlong_sum_to_all (long long *target,
                                          long long *source, int nreduce,
                                          int PE_start, int logPE_stride,
                                          int PE_size, long long *pWrk,
                                          long *pSync);
  extern void pshmem_short_sum_to_all (short *target, short *source,
                                       int nreduce, int PE_start,
                                       int logPE_stride, int PE_size,
                                       short *pWrk, long *pSync);

  extern void pshmem_complexd_prod_to_all (COMPLEXIFY (double) * target,
                                           COMPLEXIFY (double) * source,
                                           int nreduce,
                                           int PE_start, int logPE_stride,
                                           int PE_size,
                                           COMPLEXIFY (double) * pWrk,
                                           long *pSync);
  extern void pshmem_complexf_prod_to_all (COMPLEXIFY (float) * target,
                                           COMPLEXIFY (float) * source,
                                           int nreduce, int PE_start,
                                           int logPE_stride, int PE_size,
                                           COMPLEXIFY (float) * pWrk,
                                           long *pSync);
  extern void pshmem_double_prod_to_all (double *target, double *source,
                                         int nreduce, int PE_start,
                                         int logPE_stride, int PE_size,
                                         double *pWrk, long *pSync);
  extern void pshmem_float_prod_to_all (float *target, float *source,
                                        int nreduce, int PE_start,
                                        int logPE_stride, int PE_size,
                                        float *pWrk, long *pSync);
  extern void pshmem_int_prod_to_all (int *target, int *source, int nreduce,
                                      int PE_start, int logPE_stride,
                                      int PE_size, int *pWrk, long *pSync);
  extern void pshmem_long_prod_to_all (long *target, long *source,
                                       int nreduce, int PE_start,
                                       int logPE_stride, int PE_size,
                                       long *pWrk, long *pSync);
  extern void pshmem_longdouble_prod_to_all (long double *target,
                                             long double *source, int nreduce,
                                             int PE_start, int logPE_stride,
                                             int PE_size, long double *pWrk,
                                             long *pSync);
  extern void pshmem_longlong_prod_to_all (long long *target,
                                           long long *source, int nreduce,
                                           int PE_start, int logPE_stride,
                                           int PE_size, long long *pWrk,
                                           long *pSync);
  extern void pshmem_short_prod_to_all (short *target, short *source,
                                        int nreduce, int PE_start,
                                        int logPE_stride, int PE_size,
                                        short *pWrk, long *pSync);

  extern void pshmem_int_and_to_all (int *target,
                                     int *source,
                                     int nreduce,
                                     int PE_start, int logPE_stride,
                                     int PE_size, int *pWrk, long *pSync);
  extern void pshmem_long_and_to_all (long *target, long *source, int nreduce,
                                      int PE_start, int logPE_stride,
                                      int PE_size, long *pWrk, long *pSync);
  extern void pshmem_longlong_and_to_all (long long *target,
                                          long long *source, int nreduce,
                                          int PE_start, int logPE_stride,
                                          int PE_size, long long *pWrk,
                                          long *pSync);
  extern void pshmem_short_and_to_all (short *target, short *source,
                                       int nreduce, int PE_start,
                                       int logPE_stride, int PE_size,
                                       short *pWrk, long *pSync);

  extern void pshmem_int_or_to_all (int *target,
                                    int *source,
                                    int nreduce,
                                    int PE_start, int logPE_stride,
                                    int PE_size, int *pWrk, long *pSync);
  extern void pshmem_long_or_to_all (long *target, long *source, int nreduce,
                                     int PE_start, int logPE_stride,
                                     int PE_size, long *pWrk, long *pSync);
  extern void pshmem_longlong_or_to_all (long long *target, long long *source,
                                         int nreduce, int PE_start,
                                         int logPE_stride, int PE_size,
                                         long long *pWrk, long *pSync);
  extern void pshmem_short_or_to_all (short *target, short *source,
                                      int nreduce, int PE_start,
                                      int logPE_stride, int PE_size,
                                      short *pWrk, long *pSync);

  extern void pshmem_int_xor_to_all (int *target,
                                     int *source,
                                     int nreduce,
                                     int PE_start, int logPE_stride,
                                     int PE_size, int *pWrk, long *pSync);
  extern void pshmem_long_xor_to_all (long *target, long *source, int nreduce,
                                      int PE_start, int logPE_stride,
                                      int PE_size, long *pWrk, long *pSync);
  extern void pshmem_longlong_xor_to_all (long long *target,
                                          long long *source, int nreduce,
                                          int PE_start, int logPE_stride,
                                          int PE_size, long long *pWrk,
                                          long *pSync);
  extern void pshmem_short_xor_to_all (short *target, short *source,
                                       int nreduce, int PE_start,
                                       int logPE_stride, int PE_size,
                                       short *pWrk, long *pSync);

  extern void pshmem_int_max_to_all (int *target,
                                     int *source,
                                     int nreduce,
                                     int PE_start, int logPE_stride,
                                     int PE_size, int *pWrk, long *pSync);
  extern void pshmem_long_max_to_all (long *target, long *source, int nreduce,
                                      int PE_start, int logPE_stride,
                                      int PE_size, long *pWrk, long *pSync);
  extern void pshmem_longlong_max_to_all (long long *target,
                                          long long *source, int nreduce,
                                          int PE_start, int logPE_stride,
                                          int PE_size, long long *pWrk,
                                          long *pSync);
  extern void pshmem_short_max_to_all (short *target, short *source,
                                       int nreduce, int PE_start,
                                       int logPE_stride, int PE_size,
                                       short *pWrk, long *pSync);
  extern void pshmem_longdouble_max_to_all (long double *target,
                                            long double *source, int nreduce,
                                            int PE_start, int logPE_stride,
                                            int PE_size, long double *pWrk,
                                            long *pSync);
  extern void pshmem_float_max_to_all (float *target, float *source,
                                       int nreduce, int PE_start,
                                       int logPE_stride, int PE_size,
                                       float *pWrk, long *pSync);
  extern void pshmem_double_max_to_all (double *target, double *source,
                                        int nreduce, int PE_start,
                                        int logPE_stride, int PE_size,
                                        double *pWrk, long *pSync);

  extern void pshmem_int_min_to_all (int *target,
                                     int *source,
                                     int nreduce,
                                     int PE_start, int logPE_stride,
                                     int PE_size, int *pWrk, long *pSync);
  extern void pshmem_long_min_to_all (long *target, long *source, int nreduce,
                                      int PE_start, int logPE_stride,
                                      int PE_size, long *pWrk, long *pSync);
  extern void pshmem_longlong_min_to_all (long long *target,
                                          long long *source, int nreduce,
                                          int PE_start, int logPE_stride,
                                          int PE_size, long long *pWrk,
                                          long *pSync);
  extern void pshmem_short_min_to_all (short *target, short *source,
                                       int nreduce, int PE_start,
                                       int logPE_stride, int PE_size,
                                       short *pWrk, long *pSync);
  extern void pshmem_longdouble_min_to_all (long double *target,
                                            long double *source, int nreduce,
                                            int PE_start, int logPE_stride,
                                            int PE_size, long double *pWrk,
                                            long *pSync);
  extern void pshmem_float_min_to_all (float *target, float *source,
                                       int nreduce, int PE_start,
                                       int logPE_stride, int PE_size,
                                       float *pWrk, long *pSync);
  extern void pshmem_double_min_to_all (double *target, double *source,
                                        int nreduce, int PE_start,
                                        int logPE_stride, int PE_size,
                                        double *pWrk, long *pSync);

  /*
   * broadcasts
   */

  extern void pshmem_broadcast32 (void *target, const void *source,
                                  size_t nelems, int PE_root, int PE_start,
                                  int logPE_stride, int PE_size, long *pSync);

  extern void pshmem_broadcast64 (void *target, const void *source,
                                  size_t nelems, int PE_root, int PE_start,
                                  int logPE_stride, int PE_size, long *pSync);

  /*
   * collects
   */

  extern void pshmem_fcollect32 (void *target, const void *source,
                                 size_t nelems, int PE_start,
                                 int logPE_stride, int PE_size, long *pSync);
  extern void pshmem_fcollect64 (void *target, const void *source,
                                 size_t nelems, int PE_start,
                                 int logPE_stride, int PE_size, long *pSync);

  extern void pshmem_collect32 (void *target, const void *source,
                                size_t nelems, int PE_start, int logPE_stride,
                                int PE_size, long *pSync);
  extern void pshmem_collect64 (void *target, const void *source,
                                size_t nelems, int PE_start, int logPE_stride,
                                int PE_size, long *pSync);

  /*
   * locks/critical section
   */

  extern void pshmem_set_lock (long *lock);
  extern void pshmem_clear_lock (long *lock);
  extern int  pshmem_test_lock (long *lock) _WUR;

  /*
   * --end--
   */

#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif				/* _PSHMEM_H */
