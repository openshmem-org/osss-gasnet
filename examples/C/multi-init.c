/*
 * test handling multiple inits
 */

#include <stdio.h>

#include <mpp/shmem.h>

int
main(int argc, char **argv)
{
  start_pes(0);
  start_pes(0);

  printf("Hello from multi-init test\n");

  return 0;
}
