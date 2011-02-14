#include "trace.h"
#include "barrier.h"
#include "ping.h"

void
__shmem_environment_init()
{
  __shmem_tracers_init();
  __shmem_ping_init();
  __barrier_dispatch_init();
}
