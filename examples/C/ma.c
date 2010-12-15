#include <stdio.h>

#include <mpp/shmem.h>

int
main()
{
  int me, npes;
  long *x;

  start_pes(0);
  me = _my_pe();
  npes = _num_pes();

  x = (long *) shmalloc(sizeof(*x));

  shfree(x);

  return 0;
}
