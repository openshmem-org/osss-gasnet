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

  src = nextpe;

  while (tony == 0);
  shmem_int_put(dest, &src, 1, nextpe);

  shmem_barrier_all();

  printf("%4d @ %8s: got %4d: ", me, shmem_nodename(), *dest);
  if (*dest == me) {
    printf("CORRECT");
  }
  else {
    printf("WRONG, expected %d", me);
  }
  printf("\n");

  shfree(dest);
  shmem_barrier_all();

  return 0;
}
