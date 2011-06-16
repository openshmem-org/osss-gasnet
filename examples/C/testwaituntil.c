/* (c) 2011 University of Houston.  All rights reserved. */


/*
 * PE 1 waits for PE 0 to send something other than 9.
 * Send 4 9s to test wait condition, then some random values until != 9.
 *
 * Include a want-this-to-fail call at the end
 */

#include <stdio.h>
#include <mpp/shmem.h>

int
main(void)
{
  int me, npes;
  long *dest;

  {
    time_t now;
    time(&now);
    srand( now + getpid() );
  }

  start_pes(0);
  me = _my_pe();
  npes = _num_pes();

  dest = (long *) shmalloc( sizeof(*dest) );

  *dest = 9L;
  shmem_barrier_all();

  if (me == 0) {
    int i;
    for (i = 0; i < 4; i += 1) {
      long src = 9L;
      shmem_long_put(dest, &src, 1, 1);
      fprintf(stderr, "PE %d put %d\n", me, src);
    }
    fprintf(stderr, "----------------------------\n");
    for (i = 0; i < 1000; i += 1) {
      long src = rand() % 10;
      shmem_long_put(dest, &src, 1, 1);
      fprintf(stderr, "PE %d put %d\n", me, src);
      if (src != 9L)
        break;
    }
  }

  shmem_barrier_all();

  return 0;
}
