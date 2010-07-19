#include <stdio.h>

#include <shmem.h>

int
main(void)
{
  int i;
  long *pSync;
  long *target;
  long source[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  int nlong = 8;
  int me;

  shmem_init();
  me = shmem_my_pe();

  target = (long *) shmalloc( 8 * sizeof(*target) );

  pSync = shmem_sync_init();

  shmem_broadcast64(target, source, nlong, 0, 4, 0, 4, pSync);

  for (i = 0; i < 8; i++) {
    printf("%d: target[%d] = %ld\n", me, i, target[i]);
  }
  
  shmem_barrier_all();

  shfree(pSync);
  shfree(target);

  return 0;
}

