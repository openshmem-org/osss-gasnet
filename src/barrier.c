#include "state.h"
#include "stats.h"
#include "comms.h"
#include "warn.h"
#include "dispatch.h"
#include "comms.h"

/*
 * don't actually do the work here, choose the appropriate dispatcher
 */

__inline__ void
shmem_barrier_all(void)
{
  __shmem_dispatch_t barfunc = __shmem_dispatch[SHMEM_BARRIER_DISPATCH];

  if (barfunc == DISPATCH_NULL) {
    __shmem_warn(SHMEM_LOG_FATAL,
		 "no barrier implementation defined!"
		 );
  }

  (*barfunc)();

  SHMEM_STATS_BARRIER_ALL();
}

_Pragma("weak barrier=shmem_barrier_all")

/*
 * now do the various different implementations here
 */

__inline__ void
__shmem_barrier_all_basic(void)
{
  __comms_barrier_all();
}
