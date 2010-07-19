#include "gasnet_safe.h"         /* call wrapper w/ err handler    */

#include "state.h"
#include "stats.h"

/* ----------------------------------------------------------------- */

/*
 * use the global anonymous barrier for _all.
 *
 * will presumably have to set up our own barrier flags
 * for the subset-barrier
 */


void
__gasnet_barrier_all(void)
{
  GASNET_BEGIN_FUNCTION();
  gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
  gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);
}

void
shmem_barrier_all(void)
{
  __gasnet_barrier_all();
  SHMEM_STATS_BARRIER_ALL();
}

_Pragma("weak barrier=shmem_barrier_all")
