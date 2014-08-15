/*
 *
 * Copyright (c) 2011 - 2014
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



#ifndef _COMMS_H
#define _COMMS_H 1

#include <sys/types.h>
#ifdef HAVE_FEATURE_EXPERIMENTAL
#include "shmemx.h"
#endif /* HAVE_FEATURE_EXPERIMENTAL */

/*
 * initialization and shutdown of the communications layer
 */
extern void __shmem_comms_init (void);
extern void __shmem_comms_finalize (int status);
extern void __shmem_comms_exit (int status);

/*
 * query the environment (which rank/PE I am, how many PEs)
 */
extern int __shmem_comms_mynode (void);
extern int __shmem_comms_nodes (void);

/*
 * look at PE's environment
 */
extern char *__shmem_comms_getenv (const char *name);

/*
 * handlers for puts and gets
 */
extern void __shmem_comms_put (void *dst, void *src, size_t len, int pe);
extern void __shmem_comms_get (void *dst, void *src, size_t len, int pe);
extern void __shmem_comms_put_val (void *dst, long src, size_t len, int pe);
extern long __shmem_comms_get_val (void *src, size_t len, int pe);

extern void __shmem_comms_put_bulk (void *dst, void *src, size_t len, int pe);
extern void __shmem_comms_get_bulk (void *dst, void *src, size_t len, int pe);

#ifdef HAVE_FEATURE_EXPERIMENTAL
/*
 * handlers for non-blocking puts and gets (FUTURE)
 */
extern void __shmem_comms_put_nb (void *dst, void *src, size_t len, int pe, shmemx_request_handle_t *desc);
extern void __shmem_comms_get_nb (void *dst, void *src, size_t len, int pe, shmemx_request_handle_t *desc);

extern void __shmem_comms_wait_req (shmemx_request_handle_t desc);
extern void __shmem_comms_test_req (shmemx_request_handle_t desc, int *flag);
#endif /* HAVE_FEATURE_EXPERIMENTAL */

/*
 * exported to control polling and waiting
 */
extern void __shmem_comms_service (void);

/*
 * wait for a condition (e.g. variable's value) to change
 */
extern void __shmem_comms_wait_short_eq (short *var, short cmp_value);
extern void __shmem_comms_wait_int_eq (int *var, int cmp_value);
extern void __shmem_comms_wait_long_eq (long *var, long cmp_value);
extern void __shmem_comms_wait_longlong_eq (long long *var, long long cmp_value);

extern void __shmem_comms_wait_short_ne (short *var, short cmp_value);
extern void __shmem_comms_wait_int_ne (int *var, int cmp_value);
extern void __shmem_comms_wait_long_ne (long *var, long cmp_value);
extern void __shmem_comms_wait_longlong_ne (long long *var, long long cmp_value);

extern void __shmem_comms_wait_short_gt (short *var, short cmp_value);
extern void __shmem_comms_wait_int_gt (int *var, int cmp_value);
extern void __shmem_comms_wait_long_gt (long *var, long cmp_value);
extern void __shmem_comms_wait_longlong_gt (long long *var, long long cmp_value);

extern void __shmem_comms_wait_short_le (short *var, short cmp_value);
extern void __shmem_comms_wait_int_le (int *var, int cmp_value);
extern void __shmem_comms_wait_long_le (long *var, long cmp_value);
extern void __shmem_comms_wait_longlong_le (long long *var, long long cmp_value);

extern void __shmem_comms_wait_short_lt (short *var, short cmp_value);
extern void __shmem_comms_wait_int_lt (int *var, int cmp_value);
extern void __shmem_comms_wait_long_lt (long *var, long cmp_value);
extern void __shmem_comms_wait_longlong_lt (long long *var, long long cmp_value);

extern void __shmem_comms_wait_short_ge (short *var, short cmp_value);
extern void __shmem_comms_wait_int_ge (int *var, int cmp_value);
extern void __shmem_comms_wait_long_ge (long *var, long cmp_value);
extern void __shmem_comms_wait_longlong_ge (long long *var, long long cmp_value);

/*
 * for accessibility timeouts
 */
extern int __shmem_comms_ping_request (int pe);

/*
 * barriers
 */
extern void __shmem_comms_barrier_all (void);
extern void __shmem_comms_barrier (int PE_start, int logPE_stride,
				   int PE_size, long *pSync);

/*
 * the atomics we support
 *
 * Distinguish between 32 and 64 bit.
 *
 */
extern void __shmem_comms_swap_request32 (void *target, void *value,
					  size_t nbytes, int pe, void *retval);
extern void __shmem_comms_swap_request64 (void *target, void *value,
					  size_t nbytes, int pe, void *retval);

extern void __shmem_comms_cswap_request32 (void *target,
					   void *cond, void *value,
					   size_t nbytes, int pe, void *retval);
extern void __shmem_comms_cswap_request64 (void *target,
					   void *cond, void *value,
					   size_t nbytes, int pe, void *retval);

extern void __shmem_comms_fadd_request32 (void *target, void *value,
					  size_t nbytes, int pe, void *retval);
extern void __shmem_comms_fadd_request64 (void *target, void *value,
					  size_t nbytes, int pe, void *retval);

extern void __shmem_comms_finc_request32 (void *target, size_t nbytes,
					  int pe, void *retval);
extern void __shmem_comms_finc_request64 (void *target, size_t nbytes,
					  int pe, void *retval);

extern void __shmem_comms_add_request32 (void *target, void *value,
					 size_t nbytes, int pe);
extern void __shmem_comms_add_request64 (void *target, void *value,
					 size_t nbytes, int pe);

extern void __shmem_comms_inc_request32 (void *target, size_t nbytes, int pe);
extern void __shmem_comms_inc_request64 (void *target, size_t nbytes, int pe);

/*
 * Proposed by IBM Zurich
 *
 */
extern void __shmem_comms_xor_request32 (void *target, void *value,
					 size_t nbytes, int pe);
extern void __shmem_comms_xor_request64 (void *target, void *value,
					 size_t nbytes, int pe);

/*
 * fence and quiet initiators
 */
extern void __shmem_comms_fence_request (void);
extern void __shmem_comms_quiet_request (void);

/*
 * find where variable is on another PE
 */
extern void *__shmem_symmetric_addr_lookup (void *dest, int pe);

#endif /* _COMMS_H */
