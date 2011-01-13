/*
 * long put using symmetric heap
 */

#include <stdio.h>

#include <mpp/shmem.h>

int
main(void)
{
  long *f;
  int me;

  start_pes(0);
  me = _my_pe();

  f = (long *) shmalloc( sizeof(*f) );

  *f = 3;
  shmem_barrier_all();

  printf("%d: before put, f = %d\n", me, *f);

  if (me == 0) {
    shmem_long_p(f, 42, 1);
  }

  shmem_barrier_all();

  printf("%d:  after put, f = %d\n", me, *f);
}
