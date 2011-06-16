/* (c) 2011 University of Houston.  All rights reserved. */


/*
 * adaptation of example from SGI man page for shmem_iput.
 *
 * Code slightly cleaned up (removed cache call, fixed % format)
 *
 */

#include <stdio.h>
#include <mpp/shmem.h>

int
main()
{
  short source[10] = { 1, 2, 3, 4, 5,
		       6, 7, 8, 9, 10 };
  static short target[10];
  int me;

  start_pes(2);
  me = _my_pe();

  if (me == 0) {
    /* put 10 words into target on PE 1 */
    shmem_short_iput(target, source, 1, 2, 5, 1);
  }

  shmem_barrier_all();   /* sync sender and receiver */

  if (me == 1) {
    printf("target on PE %d is %hd %hd %hd %hd %hd\n", me,
	   target[0], target[1], target[2],
	   target[3], target[4] );
  }
  shmem_barrier_all();   /* sync before exiting */

  return 0;
}
