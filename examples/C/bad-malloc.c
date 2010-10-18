#include <stdio.h>

#include <mpp/shmem.h>

int
main(int argc, char *argv[])
{
  int me, npes;
  long *x;

  shmem_init();
  me = _my_pe();
  npes = _num_pes();

  x = (long *) shmalloc(8 * me);      /* deliberately pass different values */
  if (x == (long *) NULL) {
    fprintf(stderr, "shmalloc: %s\n", sherror());
    return 1;
  }

  shfree(x);

  return 0;
}
