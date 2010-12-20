/*
 * just do a shmalloc and a free
 */

#include <mpp/shmem.h>

int
main()
{
  long *x;

  start_pes(0);

  x = (long *) shmalloc(sizeof(*x));

  shfree(x);

  return 0;
}
