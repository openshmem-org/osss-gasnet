#include "comms.h"

void
shmem_fence(void)
{
  __comms_fence();
}

void
shmem_quiet(void)
{
  __comms_quiet();
}

#ifdef HAVE_PSHMEM_SUPPORT
#pragma weak pshmem_fence = shmem_fence
#pragma weak pshmem_quiet = shmem_quiet
#endif /* HAVE_PSHMEM_SUPPORT */
