#include <stdio.h>

#include <mpp/shmem.h>

int
main(void)
{
  int i;
  long *pSync;
  long *target;
  long source[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  int nlong = 8;
  int me;

  start_pes(0);
  me = _my_pe();

  target = (long *) shmalloc( 8 * sizeof(*target) );

  pSync = (long *) shmalloc( _SHMEM_BCAST_SYNC_SIZE * sizeof(*pSync) );

  for (i = 0; i < _SHMEM_BCAST_SYNC_SIZE; i++) {
    pSync[i] = _SHMEM_SYNC_VALUE;
  }
  shmem_barrier_all();

  shmem_broadcast64(target, source, nlong, 0, 4, 1, 4, pSync);

  for (i = 0; i < 8; i++) {
    printf("%d: target[%d] = %ld\n", me, i, target[i]);
  }
  
  shmem_barrier_all();

  shfree(pSync);
  shfree(target);

  return 0;
}

