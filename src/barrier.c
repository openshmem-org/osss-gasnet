#include "state.h"
#include "stats.h"
#include "comms.h"

void
shmem_barrier_all(void)
{
  __comms_barrier_all();
  SHMEM_STATS_BARRIER_ALL();
}

_Pragma("weak barrier=shmem_barrier_all")
