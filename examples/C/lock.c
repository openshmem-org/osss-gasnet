#include <stdio.h>

#include <mpp/shmem.h>

long L = 0;

int
main(int argc, char **argv)
{
  int i;
  int me;
  int slp;

  start_pes(0);
  me = _my_pe();
  slp = 1;

  shmem_barrier_all();

for (i = 0; i < 2; i+= 1) {

  if (me == 1) sleep(3);

  shmem_set_lock(&L);

  printf("%d: sleeping %d seconds...\n", me, slp);
  sleep(slp);
  printf("%d: sleeping...done\n", me);

  shmem_clear_lock(&L);

}

  shmem_barrier_all();

  return 0;
}
