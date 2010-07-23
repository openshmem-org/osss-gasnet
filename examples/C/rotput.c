/*
 * rotate PE id to right neighbor (dest), with wrap-around
 */

#include <stdio.h>
#include <time.h>

#include <shmem.h>

volatile int tony = 1;

int
main(int argc, char **argv)
{
  int nextpe;
  int me, npes;
  int src;
  int *dest;

  shmem_init();
  me = shmem_my_pe();
  npes = shmem_num_pes();

  dest = (int *)shmalloc(sizeof(*dest));
  shmem_barrier_all();

  nextpe = (me + 1) % npes;

  src = me;

  while (tony == 0);
  shmem_int_put(dest, &src, 1, nextpe);

  shmem_barrier_all();

  printf("%d @ %s: %d\n", me, shmem_nodename(), *dest);

  shmem_barrier_all();
  shfree(dest);

  return 0;
}
