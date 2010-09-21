#include <stdio.h>

#include <shmem.h>

int
main(void)
{
  float *f;
  int me;

  shmem_init();
  me = shmem_my_pe();

  f = (float *) shmalloc( sizeof(*f) );

  shmem_barrier_all();

  *f = 3.1415927;

  printf("BEFORE %d: f = %f\n", me, *f);

  if (me == 0) {
    shmem_float_p(f, 2.71828182, 1);
  }

  shmem_barrier_all();

  printf("AFTER %d: f = %f\n", me, *f);
}
