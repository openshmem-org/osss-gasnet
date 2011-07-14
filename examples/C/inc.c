/* (c) 2011 University of Houston System.  All rights reserved. */


/*
 * expected output on 2 PEs:
 *
 * 0: dst = 74
 * 1: dst = 75
 *
 */

#include <stdio.h>

#include <mpp/shmem.h>

int dst;

int
main()
{
  int me;

  start_pes(2 /* unused, just a reminder */ );
  me = _my_pe();

  dst = 74;
  shmem_barrier_all();

  if (me == 0) {
    shmem_int_inc(&dst, 1);
  }
  shmem_barrier_all();

  printf("%d: dst = %d\n", me, dst);

  return 0;
}
