#include <shmem.h>

int
main(void)
{
  long *one, *two;

  shmem_init();

  one = (long *) shmalloc( 8 * sizeof(*one) );
  two = (long *) shmalloc( 23000000 * sizeof(*two) );

  shfree(one);
  shfree(two);

  return 0;
}
