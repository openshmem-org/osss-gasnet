#include <stdlib.h>
#include <strings.h>

#include "state.h"
#include "warn.h"
#include "dispatch.h"
#include "barrier.h"

void
__shmem_environment_init()
{
  __barrier_dispatch_init();
}
