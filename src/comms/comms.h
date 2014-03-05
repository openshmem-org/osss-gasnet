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

/*
 * handlers for non-blocking puts and gets (FUTURE)
 */
extern void *__shmem_comms_put_nb (void *dst, void *src, size_t len, int pe);
extern void *__shmem_comms_get_nb (void *dst, void *src, size_t len, int pe);

extern void __shmem_comms_wait_nb (void *h);
extern int __shmem_comms_test_nb (void *h);

/*
 * exported to control polling and waiting
 */
extern void __shmem_comms_service (void);

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
 */
typedef enum
  {
    AMO_SWAP=0,
    AMO_CSWAP,
    AMO_FADD,
    AMO_FINC,
    AMO_ADD,
    AMO_INC,
    AMO_XOR,
  } atomic_op_t;

extern void __shmem_comms_amo_request (atomic_op_t op,
				       void *target, void *cond, void *value,
				       size_t nbytes,
				       int pe, void *retval);

/*
 * fence and quiet initiators
 */
extern void __shmem_comms_fence_request (void);
extern void __shmem_comms_quiet_request (void);

/*
 * TODO: review location of source
 */
extern void *__shmem_symmetric_addr_lookup (void *dest, int pe);

#endif /* _COMMS_H */
