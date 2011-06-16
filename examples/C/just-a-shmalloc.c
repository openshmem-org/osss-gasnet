/* (c) 2011 University of Houston.  All rights reserved. */


/*
 * just do a shmalloc and a free, no output to be expected
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
