/*
 * rotate PE id to right neighbor (dest), with wrap-around
 */

#include <stdio.h>
#include <time.h>

#include <shmem.h>

int
main(int argc, char **argv)
{
  int nextpe;
  int me, npes;
  long src;
  long *dest;

  shmem_init();
  me = shmem_my_pe();
  npes = shmem_num_pes();

  dest = (long *)shmalloc(sizeof(*dest));
  shmem_barrier_all();

  nextpe = (me + 1) % npes;

  src = (long)me;

  shmem_long_put(dest, &src, 1, nextpe);

  shmem_barrier_all();

  printf("%d @ %s: %ld\n", me, shmem_nodename(), *dest);

  shmem_barrier_all();
  shfree(dest);

  return 0;
}
