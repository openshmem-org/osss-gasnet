#include <stdio.h>
#include <stdlib.h>

#include <mpp/shmem.h>

long pSync[_SHMEM_BCAST_SYNC_SIZE];

int
main(void)
{
  int i;
  long *target;
  long *source;
  int me, npes;

  start_pes(0);
  me = _my_pe();
  npes = _num_pes();

  source = (long *) malloc( npes * sizeof(*source) );
  for (i = 0; i < npes; i += 1) {
    source[i] = i + 1;
  }

  target = (long *) shmalloc( npes * sizeof(*target) );
  for (i = 0; i < npes; i += 1) {
    target[i] = -999;
  }
  shmem_barrier_all();

  shmem_sync_init(pSync);

  shmem_broadcast64(target, source, npes, 0, 0, 0, npes, pSync);

  // shmem_barrier_all();

  for (i = 0; i < npes; i++) {
    printf("%-8d %ld\n", me, target[i]);
  }

  shmem_barrier_all();
  
  shfree(target);
  free(source);

  return 0;
}
