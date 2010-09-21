#include <stdio.h>
#include <stdlib.h>

#include <shmem.h>

int
main(void)
{
  int i;
  long *pSync;
  long *target;
  long *source;
  int me, npes;

  shmem_init();
  me = shmem_my_pe();
  npes = shmem_num_pes();

  source = (long *) malloc( npes * sizeof(*source) );
  for (i = 0; i < npes; i += 1) {
    source[i] = i + 1;
  }

  target = (long *) shmalloc( npes * sizeof(*target) );

  pSync = shmem_sync_init();

  shmem_broadcast64(target, source, npes, 0, 0, 0, npes, pSync);

  shmem_barrier_all();

  for (i = 0; i < npes; i++) {
    printf("%-8d %ld\n", me, target[i]);
  }
  
  shfree(pSync);
  shfree(target);

  return 0;
}

