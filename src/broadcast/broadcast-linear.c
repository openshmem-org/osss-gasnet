/*
 *
 * Copyright (c) 2011, University of Houston System and Oak Ridge National
 * Laboratory.
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



#include <stdio.h>
#include <string.h>

#include "state.h"
#include "trace.h"

#include "shmem.h"

#define SHMEM_BROADCAST_TYPE(Name, Size)				\
  void									\
  __shmem_broadcast##Name##_linear(void *target, const void *source, size_t nelems, \
				   int PE_root, int PE_start,		\
				   int logPE_stride, int PE_size,	\
				   long *pSync)				\
  {									\
    const int typed_nelems = nelems * Size;				\
    const int step = 1 << logPE_stride;					\
    const int root = (PE_root * step) + PE_start;			\
    if (GET_STATE(mype) != root) {					\
      shmem_getmem(target, source, typed_nelems, root);			\
    }									\
    shmem_barrier(PE_start, logPE_stride, PE_size, pSync);		\
  }									\

SHMEM_BROADCAST_TYPE (32, 4) SHMEM_BROADCAST_TYPE (64, 8)
#include "module_info.h"
     module_info_t
       module_info = {
       __shmem_broadcast32_linear,
       __shmem_broadcast64_linear,
     };
