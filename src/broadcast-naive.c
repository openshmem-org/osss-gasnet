#include <stdio.h>
#include <string.h>

#include "state.h"
#include "trace.h"

#include "shmem.h"


#define SHMEM_BROADCAST_TYPE(Name, Size)				\
  void									\
  __shmem_broadcast##Name##_naive (void *target, const void *source, size_t nlong, \
				   int PE_root, int PE_start,		\
				   int logPE_stride, int PE_size,	\
				   long *pSync)				\
  {									\
    const int typed_len = nlong * Size;					\
    const int step = 1 << logPE_stride;					\
    const int root = (PE_root * step) + PE_start;			\
    if (GET_STATE(mype) != root) {					\
      shmem_getmem(target, source, typed_len, root);			\
    }									\
    shmem_barrier(PE_start, logPE_stride, PE_size, pSync);		\
  }

SHMEM_BROADCAST_TYPE(32, 4)
SHMEM_BROADCAST_TYPE(64, 8)
