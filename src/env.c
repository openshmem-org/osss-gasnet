#include <stdlib.h>
#include <strings.h>

#include "trace.h"
/* #include "dispatch.h" */

void
__shmem_environment_init()
{
  __shmem_tracers_init();
  /* TODO: remove this for now....   __barrier_dispatch_init(); */
}
