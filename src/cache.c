/*
 *
 * Copyright (c) 2011, University of Houston System and Oak Ridge National
 * Loboratory.
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
 *   National Loboratory nor the names of its contributors may be used to
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



/*
 * so apparently these are compatibility routines
 * for older SGI architectures.  So probably fine
 * to let them all be empty
 */

#include "utils.h"
#include "trace.h"

#define CACHE_NO_OP(Name, Params)			\
  /* @api@ */						\
  void							\
  p##Name ( Params )					\
  {							\
    INIT_CHECK();					\
    __shmem_trace(SHMEM_LOG_CACHE,			\
		  "operation \"%s\" is a no-op",	\
		  #Name					\
		  );					\
    return;						\
  }


CACHE_NO_OP (shmem_clear_cache_inv, void)
CACHE_NO_OP (shmem_set_cache_inv, void)
CACHE_NO_OP (shmem_clear_cache_line_inv, void *target)
CACHE_NO_OP (shmem_set_cache_line_inv, void *target)
CACHE_NO_OP (shmem_udcflush, void)
CACHE_NO_OP (shmem_udcflush_line, void *target)
#pragma weak shmem_clear_cache_inv = pshmem_clear_cache_inv
#pragma weak shmem_set_cache_inv = pshmem_set_cache_inv
#pragma weak shmem_clear_cache_line_inv = pshmem_clear_cache_line_inv
#pragma weak shmem_set_cache_line_inv = pshmem_set_cache_line_inv
#pragma weak shmem_udcflush = pshmem_udcflush
#pragma weak shmem_udcflush_line = pshmem_udcflush_line
