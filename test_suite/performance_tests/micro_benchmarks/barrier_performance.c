/* (c) 2011 University of Houston System.  All rights reserved. */
/* Performance test for shmem_barrier*/

#include <stdio.h>
#include <mpp/shmem.h>

#define NPES 4

long pSync[_SHMEM_BCAST_SYNC_SIZE];
int x = 10101;

int
main()
{
  int me, npes, src;
  int i,j;
  struct timeval start, end;
  long time_taken,start_time,end_time;

  for (i = 0; i < _SHMEM_BCAST_SYNC_SIZE; i += 1) {
    pSync[i] = _SHMEM_SYNC_VALUE;
  }

  start_pes(0);
  me = _my_pe();
  npes = _num_pes();
  src = me-1;
  time_taken = 0;

  for (i=0;i<10000;i++){
    if (me != 0) {
      shmem_int_p(&x,src*(i+1), me-1);
    }
    else
      shmem_int_p(&x,src*(i+1), npes-1);
    shmem_barrier_all();

    gettimeofday(&start, NULL);
    start_time = (start.tv_sec * 1000000.0) + start.tv_usec;

    shmem_barrier(0, 0, npes, pSync);

    gettimeofday(&end, NULL);
    end_time = (end.tv_sec * 1000000.0) + end.tv_usec;
    time_taken = time_taken + (end_time - start_time);

  }
  /*printf("%d: x = %d\n", me, x);*/
  if(me == 0)
    printf("Time required for a barrier, with %d PEs is %ld microseconds\n",npes,time_taken/10000);

  return 0;
}
