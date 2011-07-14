/* (c) 2011 University of Houston System.  All rights reserved. */


#include <stdio.h>

#include <mpp/shmem.h>

int
main()
{
  start_pes(0);

  if (_my_pe() == 0) {
    shmem_int_put(NULL, NULL, 1, 1);
  }

  shmem_barrier_all();

  return 0;
}
