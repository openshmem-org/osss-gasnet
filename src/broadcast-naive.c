/* TODO: implement a sensible broadcast, this is just to have something in place */

#include <stdio.h>
#include <string.h>

#include "state.h"
#include "trace.h"

#include "shmem.h"


#define SHMEM_BROADCAST_TYPE(Size, Type)				\
  /* @api */								\
  void									\
  shmem_broadcast##Size##_naive (void *target, const void *source, size_t nlong, \
				 int PE_root, int PE_start,		\
				 int logPE_stride, int PE_size,		\
				 long *pSync)				\
  {									\
    const int step = 1 << logPE_stride;					\
    const int root = PE_root + PE_start;				\
    if (root == GET_STATE(mype)) {					\
      int i;								\
      register int this_pe = PE_start;					\
      for (i = 0; i < PE_size; i += 1) {				\
	if (this_pe != root) {						\
	  shmem_##Type##_put(target, source, nlong, this_pe);		\
	}								\
	this_pe += step;						\
      }									\
    }									\
    shmem_barrier(PE_start, logPE_stride, PE_size, pSync);		\
  }

SHMEM_BROADCAST_TYPE(32, int)
SHMEM_BROADCAST_TYPE(64, long)
