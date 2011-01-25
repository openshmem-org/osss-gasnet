#include <stdio.h>

#include <mpp/shmem.h>

int
main(int argc, char **argv)
{
  int me, npes;

  start_pes(0);

  my_pe();
  npes = _num_pes();

  printf("Hello from %d PEs\n", npes); 

  return 0;
}
