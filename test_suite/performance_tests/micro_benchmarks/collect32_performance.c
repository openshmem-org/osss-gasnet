/* (c) 2011 University of Houston System.  All rights reserved. */
/* Performance test for shmem_collect32*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpp/shmem.h>
long pSync[_SHMEM_BCAST_SYNC_SIZE];

#define N_ELEMENTS 4
  int
main(void)
{
  int i,j,k;
  int *target;
  int *source;
  int me, npes;
  struct timeval start, end;
  long time_taken,start_time,end_time;

  start_pes(0);
  me = _my_pe();
  npes = _num_pes();

  source = (int *) shmalloc( N_ELEMENTS * sizeof(*source) );

  time_taken = 0;

  for (i = 0; i < N_ELEMENTS; i += 1) {
    source[i] = (i + 1)*10 + me;
  }
  target = (int *) shmalloc( N_ELEMENTS * sizeof(*target)*npes );
  for (i = 0; i < N_ELEMENTS; i += 1) {
    target[i] = -90;
  }
  for (i = 0; i < _SHMEM_BCAST_SYNC_SIZE; i += 1) {
    pSync[i] = _SHMEM_SYNC_VALUE;
  }
  shmem_barrier_all();

  for(i=0;i<10000;i++){
    gettimeofday(&start, NULL);
 
    start_time = (start.tv_sec * 1000000.0) + start.tv_usec;
 
    shmem_collect32(target, source, N_ELEMENTS, 0, 0, npes, pSync);
 
    gettimeofday(&end, NULL);

    end_time = (end.tv_sec * 1000000.0) + end.tv_usec;
    if(me==0){
      time_taken = time_taken + (end_time - start_time);
    }

  }
  if(me == 0)
    printf("Time required to collect %d bytes of data, with %d PEs is %ld microseconds\n",(4*N_ELEMENTS * npes),npes,time_taken/10000);

  shmem_barrier_all();
 
  shfree(target);
  shfree(source);
  return 0;
}

