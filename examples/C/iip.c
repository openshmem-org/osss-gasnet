#include <stdio.h>

#include <shmem.h>

int
main(void)
{
  int *f;
  int me;

  shmem_init();
  me = shmem_my_pe();

  f = (int *) shmalloc( sizeof(*f) );

  shmem_barrier_all();

  *f = 3;

  printf("BEFORE %d: f = %d\n", me, *f);

  if (me == 0) {
    shmem_int_p(f, 42, 1);
  }

  shmem_barrier_all();

  printf("AFTER %d: f = %d\n", me, *f);
}
