/* TODO: implement a sensible broadcast, this is just to have something in place */

#include <stdio.h>
#include <string.h>

#include "shmem.h"

#include "state.h"
#include "putget.h"

#define SHMEM_BROADCAST_TYPE(Size, Type)				\
  /* @api */								\
  void shmem_broadcast##Size (void *target, const void *source, size_t nlong, \
			      int PE_root, int PE_start, int logPE_stride, int PE_size, \
			      long *pSync)				\
  {									\
    const int step = 1 << logPE_stride;					\
    const int root = PE_root + PE_start;				\
    if (root == __state.mype) {						\
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

#ifdef HAVE_PSHMEM_SUPPORT
_Pragma("weak pshmem_broadcast32=shmem_broadcast32")
_Pragma("weak pshmem_broadcast64=shmem_broadcast64")
#endif /* HAVE_PSHMEM_SUPPORT */

/* @api@ */
long *
shmem_sync_init(void)
{
  long *sync;
  const int nb = _SHMEM_BCAST_SYNC_SIZE * sizeof(*sync);

  sync = shmalloc(nb);
  memset(sync, _SHMEM_SYNC_VALUE, nb);
  shmem_barrier_all();
  return sync;
}

#ifdef HAVE_PSHMEM_SUPPORT
_Pragma("weak pshmem_sync_init=shmem_sync_init")
#endif /* HAVE_PSHMEM_SUPPORT */
