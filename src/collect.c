#include <sys/types.h>

#include "state.h"
#include "putget.h"
#include "trace.h"
#include "symmem.h"
#include "utils.h"

#include "shmem.h"

/*
 * collect puts nelems (can vary from PE to PE) from source on each
 * PE in the set to target on all PEs in the set.  source -> target is
 * done in PE order.
 *
 * We set up a wavefront that propagates the accumulated offsets and
 * then overlaps forwarding the extra offset contribution from each PE
 * with actually sending source -> target.  Control data goes left to
 * right, application data goes "top to bottom", as it were.
 *
 */

/* @api@ */
void
pshmem_collect32(void *target, const void *source, size_t nelems,
		 int PE_start, int logPE_stride, int PE_size,
		 long *pSync)
{
  const int step = 1 << logPE_stride;
  const int last_pe = PE_start + step * (PE_size - 1);
  long save = pSync[0];
  long *acc_off = & (pSync[0]);

  INIT_CHECK();

  /* make sure accumulator has been initialized on all active PEs */
  *acc_off = (__state.mype > PE_start) ? -1 : 0;

  /*
   * wait for left neighbor (if it exists) to send accumulated
   * offsets
   */
  if (__state.mype > PE_start) {
    shmem_long_wait(acc_off, -1);
  }

  /*
   * forward my contribution to (notify) right neighbor if not last PE
   * in set
   */
  if (__state.mype < last_pe) {
    const long next_off = *acc_off + nelems;
    const int rnei = __state.mype + step;

    shmem_long_p(acc_off, next_off, rnei);
  }

  /* send my array slice to target everywhere */
  {
    const long tidx = *acc_off * sizeof(int);
    int i;
    int pe = PE_start;

    for (i = 0; i < PE_size; i += 1) {
      shmem_put32(target + tidx, source, nelems, pe);
      pe += step;
    }
  }

  /* wait for everyone to finish and clean up */
  *acc_off = save;
  shmem_barrier(PE_start, logPE_stride, PE_size, pSync);
}

/*
 * just twice the size of collect32
 */

/* @api@ */
void
pshmem_collect64(void *target, const void *source, size_t nelems,
		 int PE_start, int logPE_stride, int PE_size,
		 long *pSync)
{
  shmem_collect32(target, source,
		  nelems + nelems,
		  PE_start, logPE_stride, PE_size, pSync
		  );
}

#pragma weak shmem_collect32 = pshmem_collect32
#pragma weak shmem_collect64 = pshmem_collect64
