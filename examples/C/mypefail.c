#include <stdio.h>

#include <mpp/shmem.h>

int
main(int argc, char **argv)
{
  int me, npes;

  me  = _my_pe();

  printf("Hello from node %d\n", me); 

  return 0;
}
