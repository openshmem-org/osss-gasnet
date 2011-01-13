#include <stdio.h>

#include <mpp/shmem.h>

int
main(void)
{
  double *f;
  int me;

  start_pes(0);
  me = _my_pe();

  f = (double *) shmalloc( sizeof(*f) );

  *f = 3.1415927;
  shmem_barrier_all();

  printf("%d: before put, f = %f\n", me, *f);

  if (me == 0) {
    shmem_double_p(f, 2.71828182, 1);
  }

  shmem_barrier_all();

  printf("%d:  after put, f = %f\n", me, *f);
}
