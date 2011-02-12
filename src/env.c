#include <stdlib.h>
#include <strings.h>

#include "trace.h"
#include "ping.h"

/* TODO: #include "dispatch.h" */

void
__shmem_environment_init()
{
  __shmem_tracers_init();
  __shmem_ping_init();
  /* TODO: remove this for now....   __barrier_dispatch_init(); */
}
