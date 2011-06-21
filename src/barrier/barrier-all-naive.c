/* (c) 2011 University of Houston.  All rights reserved. */


#include "comms.h"

void
__shmem_barrier_all_naive(void)
{
  __shmem_comms_barrier_all();
}

#include "module_info.h"
module_info_t module_info =
  {
    __shmem_barrier_all_naive,
    __shmem_barrier_all_naive,
  };
