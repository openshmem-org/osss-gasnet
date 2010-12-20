#include <stdio.h>

#include <mpp/shmem.h>

int
main(void)
{
  float *f;
  int me;

  start_pes(0);
  me = _my_pe();

  f = (float *) shmalloc( sizeof(*f) );

  *f = 3.1415927;
  shmem_barrier_all();

  printf("BEFORE %d: f = %f\n", me, *f);

  if (me == 0) {
    shmem_float_p(f, 2.71828182, 1);
  }

  shmem_barrier_all();

  printf("AFTER %d: f = %f\n", me, *f);
}
