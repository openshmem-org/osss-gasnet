#include "state.h"
#include "stats.h"
#include "comms.h"
#include "warn.h"
#include "dispatch.h"
#include "comms.h"
#include "hooks.h"

/*
 * don't actually do the work here, choose the appropriate dispatcher
 */

void
shmem_barrier_all(void)
{
  __shmem_dispatch_t barfunc = __shmem_dispatch[SHMEM_BARRIER_DISPATCH];

  if (barfunc == DISPATCH_NULL) {
    __shmem_warn(SHMEM_LOG_FATAL,
		 "no barrier implementation defined!"
		 );
  }

  __hooks_pre_barrier();

  (*barfunc)();

  __hooks_post_barrier();

  SHMEM_STATS_BARRIER();
}

_Pragma("weak barrier=shmem_barrier_all")

/*
 * now do the various different implementations here
 */

void
__shmem_barrier_all_basic(void)
{
  __comms_barrier_all();
}

/*
 * TODO: this needs to go into the dispatch framework too
 */

void
shmem_barrier(int PE_start, int logPE_stride, int PE_size, long *pSync)
{
  __comms_barrier(PE_start, logPE_stride, PE_size, pSync);
}
