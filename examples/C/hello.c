#include <stdio.h>

#include <shmem.h>

int
main(int argc, char **argv)
{
  int me, npes;

  shmem_init();

  me   = my_pe();
  npes = num_pes();

  printf("Hello from node %4d of %4d\n", me, npes); 

  return 0;
}
