/* (c) 2011 University of Houston System.  All rights reserved. */


#include "comms.h"

void
__shmem_barrier_all_linear(void)
{
  __shmem_comms_barrier_all();
}

#include "module_info.h"
module_info_t module_info =
  {
    __shmem_barrier_all_linear,
    __shmem_barrier_all_linear,
  };
