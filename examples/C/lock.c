#include <stdio.h>

#include <mpp/shmem.h>

int
main(int argc, char **argv)
{
  long L;

  start_pes(0);

  shmem_set_lock(&L);

  return 0;
}
