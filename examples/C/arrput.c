/*
 * rotate put an array to right neighbor
 */

#include <stdio.h>
#include <time.h>

#include <shmem.h>

#define N 7

int
main(int argc, char **argv)
{
  int i;
  int nextpe;
  int me, npes;
  long src[N];
  long *dest;

  shmem_init();
  me = shmem_my_pe();
  npes = shmem_num_pes();

  dest = (long *)shmalloc( N * sizeof(*dest) );

  shmem_barrier_all();

  nextpe = (me + 1) % npes;

  for (i = 0; i < N; i += 1) {
    src[i] = (long)me;
  }

  shmem_long_put(dest, src, N, nextpe);

  shmem_barrier_all();

  for (i = 0; i < N; i += 1) {
    printf("%d @ %s: dest[%d] = %ld\n", me, shmem_nodename(), i, dest[i]);
  }

  shfree(dest);

  return 0;
}
