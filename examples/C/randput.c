/*
 * put from 0 (master) to some other random PE
 */

#include <stdio.h>
#include <time.h>

#include <shmem.h>

int
main(int argc, char **argv)
{
  long src;
  long *dest;
  int me, npes;

  shmem_init();

  me = shmem_my_pe();
  npes = shmem_num_pes();

  {
    time_t now;
    time(&now);
    srand( now + getpid() );
  }

  src = rand() % 1000;

  dest = (long *)shmalloc(sizeof(*dest));

  if (me == 0) {
    int other_pe = rand() % npes;
    printf("Random destination PE is %d, sending value %ld\n", other_pe, src);
    shmem_long_put(dest, &src, 1, other_pe);
  }

  shmem_barrier_all();

  printf("%d @ %s: %ld\n", me, shmem_nodename(), *dest);

  return 0;
}
