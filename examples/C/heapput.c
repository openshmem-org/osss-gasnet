/* (c) 2011 University of Houston System.  All rights reserved. */


#include <stdio.h>
#include <assert.h>

#include <mpp/shmem.h>

long dest = -999;

int
main()
{
  int *all;
  int me;

  start_pes(0);

  me = _my_pe();

  if (me == 0) {
    long src = 4321;
    shmem_long_put(&dest, &src, 1, 1);
  }

  shmem_barrier_all();

  printf("%d: dest = %ld\n", me, dest);

#if 0
  all = (int *) shmalloc(sizeof(*all));
  assert(all != NULL);

  *all = 314159;
  shmem_barrier_all();

  if (me == 1) {
    int send_to_all = 27182;
    shmem_int_put(all, &send_to_all, 1, 0);
  }

  shmem_barrier_all();

  printf("%d: all = %d\n", me, *all);

  shfree(all);
#endif

  return 0;
}
