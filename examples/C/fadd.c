/* (c) 2011 University of Houston.  All rights reserved. */


/*
 * expected output on 2 PEs:
 *
 * 0: old = 22, dst = 22
 * 1: old = -1, dst = 66
 *
 */

#include <stdio.h>

#include <mpp/shmem.h>

int dst;

int
main()
{
  int me;
  int old;

  start_pes(2 /* unused, just a reminder */ );
  me = _my_pe();

  old = -1;
  dst = 22;
  shmem_barrier_all();

  if (me == 1) {
    old = shmem_int_fadd(&dst, 44, 0);
  }
  shmem_barrier_all();

  printf("%d: old = %d, dst = %d\n", me, old, dst);

  return 0;
}
