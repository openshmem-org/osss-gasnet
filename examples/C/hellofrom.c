#include <stdio.h>

#include <shmem.h>

int
main(int argc, char **argv)
{
  int me, npes;

  shmem_init();
  me   = shmem_my_pe();
  npes = shmem_num_pes();

  printf("Hello from node %4d of %4d on \"%s\"\n",
         me, npes, shmem_nodename());

  return 0;
}
