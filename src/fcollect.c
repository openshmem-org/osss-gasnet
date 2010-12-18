#include <sys/types.h>

#include "state.h"
#include "putget.h"
#include "warn.h"

#include "shmem.h"

/*
 * fcollect puts nelems (*same* value on all PEs) from source on each
 * PE in the set to target on all PEs in the set.  source -> target is
 * done in PE order.
 *
 */

#define FCOLLECT_EMIT(Size, Type)					\
  /* @api@ */								\
  void									\
  shmem_fcollect##Size (void *target, const void *source, size_t nelems, \
			int PE_start, int logPE_stride, int PE_size,	\
			long *pSync)					\
  {									\
    const int step = 1 << logPE_stride;					\
    const size_t tidx = nelems * sizeof(Type) * __state.mype;		\
    int pe = PE_start;							\
    int i;								\
    for (i = 0; i < PE_size; i += 1) {					\
      shmem_put##Size (target + tidx, source, nelems, pe);		\
      pe += step;							\
    }									\
    shmem_barrier(PE_start, logPE_stride, PE_size, pSync);		\
    __shmem_warn(SHMEM_LOG_COLLECT,					\
		 "completed barrier"					\
		 );							\
  }

FCOLLECT_EMIT(32, int)
FCOLLECT_EMIT(64, long)


#ifdef HAVE_PSHMEM_SUPPORT
#pragma weak pshmem_fcollect32 = shmem_fcollect32
#pragma weak pshmem_fcollect64 = shmem_fcollect64
#endif /* HAVE_PSHMEM_SUPPORT */
