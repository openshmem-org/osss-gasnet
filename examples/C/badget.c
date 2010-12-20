/*
 * get on PE 0 (master) from PE 1, but with a non-symmetric variable
 */

#include <stdio.h>
#include <time.h>

#include <mpp/shmem.h>

int
main(int argc, char **argv)
{
  long dest;
  long src;
  int me, npes;

  start_pes(0);

  me = _my_pe();
  npes = _num_pes();

  src = 42;

  shmem_barrier_all();

  if (me == 0) {
    shmem_long_get(&dest, &src, 1, 1);
  }

  shmem_barrier_all();

  return 0;
}
