#include <stdio.h>

#include <mpp/shmem.h>

int
main(int argc, char **argv)
{
  int me, npes;

  start_pes(0);

  _my_pe();			/* expect to get warning here */
  npes = _num_pes();

  printf("Hello from %d PEs\n", npes); 

  return 0;
}
