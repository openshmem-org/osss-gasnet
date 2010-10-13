#include <stdio.h>

#include <mpp/shmem.h>

int
main(int argc, char **argv)
{
  shmem_init();

  printf("Hello from SHMEM library version \"%s\"\n",
         shmem_version());

  return 0;
}
