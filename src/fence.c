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
_Pragma("weak pshmem_fence=shmem_fence")
_Pragma("weak pshmem_quiet=shmem_quiet")
#endif /* HAVE_PSHMEM_SUPPORT */
