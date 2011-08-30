/* (c) 2011 University of Houston System.  All rights reserved. */


/*
 * shmem_int_wait,  shmem_int_wait_until,  shmem_long_wait,
 * shmem_long_wait_until, shmem_longlong_wait,  shmem_longlong_wait_until, shmem_short_wait,
 * shmem_short_wait_until
 *
 * Tests conditational wait (shmem_long_wait) call
 * PE 1 waits for PE 0 to send something other than 9.
 * Send 4 9s to test wait condition, then some random values until != 9.
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
  long src = 9L;

  if(npes>1){
  dest = (long *) shmalloc( sizeof(*dest) );

  *dest = 9L;
  shmem_barrier_all();

  if (me == 0) {
    int i;
    for (i = 0; i < 4; i += 1) {
      
      shmem_long_put(dest, &src, 1, 1);
    }
    for (i = 0; i < 10; i += 1) {
      long src = rand() % 10;
      shmem_long_put(dest, &src, 1, 1);
      if (src != 9L)
        break;
    }
  }

  shmem_barrier_all();

  if (me == 1) {
    shmem_long_wait(dest, 9L);
    printf("Test for conditional wait: Passed\n");
  }

  shmem_barrier_all();
  
  *dest = 9L;
  shmem_barrier_all();

  if (me == 0) {
    int i;
    for (i = 0; i < 4; i += 1) {
      long src = 9L;
      shmem_long_put(dest, &src, 1, 1);
    }
    for (i = 0; i < 10; i += 1) {
      long src = rand() % 10;
      shmem_long_put(dest, &src, 1, 1);
      if (src != 9L)
        break;
    }
  }

  shmem_barrier_all();

  if (me == 1) {
    shmem_long_wait_until(dest, SHMEM_CMP_NE, 9L);
    printf("Test for explicit conditional wait: Passed\n");
  }

  shmem_barrier_all();
  }
  else
    printf("Test for conditional wait requires more than 1 PE, test skipped\n");

  return 0;
}
