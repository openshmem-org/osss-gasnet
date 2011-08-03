/* (c) 2011 University of Houston System.  All rights reserved. */


/*
 * Tests shmem_long_max_to_all, shmem_long_min_to_all, shmem_int_sum_to_all
 * shmem_int_and_to_all, shmem_int_prod_to_all, shmem_int_or_to_all,
 * shmem_int_xor_to_all
 */

#include <stdio.h>
#include <string.h>

#include <mpp/shmem.h>

long pSync[_SHMEM_BCAST_SYNC_SIZE];

#define N 3


long src[N];
long dst[N];
int src1;
int dst1;
int expected_result;
int pWrk1[_SHMEM_REDUCE_SYNC_SIZE];
long pWrk2[_SHMEM_REDUCE_SYNC_SIZE];

int
main()
{
  int i,j;
  int me, npes;
  int success =0;

  start_pes(0);
  me = _my_pe();
  npes = _num_pes();


  for (i = 0; i < SHMEM_BCAST_SYNC_SIZE; i += 1) {
    pSync[i] = _SHMEM_SYNC_VALUE;
  }

  for (i = 0; i < N; i += 1) {
    src[i] = _my_pe() + i;
  }
  
  /*Test MAX*/
  shmem_barrier_all();

  shmem_long_max_to_all(dst, src, N, 0, 0, npes, pWrk2, pSync);

  if(me == 0){
    for (i = 0,j=-1; i < N; i++,j++) {
      if(dst[i] != npes+j)
        success =1;
    }
    if(success==1)
      printf("Test Reduction operation (max): Failed\n");
    else
      printf("Test Reduction operation (max): Passed\n");
  }
  
  
  /*Test MIN*/
  success = 0;
  for (i = 0; i < N; i += 1) {
    dst[i] = -9;
  }
   
  shmem_barrier_all();
  
  shmem_long_min_to_all(dst, src, N, 0, 0, npes, pWrk2, pSync);

  if(me == 0){
    for (i = 0; i < N; i++) {
      if(dst[i] != i)
        success =1;
    }
    if(success==1)
      printf("Test Reduction operation (min): Failed\n");
    else
      printf("Test Reduction operation (min): Passed\n");
  }
  
  /*Test SUM*/
  success=0;
  src1 = _my_pe() + 1;
  dst1=-9;
  shmem_barrier_all();

  shmem_int_sum_to_all(&dst1, &src1, 1, 0, 0, npes, pWrk1, pSync);

  if(me==0){
    if(dst1!= (npes * (npes+1)/2))
      printf("Test Reduction operation (sum): Failed\n");
    else
      printf("Test Reduction operation (sum): Passed\n");
  }
  
  /*Test AND */
  success=0;
  src1 = _my_pe()%3 + 1;
  dst1=-9;
  shmem_barrier_all();
  
  shmem_int_and_to_all(&dst1, &src1, 1, 0, 0, npes, pWrk1, pSync);
  
  if(me==0){
    if(dst1!= 0)
      printf("Test Reduction operation (and): Failed\n");
    else
      printf("Test Reduction operation (and): Passed\n");
  }
  
 /*Test PROD */
  src1 = (me + 1);
  dst1 = -9;
  success=0;
  expected_result = 1;
  for(i=1;i<=npes;i++){
    expected_result = expected_result * i;
  }
   
  shmem_barrier_all();
 
  shmem_int_prod_to_all(&dst1, &src1, 1, 0, 0, npes, pWrk1, pSync);
 
  if(me==0){
    if(dst1!=expected_result)
      printf("Test Reduction operation (prod): Failed\n");
    else
      printf("Test Reduction operation (prod): Passed\n");
  }
 
 /*Test OR*/
  src1 = (me + 1)% 4;
  dst1=-9;
  success=0;
  shmem_barrier_all();
 
  shmem_int_or_to_all(&dst1, &src1, 1, 0, 0, npes, pWrk1, pSync);
 
  if(me==0){
    if(dst1!=3)
      printf("Test Reduction operation (or): Failed\n");
    else
      printf("Test Reduction operation (or): Passed\n");
  }
 
 /*Test XOR*/
  src1 = me % 2;
  dst1=-9;
  success=0;
  if(npes%2==0)
    expected_result = 0;
  else
    expected_result = 1;
	
  shmem_barrier_all();
 
  shmem_int_xor_to_all(&dst1, &src1, 1, 0, 0, npes, pWrk1, pSync);
  
 
  if(me==0){
    if(dst1!=expected_result)
      printf("Test Reduction operation (xor): Failed\n");
    else
      printf("Test Reduction operation (xor): Passed\n");
  }

  return 0;
}
