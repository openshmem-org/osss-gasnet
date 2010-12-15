#include <stdio.h>

#include <mpp/shmem.h>

int
main(int argc, char **argv)
{
  start_pes(0);

  if (_my_pe() == 0) {
    printf("Hello from SHMEM library version \"%s\"\n",
           shmem_version()
          );
  }

  return 0;
}
