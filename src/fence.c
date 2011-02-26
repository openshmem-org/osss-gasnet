#include "comms.h"
#include "utils.h"

/* @api@ */
void
pshmem_fence(void)
{
  INIT_CHECK();
  __comms_fence_request();
}

/* @api@ */
void
pshmem_quiet(void)
{
  INIT_CHECK();
  __comms_quiet_request();
}

#pragma weak shmem_fence = pshmem_fence
#pragma weak shmem_quiet = pshmem_quiet
