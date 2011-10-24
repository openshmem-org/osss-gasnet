/* (c) 2011 University of Houston System.  All rights reserved. */
/* Performance test for shmem_XX_put */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpp/shmem.h>

#define N_ELEMENTS 25600
  int
main(void)
{
  int i,j,k;
  int *target;
  int *source;
  int me, npes;
  int nxtpe;
  struct timeval start, end;
  long time_taken,start_time,end_time;

  start_pes(0);
  me = _my_pe();
  npes = _num_pes();

  nxtpe = (me+1)%npes;
  source = (int *) shmalloc( N_ELEMENTS * sizeof(*source) );

  time_taken = 0;

  for (i = 0; i < N_ELEMENTS; i += 1) {
    source[i] = i + 1;
  }
  target = (int *) shmalloc( N_ELEMENTS * sizeof(*target) );
  for (i = 0; i < N_ELEMENTS; i += 1) {
    target[i] = -90;
  }
  shmem_barrier_all();

  for(i=0;i<10000;i++){
    gettimeofday(&start, NULL);

    start_time = (start.tv_sec * 1000000.0) + start.tv_usec;

    shmem_int_put(target, source, N_ELEMENTS,nxtpe);

    gettimeofday(&end, NULL);

    end_time = (end.tv_sec * 1000000.0) + end.tv_usec;
    if(me==0){
      time_taken = time_taken + (end_time - start_time);
    }

  }
  if(me == 0)
    printf("Time required for a int put of 100 Kbytes of data is %ld microseconds\n",time_taken/10000);

  shmem_barrier_all();

  shfree(target);
  shfree(source);
  return 0;
  }
