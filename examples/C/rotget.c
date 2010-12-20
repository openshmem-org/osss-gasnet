/*
 * get from right neighbor, with wrap-around
 */

#include <stdio.h>
#include <time.h>

#include <mpp/shmem.h>

int
main(int argc, char **argv)
{
  long *src;
  long dest;
  int nextpe;
  int me, npes;

  start_pes(0);
  me = _my_pe();
  npes = _num_pes();

  src = (long *)shmalloc(sizeof(*src));

  *src = me;

  shmem_barrier_all();

  nextpe = (me + 1) % npes;

  shmem_long_get(&dest, src, 1, nextpe);

  shmem_barrier_all();

  printf("%d @ %s: %ld\n", me, shmem_nodename(), dest);

  return 0;
}
