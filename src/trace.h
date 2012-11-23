/*
 *
 * Copyright (c) 2011 - 2013
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



#ifndef _WARN_H
#define _WARN_H 1

#include <stdarg.h>
#include <stdio.h>

typedef enum
{
  SHMEM_LOG_FATAL = 0,		/* unrecoverable problem */

  SHMEM_LOG_ATOMIC,		/* used by various atomic ops */
  SHMEM_LOG_AUTH,		/* something not authorized */
  SHMEM_LOG_BARRIER,		/* barrier ops */
  SHMEM_LOG_BROADCAST,		/* broadcast ops */
  SHMEM_LOG_CACHE,		/* cache flushing ops */
  SHMEM_LOG_COLLECT,		/* [f]collect ops */
  SHMEM_LOG_DEBUG,		/* debugging information */
  SHMEM_LOG_FENCE,		/* fence calls */
  SHMEM_LOG_INFO,		/* informational */
  SHMEM_LOG_INIT,		/* during OpenSHMEM initialization */
  SHMEM_LOG_LOCK,		/* global locks */
  SHMEM_LOG_MEMORY,		/* symmetric memory operations */
  SHMEM_LOG_NOTICE,		/* serious, but non-fatal */
  SHMEM_LOG_QUIET,		/* quiet calls */
  SHMEM_LOG_REDUCE,		/* reduction ops */
  SHMEM_LOG_SERVICE,		/* network service thread */
  SHMEM_LOG_SYMBOLS,		/* dump global dymbol table */
  SHMEM_LOG_VERSION,		/* show library version */
  SHMEM_LOG_PROFILING,		/* for the PSHMEM profiling interface */
  SHMEM_LOG_MODULES,		/* loadable routines modules */
} shmem_trace_t;

extern void __shmem_tracers_init (void);
extern void __shmem_tracers_show (void);

extern void __shmem_maybe_tracers_show_info (void);

extern void __shmem_trace (shmem_trace_t msg_type, char *fmt, ...);
extern int __shmem_trace_is_enabled (shmem_trace_t level);

#endif /* _WARN_H */
