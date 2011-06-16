/* (c) 2011 University of Houston.  All rights reserved. */


/*
 * integer value put to global
 *
 */

#include <stdio.h>

#include <mpp/shmem.h>

int n;

int
main(void)
{
  int me;

  start_pes(0);
  me = _my_pe();

  n = 3;

  shmem_barrier_all();

  if (me == 0) {
    shmem_int_p(&n, 42, 1);
  }

  shmem_barrier_all();

  /* now check */

  if (me == 1) {
    printf("%s\n", (n == 42) ? "OK" : "FAIL");
  }

  return 0;
}
