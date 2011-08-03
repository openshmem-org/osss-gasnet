/* (c) 2011 University of Houston System.  All rights reserved. */
/*Tests shmem_barrier call*/

#include <stdio.h>

#include <mpp/shmem.h>


long pSync[_SHMEM_BCAST_SYNC_SIZE];
int x = 10101;

int
main()
{
  int me, npes;
  int i;
  
  start_pes(0);
  me = _my_pe();
  npes = _num_pes();
  
  for (i = 0; i < _SHMEM_BCAST_SYNC_SIZE; i += 1) {
    pSync[i] = _SHMEM_SYNC_VALUE;
  }
  
  if(npes > 1){

  shmem_int_p(&x, 4, (me+1)%npes);
  
  shmem_barrier_all();
  
  if(me==npes-1){
    if(x==4)
      printf("Test shmem_barrier_all: Passed\n");
    else
      printf("Test shmem_barrier_all: Failed\n");
  } 
  
  x=-9;
  shmem_barrier_all();
 
  if(me==0||me==1){
  
    if(me==0)
	  shmem_int_p(&x, 4, 1);
  
    shmem_barrier(0, 0, 2, pSync);

    if(me==1){
      if(x==4)
        printf("Test shmem_barrier: Passed\n");
      else
        printf("Test shmem_barrier: Failed\n");
    } 
  }
}
else{
  printf("Number of PEs must be > 1 to test barrier, test skipped\n");

}  
  return 0;
}
