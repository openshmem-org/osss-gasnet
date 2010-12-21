#include <stdlib.h>
#include <strings.h>

#include "warn.h"
/* #include "dispatch.h" */

void
__shmem_environment_init()
{
  __shmem_warnings_init();
  /* TODO: remove this for now....   __barrier_dispatch_init(); */
}
