#include <stdio.h>

#include <shmem.h>

int
main(int argc, char **argv)
{
  long L;

  shmem_init();

  shmem_set_lock(&L);

  return 0;
}
