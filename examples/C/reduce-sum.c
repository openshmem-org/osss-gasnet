/* (c) 2011 University of Houston.  All rights reserved. */


/*
 * reduce by SUM() [1,2,3,4] across 4 PEs
 *
 */

#include <stdio.h>
#include <string.h>

#include <mpp/shmem.h>

int pWrk[_SHMEM_REDUCE_SYNC_SIZE];
long pSync[_SHMEM_BCAST_SYNC_SIZE];

int src;
int dst;

int
main()
{
  int i;

  for (i = 0; i < SHMEM_BCAST_SYNC_SIZE; i += 1) {
    pSync[i]  =_SHMEM_SYNC_VALUE;
  }

  start_pes(0);

  src = _my_pe() + 1;
  shmem_barrier_all();

  shmem_int_sum_to_all(&dst, &src, 1, 0, 0, 4, pWrk, pSync);

  printf("%d/%d   dst =", _my_pe(), _num_pes() );
  printf(" %d", dst);
  printf("\n");

  return 0;
}
