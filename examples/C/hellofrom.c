/* (c) 2011 University of Houston.  All rights reserved. */


#include <stdio.h>

#include <mpp/shmem.h>

int
main(int argc, char **argv)
{
  int me, npes;

  start_pes(0);
  me   = _my_pe();
  npes = _num_pes();

  printf("Hello from node %4d of %4d on \"%s\"\n",
         me, npes, shmem_nodename());

  return 0;
}
