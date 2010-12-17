#include "comms.h"

void
shmem_fence(void)
{
  __comms_fence();
}

/*
 * This is probably OK
 */
void
shmem_quiet(void)
{
  __comms_fence();
}

#ifdef HAVE_PSHMEM_SUPPORT
#pragma weak pshmem_fence = shmem_fence
#pragma weak pshmem_quiet = shmem_quiet
#endif /* HAVE_PSHMEM_SUPPORT */
