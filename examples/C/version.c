#include <stdio.h>

#include <shmem.h>

int
main(int argc, char **argv)
{
  shmem_init();

  printf("Hello from SHMEM library version \"%s\"\n",
         shmem_version());

  return 0;
}
