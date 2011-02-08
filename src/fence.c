#include "comms.h"

/* @api@ */
void
pshmem_fence(void)
{
  __comms_fence();
}

/* @api@ */
void
pshmem_quiet(void)
{
  __comms_quiet();
}

#pragma weak shmem_fence = pshmem_fence
#pragma weak shmem_quiet = pshmem_quiet
