#include <sys/types.h>

#include "state.h"
#include "putget.h"
#include "trace.h"

#include "pshmem.h"

/*
 * fcollect puts nelems (*same* value on all PEs) from source on each
 * PE in the set to target on all PEs in the set.  source -> target is
 * done in PE order.
 *
 */

/* @api@ */
void
pshmem_fcollect32(void *target, const void *source, size_t nelems,
		  int PE_start, int logPE_stride, int PE_size,
		  long *pSync)
{
  const int step = 1 << logPE_stride;
  const size_t tidx = nelems * sizeof(int) * __state.mype;
  int pe = PE_start;
  int i;
  for (i = 0; i < PE_size; i += 1) {
    pshmem_put32(target + tidx, source, nelems, pe);
    pe += step;
  }
  pshmem_barrier(PE_start, logPE_stride, PE_size, pSync);
  __shmem_trace(SHMEM_LOG_COLLECT,
		"completed barrier"
		);
}

/*
 * just twice the size
 */

/* @api@ */
void
pshmem_fcollect64(void *target, const void *source, size_t nelems,
		  int PE_start, int logPE_stride, int PE_size,
		  long *pSync)
{
  pshmem_fcollect32(target, source,
		    nelems + nelems,
		    PE_start, logPE_stride, PE_size, pSync
		    );
}

#pragma weak shmem_fcollect32 = pshmem_fcollect32
#pragma weak shmem_fcollect64 = pshmem_fcollect64
