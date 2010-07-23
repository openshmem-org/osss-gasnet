#include <shmem.h>

int
main(void)
{
  void *one;
  long *two;

  shmem_init();

  one = shmalloc( 23 * 8 );
  shmem_barrier_all();
  two = (long *) shmalloc( 23 * sizeof(*two) );
  shmem_barrier_all();

  shfree(one);
  shfree(two);

  return 0;
}
