/* (c) 2011 University of Houston System.  All rights reserved. */


/*
 * simply prints out the version of OpenSHMEM from PE 0 only
 *
 */

#include <stdio.h>

#include <mpp/shmem.h>

int
main(int argc, char **argv)
{
  int npes;
  int me;

  start_pes(0);
  npes = _num_pes();
  me = _my_pe();

  if (me == 0) {
    int ma, mi;

    shmem_version(&ma, &mi);

    printf("PE %d (of %d) says hello from\n", me, npes);
    printf("  OpenSHMEM library version %d.%d\n",
           ma, mi
          );
  }

  return 0;
}
