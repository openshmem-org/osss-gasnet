/* (c) 2011 University of Houston System.  All rights reserved. */


#include <stdio.h>
#include <string.h>

#include "state.h"
#include "trace.h"

#include "mpp/shmem.h"

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

SHMEM_BROADCAST_TYPE(32, 4)
SHMEM_BROADCAST_TYPE(64, 8)

#include "module_info.h"
module_info_t module_info =
  {
    __shmem_broadcast32_linear,
    __shmem_broadcast64_linear,
  };
