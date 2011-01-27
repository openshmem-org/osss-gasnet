/*
 * put from 0 (master) to some other random PE
 */

#include <stdio.h>
#include <time.h>

#include <mpp/shmem.h>

int
main(int argc, char **argv)
{
  long src;
  long *dest;
  int me, npes;

  start_pes(0);

  me = _my_pe();
  npes = _num_pes();

  {
    time_t now;
    time(&now);
    srand( now + getpid() );
  }

  src = rand() % 1000;

  dest = (long *)shmalloc(sizeof(*dest));
  *dest = -1;
  shmem_barrier_all();

  if (me == 0) {
    int other_pe = rand() % npes;
    printf("%d: -> %d, sending value %ld\n", me, other_pe, src);
    shmem_long_put(dest, &src, 1, other_pe);
  }

  shmem_barrier_all();

  printf("Result: %d @ %s: %ld\n", me, shmem_nodename(), *dest);

  return 0;
}
