#include <sys/types.h>

#include "state.h"
#include "putget.h"
#include "warn.h"
#include "symmem.h"

#include "shmem.h"

/*
 * fcollect puts nelems from source on each PE in the set to
 * target on all PEs in the set.  source -> target is done
 * in PE order.
 *
 */

void
shmem_collect32 (void *target, const void *source, size_t nelems,
		 int PE_start, int logPE_stride, int PE_size,
		 long *pSync)
{
  const int step = 1 << logPE_stride;
  const int rnei = __state.mype + step;
  const int last_pe = PE_start + step * (PE_size - 1);
  int pe;
  int i;
  long *acc_off = (long *) __shmalloc_no_check(sizeof(*acc_off));

  if (acc_off == (long *) NULL) {
    __shmem_warn(SHMEM_LOG_FATAL,
		 "internal error during memory allocation for collect"
		 /* NOT REACHED */
		 );
  }

  *acc_off = (__state.mype == PE_start) ? 0 : -1;

  /* make sure accumulator initialized */
  shmem_barrier(PE_start, logPE_stride, PE_size, pSync);

  /* wait for left neighbor to send accumulated indexes */
  if (__state.mype != PE_start) {
    __shmem_warn(SHMEM_LOG_COLLECT,
		 "enters wait"
		 );
    shmem_long_wait(acc_off, -1);
    __shmem_warn(SHMEM_LOG_COLLECT,
		 "leaves wait"
		 );
  }
  __shmem_warn(SHMEM_LOG_COLLECT,
	       "has offset %ld",
	       *acc_off
	       );

  /* forward my contribution to right neighbor if not last PE in set */
  if (__state.mype < last_pe) {
    long next_off = *acc_off + nelems;
    shmem_long_p(acc_off, next_off, rnei);
    __shmem_warn(SHMEM_LOG_COLLECT,
		 "sends offset %ld to %d",
		 next_off,
		 rnei
		 );
  }

  /* send my array slice to target everywhere */
  pe = PE_start;
  for (i = 0; i < PE_size; i += 1) {
    shmem_put32 (target + (*acc_off * sizeof(int)), source, nelems, pe);
    __shmem_warn(SHMEM_LOG_COLLECT,
		 "puts to offset %ld on PE %d",
		 *acc_off,
		 pe
		 );
    pe += step;
  }

  /* wait for everyone to finish and clean up */
  shmem_barrier(PE_start, logPE_stride, PE_size, pSync);
  __shmem_warn(SHMEM_LOG_COLLECT,
	       "completed barrier"
	       );
  shfree(acc_off);
}

void
shmem_collect64 (void *target, const void *source, size_t nelems,
		 int PE_start, int logPE_stride, int PE_size,
		 long *pSync)
{
  shmem_collect32 (target, source, nelems + nelems,
		   PE_start, logPE_stride, PE_size, pSync);
}

#ifdef HAVE_PSHMEM_SUPPORT
#pragma weak pshmem_collect32 = shmem_collect32
#pragma weak pshmem_collect64 = shmem_collect64
#endif /* HAVE_PSHMEM_SUPPORT */
