#include <sys/types.h>

#include "state.h"
#include "putget.h"
#include "trace.h"
#include "utils.h"
#include "symmtest.h"

#include "mpp/shmem.h"

/*
 * fcollect puts nelems (*same* value on all PEs) from source on each
 * PE in the set to target on all PEs in the set.  source -> target is
 * done in PE order.
 *
 */

#define SHMEM_FCOLLECT(Bits, Bytes)					\
  /* @api@ */								\
  void									\
  pshmem_fcollect##Bits(void *target, const void *source, size_t nelems, \
			int PE_start, int logPE_stride, int PE_size,	\
			long *pSync)					\
  {									\
    const int step = 1 << logPE_stride;					\
    const int vpe = (GET_STATE(mype) - PE_start) >> logPE_stride;	\
    const size_t tidx = nelems * Bytes * vpe;				\
    int i;								\
    int pe = PE_start;							\
    INIT_CHECK();							\
    for (i = 0; i < PE_size; i += 1) {					\
      shmem_put##Bits(target + tidx, source, nelems, pe);		\
      pe += step;							\
    }									\
    shmem_barrier(PE_start, logPE_stride, PE_size, pSync);		\
    __shmem_trace(SHMEM_LOG_COLLECT,					\
		  "completed barrier"					\
		  );							\
  }

SHMEM_FCOLLECT(32, 4)
SHMEM_FCOLLECT(64, 8)

#pragma weak shmem_fcollect32 = pshmem_fcollect32
#pragma weak shmem_fcollect64 = pshmem_fcollect64
