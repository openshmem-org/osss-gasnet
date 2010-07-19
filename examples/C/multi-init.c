/*
 * test handling multiple inits
 */

#include <stdio.h>

#include <shmem.h>

int
main(int argc, char **argv)
{
  shmem_init();
  shmem_init();

  printf("Hello from multi-init test\n");

  return 0;
}
