/* (c) 2011 University of Houston System.  All rights reserved. */


/*
 * test if PE is accessible
 *
 */

#include <stdio.h>

#include <mpp/shmem.h>

int
main()
{
  int me, npes;

  setbuf(stdout, NULL);

  start_pes(0);
  me = _my_pe();
  npes = _num_pes();

  if (me == 0) {
    int i;
    for (i = 1; i < npes; i += 1) {
      printf("From %d: PE %d is ", me, i);
      printf("%s", shmem_pe_accessible(i) ? "" : "NOT ");
      printf("accessible\n");
    }
  }
  else {
    ;
  }

  return 0;
}
