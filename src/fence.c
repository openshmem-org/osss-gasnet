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
