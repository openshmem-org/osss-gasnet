/* (c) 2011 University of Houston System.  All rights reserved. */

/*Tests shmem_broadcast32 shmem_broadcast64 calls
 * PE 0 broadcasts to all other PEs
 * source and destination arrays are shmalloc-ed
 * */
#include <stdio.h>
#include <stdlib.h>

#include <mpp/shmem.h>

long pSync[_SHMEM_BCAST_SYNC_SIZE];

int
main(void)
{
  int i,success32, success64;
  int *targ;
  int *src;
  long *target;
  long *source;
  int me, npes;

  start_pes(0);
  me = _my_pe();
  npes = _num_pes();
  success32 = 0;
  success64 = 0;

  if(npes > 1){
    for (i = 0; i < _SHMEM_BCAST_SYNC_SIZE; i += 1) {
      pSync[i] = _SHMEM_SYNC_VALUE;
    }

    /*Test shmem_broadcast32*/
    src = (int *) shmalloc( npes * sizeof(*src) );
    for (i = 0; i < npes; i += 1) {
      src[i] = i + 1;
    }

    targ = (int *) shmalloc( npes * sizeof(*targ) );
    for (i = 0; i < npes; i += 1) {
      targ[i] = -999;
    }


    shmem_barrier_all();

    shmem_broadcast32(targ, src, npes, 0, 0, 0, npes, pSync);

    shmem_barrier_all();

    if(me == 1){
      for (i = 0; i < npes; i++) {
        if( targ[i] != (i+1))
          success32=1;
      }
      if(success32==1)
        printf("Test shmem_broadcast32: Failed\n");
      else
        printf("Test shmem_broadcast32: Passed\n");
    }

    shmem_barrier_all();

    /*Test shmem_broadcast64*/

    source = (long *) shmalloc( npes * sizeof(*source) );
    for (i = 0; i < npes; i += 1) {
      source[i] = i + 1;
    }

    target = (long *) shmalloc( npes * sizeof(*target) );
    for (i = 0; i < npes; i += 1) {
      target[i] = -999;
    }


    shmem_barrier_all();

    shmem_broadcast64(target, source, npes, 0, 0, 0, npes, pSync);

    shmem_barrier_all();

    if(me == 1){
      for (i = 0; i < npes; i++) {
        if( target[i] != (i+1))
          success64=1;
      }
      if(success64==1)
        printf("Test shmem_broadcast64: Failed\n");
      else
        printf("Test shmem_broadcast64: Passed\n");
    }

    shfree(targ);
    shfree(src);
    shfree(target);
    shfree(source);
  }
  else{
    printf("Number of PEs must be > 1 to test broadcast, test skipped\n");
  }
  return 0;
}
