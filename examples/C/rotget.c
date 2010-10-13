/*
 * get from right neighbor, with wrap-around
 */

#include <stdio.h>
#include <time.h>

#include <shmem.h>

int
main(int argc, char **argv)
{
  long *src;
  long dest;
  int nextpe;
  int me, npes;

  shmem_init();
  me = shmem_my_pe();
  npes = shmem_num_pes();

  src = (long *)shmalloc(sizeof(*src));

  *src = me;

  shmem_barrier_all();

  nextpe = (me + 1) % npes;

  shmem_long_get(&dest, src, 1, nextpe);

  shmem_barrier_all();

  printf("%d @ %s: %ld\n", me, shmem_nodename(), dest);

  return 0;
}
